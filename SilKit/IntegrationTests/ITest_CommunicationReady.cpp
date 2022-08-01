/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include "GetTestPid.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "silkit/SilKit.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"
#include "silkit/services/orchestration/ILifecycleService.hpp"
#include "silkit/services/all.hpp"
#include "silkit/services/orchestration/string_utils.hpp"

#include "ConfigurationTestUtils.hpp"

namespace {

using namespace std::chrono_literals;
using namespace SilKit;
using namespace SilKit::Config;
using namespace SilKit::Services::PubSub;
using namespace SilKit::Services::Orchestration;

const std::string systemMasterName{"SystemMaster"};
uint64_t numMsgToPublishPerController;
uint64_t numMsgToReceiveTotal;

const std::chrono::milliseconds communicationTimeout{5000s};

class TestParticipant
{
public:
    TestParticipant(const TestParticipant&) = delete;
    TestParticipant& operator=(const TestParticipant&) = delete;
    TestParticipant(TestParticipant&&) = default;
    TestParticipant& operator=(TestParticipant&&) = default;

    TestParticipant(const std::string& newName, std::vector<std::string> newPubTopics,
                    std::vector<std::string> newSubTopics)
    {
        name = newName;
        pubTopics = newPubTopics;
        subTopics = newSubTopics;

        if (pubTopics.size() > 0)
        {
            pubControllers.reserve(pubTopics.size());
            hasPubControllers = true; 
        }

        if (subTopics.size() > 0)
        {
            subControllers.reserve(subTopics.size());
            hasSubControllers = true;
        }
        else
        {
            AffirmCommunication();
        }
    }
    ~TestParticipant()
    {
        if(preSimulationWorker.joinable())
        {
            preSimulationWorker.join();
        }
    }

    void ResetReception()
    {
        receiveMsgCount = 0;
        allReceivedPromise = std::promise<void>{};
        allReceived = false;
    }

    void AffirmCommunication()
    {
        allReceived = true;
        allReceivedPromise.set_value();
    }

    void AwaitCommunication() 
    {
        auto futureStatus = allReceivedPromise.get_future().wait_for(communicationTimeout);
        EXPECT_EQ(futureStatus, std::future_status::ready)
            << "Test Failure: Awaiting test communication timed out";
    }

    void AffirmAllDone()
    {
        allDone = true;
        allDonePromise.set_value();
    }

    void AwaitAllDone()
    {
        auto futureStatus = allDonePromise.get_future().wait_for(communicationTimeout);
        EXPECT_EQ(futureStatus, std::future_status::ready) << "Test Failure: Awaiting all done timed out";
    }

    void Publish()
    {
        std::stringstream ss1;
        ss1 << "[" << name << "] Start publishing..." << std::endl;
        std::cout << ss1.str();
        for (auto publisher : pubControllers)
        {
            for (uint64_t i = 0; i < numMsgToPublishPerController; i++)
            {
                publisher->Publish(std::vector<uint8_t>{0});
            }
        }
        std::stringstream ss2;
        ss2 << "[" << name << "] ...all published" << std::endl;
        std::cout << ss2.str();
    }

    //Participant's entry point
    void ThreadMain( const std::string& registryUri)
    {
        participant =
            SilKit::CreateParticipant(SilKit::Config::MakeParticipantConfigurationWithLogging(Services::Logging::Level::Warn), name, registryUri);

        lifecycleService = participant->CreateLifecycleService();
        auto* timeSyncService = lifecycleService->CreateTimeSyncService();

        // We need to create a dedicated thread, so we do not block the 
        // CommunicationReadyHandlerAsync when we communication becomes ready.
        preSimulationStart = preSimulationPromise.get_future();
        preSimulationWorker = std::thread{[this]{
            preSimulationStart.wait_for(communicationTimeout);

            std::stringstream buf;
            buf << "[" << name << "] preSimulationWorker";
            std::cout  << buf.str() << std::endl;

            if (hasPubControllers)
            {
                Publish();
            }

            if (hasSubControllers)
            {
                AwaitCommunication();
            }

            lifecycleService->CompleteCommunicationReadyHandlerAsync();
            AffirmAllDone();
        }};

        lifecycleService->SetCommunicationReadyHandlerAsync([this]() {
            std::cout << "[" << name << "] CommunicationReadyHandlerAsync: invoking preSimulationWorker" << std::endl;
            preSimulationPromise.set_value();
        });

        lifecycleService->SetStopHandler([this]() {
            std::cout << "[" << name << "] StopHandler" << std::endl;
        });

        lifecycleService->SetShutdownHandler([this]() {
            std::cout << "[" << name << "] ShutdownHandler" << std::endl;
        });

        if (hasPubControllers)
        {
            std::cout << "[" << name << "] Creating publishers..." << std::endl;
            uint32_t controllerIndex = 0;
            for (auto& topic : pubTopics)
            {
                const DataPublisherSpec spec{topic, ""};
                const auto controllerName = "Pub-" + std::to_string(controllerIndex);
                controllerIndex++;
                pubControllers.push_back(
                    participant->CreateDataPublisher(controllerName, spec));
            }
            std::cout << "[" << name << "] ...created publishers" << std::endl;
        }

        if (hasSubControllers)
        {
            std::cout << "[" << name << "] Creating subscribers..." << std::endl;
            uint32_t controllerIndex = 0;
            for (auto& topic : subTopics)
            {
                const DataSubscriberSpec spec{topic, ""};
                const auto controllerName = "Sub-" + std::to_string(controllerIndex);
                controllerIndex++;
                subControllers.push_back(participant->CreateDataSubscriber(
                    controllerName, spec,
                    [this](IDataSubscriber* /*subscriber*/, const DataMessageEvent& /*dataMessageEvent*/) {
                        if (!allReceived)
                        {
                            //std::cout << "[" << participant.name << "] Receive #" << participant.receiveMsgCount << std::endl;

                            receiveMsgCount++;
                            if (receiveMsgCount >= numMsgToReceiveTotal)
                            {
                                std::cout << "[" << name << "] All received" << std::endl;
                                AffirmCommunication();
                            }
                        }
                    } ));
            }
            std::cout << "[" << name << "] ...created subscribers" << std::endl;
        }

        timeSyncService->SetSimulationStepHandler(
            [this](std::chrono::nanoseconds now) {
                std::stringstream ss; 
                ss << "[" << name << "] SimTask now=" << now.count() << std::endl;
                std::cout << ss.str();
            }, 1s);

        LifecycleConfiguration lc{};
        lc.operationMode = OperationMode::Coordinated;
        auto finalStateFuture = lifecycleService->StartLifecycle(lc);
        std::cout << "[" << name << "] Started Lifecycle" << std::endl;

        finalStateFuture.get();
    }

    auto Name() const -> std::string
    {
        return name;
    }
private://Members
    std::string name;
    std::unique_ptr<IParticipant> participant;
    Services::Orchestration::ILifecycleService* lifecycleService{nullptr};
    std::vector<IDataPublisher*> pubControllers;
    std::vector<IDataSubscriber*> subControllers;
    std::vector<std::string> pubTopics;
    std::vector<std::string> subTopics;

    bool hasPubControllers = false;
    bool hasSubControllers = false;

    uint64_t receiveMsgCount = 0;

    bool allReceived{false};
    std::promise<void> allReceivedPromise = std::promise<void>{};

    bool allDone{false};
    std::promise<void> allDonePromise = std::promise<void>{};

    // allow communication before the simulation enters the actual running state
    std::promise<void> preSimulationPromise;
    std::future<void> preSimulationStart;
    std::thread preSimulationWorker;
};

class ITest_CommunicationReady : public testing::Test
{

protected:
    ITest_CommunicationReady()
    {
    }


    struct SystemMaster
    {
        ILifecycleService* lifecycleService;
        std::unique_ptr<IParticipant> participant;
        ISystemController* systemController;
        ISystemMonitor* systemMonitor;

        std::promise<void> waitForStopPromise;
        std::future<void> waitForStopFuture;
    };

    void SystemStateHandler(SystemState newState)
    {
        switch (newState)
        {
        case SystemState::Error:
            std::cout << "State = " << newState << std::endl;
            AbortAndFailTest("Reached SystemState::Error");
            break;
        case SystemState::Running:
            std::cout << "State = " << newState << std::endl;
            systemMaster.lifecycleService->Stop("End of test");
            systemMaster.waitForStopPromise.set_value();
            break;
        default: 
            std::cout << "State = " << newState << std::endl;
            break;
        }
    }

    void AbortAndFailTest(const std::string& reason)
    {
        systemMaster.systemController->AbortSimulation();
        FAIL() << reason;
    }


    void RunRegistry(const std::string& registryUri)
    {
        registry = SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::MakeEmptyParticipantConfiguration());
        registry->StartListening(registryUri);
    }

    void RunSystemMaster(const std::string& registryUri)
    {
        systemMaster.participant =
            SilKit::CreateParticipant(SilKit::Config::MakeEmptyParticipantConfiguration(), systemMasterName, registryUri);

        systemMaster.lifecycleService = systemMaster.participant->CreateLifecycleService();
        systemMaster.systemController = systemMaster.participant->CreateSystemController();
        systemMaster.systemMonitor = systemMaster.participant->CreateSystemMonitor();

        systemMaster.waitForStopFuture = systemMaster.waitForStopPromise.get_future();

        systemMaster.systemController->SetWorkflowConfiguration({participantNames});

        systemMaster.systemMonitor->AddSystemStateHandler([this](SystemState newState) {
            SystemStateHandler(newState);
        });

        LifecycleConfiguration lc;
        lc.operationMode = OperationMode::Coordinated;
        systemMaster.lifecycleService->StartLifecycle(lc);
    }

    void RunParticipantThreads(std::vector<TestParticipant>& participants, const std::string& registryUri)
    {
        for (auto& p : participants)
        {
            participantThreads.emplace_back(
                [&p, registryUri] { p.ThreadMain(registryUri); });
        }

    }

    void JoinParticipantThreads()
    {
        for (auto&& thread : participantThreads)
        {
            thread.join();
        }
        participantThreads.clear();
    }

    void SetupSystem(const std::string& registryUri, std::vector<TestParticipant>& participants)
    {
        for (auto&& p : participants)
        {
            participantNames.push_back(p.Name());
        }
        participantNames.push_back(systemMasterName);

        RunRegistry(registryUri);
        RunSystemMaster(registryUri);
        
    }

    void ShutdownSystem()
    {
        systemMaster.participant.reset();
        registry.reset();
    }

protected:
    std::vector<std::string> participantNames;
    std::unique_ptr<SilKit::Vendor::Vector::ISilKitRegistry> registry;
    SystemMaster systemMaster;
    std::vector<std::thread> participantThreads;

    std::chrono::seconds simtimeToPass{ 3s };
};


TEST_F(ITest_CommunicationReady, test_receive_comm_ready)
try {
    auto registryUri = MakeTestRegistryUri();

    // Setup 1 publisher and lots of subscribers that the subParticipant is busy 
    // doing the Subscription handshake and the pubParticipant not.
    // So we have 1->N communication on one topic.

    const uint32_t numPub = 1;
    const uint32_t numSub = 100u;
    const std::string commonTopic = "Topic";

    numMsgToPublishPerController = 100u;
    numMsgToReceiveTotal = numMsgToPublishPerController * numSub;

    std::vector<std::string> pubTopics;
    for (uint32_t i = 0; i < numPub; i++)
    {
        pubTopics.push_back(commonTopic);
    }

    std::vector<std::string> subTopics;
    for (uint32_t i = 0; i < numSub; i++)
    {
        subTopics.push_back(commonTopic);
    }

    std::vector<TestParticipant> participants;
    participants.push_back({"Pub", pubTopics, {}});
    participants.push_back({"Sub", {}, subTopics});

    SetupSystem(registryUri, participants);

    RunParticipantThreads(participants, registryUri);

    std::cout << ">> Await all done" << std::endl;
    for (auto& p : participants)
        p.AwaitAllDone();

    systemMaster.waitForStopFuture.wait_for(10s);

    JoinParticipantThreads();

    ShutdownSystem();
}
catch (const std::exception& error)
{
    std::stringstream ss;
    ss << "Something went wrong: " << error.what() << std::endl;
    AbortAndFailTest(ss.str());
}

} // anonymous namespace
