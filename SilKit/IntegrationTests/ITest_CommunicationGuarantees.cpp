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

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "silkit/SilKit.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"
#include "silkit/services/orchestration/ILifecycleService.hpp"
#include "silkit/services/all.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"

#include "ITestThreadSafeLogger.hpp"

namespace {

using namespace std::chrono_literals;
using namespace SilKit;
using namespace SilKit::Config;
using namespace SilKit::Services::PubSub;
using namespace SilKit::Services::Orchestration;
using namespace SilKit::Tests;

const std::string systemMasterName{"SystemMaster"};
uint64_t testParam_numMsgToPublishPerController;
uint64_t numMsgToReceiveTotal;
size_t testParam_messageSizeInBytes;

const std::chrono::milliseconds communicationTimeout{5000s};

enum class TimeMode
{
    Async,
    Sync
};


class TestParticipant
{
public:
    TestParticipant(const TestParticipant&) = delete;
    TestParticipant& operator=(const TestParticipant&) = delete;
    TestParticipant(TestParticipant&&) = default;
    TestParticipant& operator=(TestParticipant&&) = default;

    TestParticipant(const std::string& newName, std::vector<std::string> newPubTopics,
                    std::vector<std::string> newSubTopics, OperationMode newOperationMode, TimeMode newTimeMode)
    {
        name = newName;
        operationMode = newOperationMode;
        timeMode = newTimeMode;
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
        if (preSimulationWorker.joinable())
        {
            preSimulationWorker.join();
        }
    }

    void SetPublishInCommReady(bool on)
    {
        publishInCommReady = on;
    }
    void SetPublishInStopHandler(bool on)
    {
        publishInStopHandler = on;
    }
    void SetPublishInShutdownHandler(bool on)
    {
        publishInShutdownHandler = on;
    }
    void SetPublishInAbortHandler(bool on)
    {
        publishInAbortHandler = on;
    }
    void SetWaitForCommunicationInCommReadyHandler(bool on)
    {
        waitForCommunicationInCommReadyHandler = on;
    }

    auto GetPubBuffer()
    {
        return pubBuffer;
    }
    auto GetSubBuffer()
    {
        return subBuffer;
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
        if (futureStatus != std::future_status::ready)
        {
            FAIL() << "Test Failure: Awaiting test communication timed out";
        }
    }

    void AffirmRunning()
    {
        if (!reachedRunning)
        {
            reachedRunning = true;
            reachedRunningPromise.set_value();
        }
    }

    void AffirmFinalState()
    {
        if (!reachedFinalState)
        {
            reachedFinalState = true;
            finalStatePromise.set_value();
        }
    }

    void AwaitControllerCreation()
    {
        auto futureStatus = controllerCreationPromise.get_future().wait_for(communicationTimeout);
        EXPECT_EQ(futureStatus, std::future_status::ready) << "Test Failure: Awaiting controller creation timed out";
    }

    void AwaitRunning()
    {
        auto futureStatus = reachedRunningPromise.get_future().wait_for(communicationTimeout);
        EXPECT_EQ(futureStatus, std::future_status::ready) << "Test Failure: Awaiting running timed out";
    }

    void AwaitFinalState()
    {
        auto futureStatus = finalStatePromise.get_future().wait_for(communicationTimeout);
        EXPECT_EQ(futureStatus, std::future_status::ready) << "Test Failure: Awaiting final state timed out";
    }

    auto ToByteVec(uint64_t x)
    {
        std::vector<uint8_t> byteVec{};
        for (auto i = 0; i < 8; i++)
        {
            byteVec.push_back(static_cast<uint8_t>(x >> 8 * i));
        }
        return byteVec;
    }

    void Publish()
    {
        if (hasPubControllers)
        {
            Log() << "[" << name << "] Start publishing...";
            // Publish in reversed order that lastly created publishers come first
            for (auto publisher = pubControllers.rbegin(); publisher != pubControllers.rend(); ++publisher)
            {
                for (uint64_t i = 0; i < testParam_numMsgToPublishPerController; i++)
                {
                    //auto vec = ToByteVec(i);
                    auto vec = std::vector<uint8_t>(testParam_messageSizeInBytes, static_cast<uint8_t>(i));
                    (*publisher)->Publish(vec);
                    pubBuffer.push_back(vec);
                }
            }
            Log() << "[" << name << "] ...all published";
        }
    }

    void ParticipantThread(const std::string& registryUri)
    {
        participant = SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(
                                                    R"({"Logging": {"Sinks": [{"Type": "Stdout", "Level":"Warn"}]}})"),
                                                name, registryUri);

        systemController = SilKit::Experimental::Participant::CreateSystemController(participant.get());
        lifecycleService = participant->CreateLifecycleService({operationMode});

        ITimeSyncService* timeSyncService = nullptr;
        ISystemMonitor* systemMonitor = nullptr;
        if (timeMode == TimeMode::Sync)
        {
            // Sync participant stop the test in the first SimTask
            timeSyncService = lifecycleService->CreateTimeSyncService();
            timeSyncService->SetSimulationStepHandler(
                [](std::chrono::nanoseconds /*now*/, std::chrono::nanoseconds /*duration*/) {}, 1s);
        }

        // Async participant stop the test on enter RunningState
        systemMonitor = participant->CreateSystemMonitor();
        systemMonitor->AddParticipantStatusHandler([this](ParticipantStatus status) {
            if (status.state == ParticipantState::Running && status.participantName == name)
            {
                AffirmRunning();
            }
        });

        // We need to create a dedicated thread, so we do not block the
        // CommunicationReadyHandlerAsync when the communication becomes ready.
        commReadyFuture = commReadyPromise.get_future();
        preSimulationWorker = std::thread{[this] {
            commReadyFuture.wait_for(communicationTimeout);

            if (publishInCommReady)
            {
                Publish();
            }

            if (hasSubControllers && waitForCommunicationInCommReadyHandler)
            {
                AwaitCommunication();
            }

            Log() << "[" << name << "] CompleteCommunicationReadyHandlerAsync";
            lifecycleService->CompleteCommunicationReadyHandlerAsync();
        }};

        lifecycleService->SetCommunicationReadyHandlerAsync([this]() {
            Log() << "[" << name << "] CommunicationReadyHandlerAsync: invoking preSimulationWorker";
            commReadyPromise.set_value();
        });

        lifecycleService->SetStopHandler([this]() {
            Log() << "[" << name << "] StopHandler begin";
            if (publishInStopHandler)
            {
                Publish();
            }
            Log() << "[" << name << "] StopHandler end";
        });

        lifecycleService->SetShutdownHandler([this]() {
            Log() << "[" << name << "] ShutdownHandler begin";
            if (publishInShutdownHandler)
            {
                Publish();
            }
            Log() << "[" << name << "] ShutdownHandler end";
        });

        lifecycleService->SetAbortHandler([this](ParticipantState /*lastState*/) {
            Log() << "[" << name << "] AbortHandler begin";
            if (publishInAbortHandler)
            {
                Publish();
            }
            Log() << "[" << name << "] AbortHandler end";
        });


        if (hasPubControllers)
        {
            Log() << "[" << name << "] Creating publishers...";
            uint32_t controllerIndex = 0;
            for (auto& topic : pubTopics)
            {
                const PubSubSpec spec{topic, ""};
                const auto controllerName = "Pub-" + std::to_string(controllerIndex);
                controllerIndex++;
                pubControllers.push_back(participant->CreateDataPublisher(controllerName, spec));
            }
            Log() << "[" << name << "] ...created publishers";
        }

        if (hasSubControllers)
        {
            Log() << "[" << name << "] Creating subscribers...";
            uint32_t controllerIndex = 0;
            for (auto& topic : subTopics)
            {
                const PubSubSpec spec{topic, ""};
                const auto controllerName = "Sub-" + std::to_string(controllerIndex);
                controllerIndex++;
                subControllers.push_back(participant->CreateDataSubscriber(
                    controllerName, spec,
                    [this](IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent) {
                    if (!allReceived)
                    {
                        receiveMsgCount++;
                        if (receiveMsgCount >= numMsgToReceiveTotal)
                        {
                            Log() << "[" << name << "] All received";
                            AffirmCommunication();
                        }
                    }
                    subBuffer.push_back(SilKit::Util::ToStdVector(dataMessageEvent.data));
                }));
            }
            Log() << "[" << name << "] ...created subscribers";
        }

        controllerCreationPromise.set_value();

        Log() << "[" << name << "] Starting Lifecycle...";
        auto finalStateFuture = lifecycleService->StartLifecycle();
        Log() << "[" << name << "] ... Lifecycle started";

        finalStateFuture.get();

        AffirmFinalState();
    }

    auto Name() const -> std::string
    {
        return name;
    }

public:
    std::string name;
    OperationMode operationMode;
    TimeMode timeMode;
    ILifecycleService* lifecycleService{nullptr};
    SilKit::Experimental::Services::Orchestration::ISystemController* systemController{nullptr};

private: //Members
    std::unique_ptr<IParticipant> participant;
    std::vector<IDataPublisher*> pubControllers;
    std::vector<IDataSubscriber*> subControllers;
    std::vector<std::string> pubTopics;
    std::vector<std::string> subTopics;

    std::vector<std::vector<uint8_t>> pubBuffer;
    std::vector<std::vector<uint8_t>> subBuffer;
    bool publishInCommReady = false;
    bool publishInStopHandler = false;
    bool publishInShutdownHandler = false;
    bool publishInAbortHandler = false;
    bool waitForCommunicationInCommReadyHandler = false;

    bool hasPubControllers = false;
    bool hasSubControllers = false;

    uint64_t receiveMsgCount = 0;

    bool allReceived{false};
    std::promise<void> allReceivedPromise = std::promise<void>{};

    std::promise<void> controllerCreationPromise = std::promise<void>{};

    bool reachedRunning{false};
    std::promise<void> reachedRunningPromise = std::promise<void>{};

    bool reachedFinalState{false};
    std::promise<void> finalStatePromise = std::promise<void>{};

    // allow communication before the simulation enters the actual running state
    std::promise<void> commReadyPromise;
    std::future<void> commReadyFuture;
    std::thread preSimulationWorker;
};

class ITest_CommunicationGuarantees : public testing::Test
{
protected:
    ITest_CommunicationGuarantees() {}

    struct SystemMaster
    {
        ILifecycleService* lifecycleService;
        std::unique_ptr<IParticipant> participant;
        SilKit::Experimental::Services::Orchestration::ISystemController* systemController;
        ISystemMonitor* systemMonitor;

        std::promise<void> systemStateRunningPromise;
        std::future<void> systemStateRunning;
        bool active;
    };

    void SystemStateHandler(SystemState newState)
    {
        switch (newState)
        {
        case SystemState::Error:
            Log() << "[Monitor] "
                  << "SystemState = " << newState;
            AbortAndFailTest("Reached SystemState::Error");
            break;
        case SystemState::Running:
            Log() << "[Monitor] "
                  << "SystemState = " << newState;
            systemController.systemStateRunningPromise.set_value();
            break;
        default:
            Log() << "[Monitor] "
                  << "SystemState = " << newState;
            break;
        }
    }

    void ParticipantStatusHandler(ParticipantStatus newStatus)
    {
        Log() << "[Monitor] " << newStatus.participantName << ": ParticipantState = " << newStatus.state;
    }

    void AbortAndFailTest(const std::string& reason)
    {
        systemController.systemController->AbortSimulation();
        FAIL() << reason;
    }

    auto RunRegistry(const std::string& registryUri) -> std::string
    {
        registry = SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::ParticipantConfigurationFromString(""));
        return registry->StartListening(registryUri);
    }

    void RunSystemMaster(const std::string& registryUri)
    {
        systemController.participant = SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(""),
                                                                 systemMasterName, registryUri);

        systemController.lifecycleService =
            systemController.participant->CreateLifecycleService({OperationMode::Coordinated});

        systemController.systemController =
            SilKit::Experimental::Participant::CreateSystemController(systemController.participant.get());

        systemController.systemMonitor = systemController.participant->CreateSystemMonitor();

        systemController.systemController->SetWorkflowConfiguration({requiredParticipantNames});

        systemController.systemMonitor->AddSystemStateHandler(
            [this](SystemState newState) { SystemStateHandler(newState); });

        systemController.systemMonitor->AddParticipantStatusHandler(
            [this](ParticipantStatus newStatus) { ParticipantStatusHandler(newStatus); });

        systemController.systemStateRunning = systemController.systemStateRunningPromise.get_future();

        finalStateSystemMaster = systemController.lifecycleService->StartLifecycle();
    }

    void RunParticipantThreads(std::vector<TestParticipant>& participants, const std::string& registryUri)
    {
        for (auto& p : participants)
        {
            participantThreads.emplace_back([&p, registryUri] { p.ParticipantThread(registryUri); });
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

    auto SetupSystem(const std::string& requestedRegistryUri, std::vector<TestParticipant>& participants) -> std::string
    {
        auto registryUri = RunRegistry(requestedRegistryUri);

        for (auto&& p : participants)
        {
            if (p.operationMode == OperationMode::Coordinated)
            {
                requiredParticipantNames.push_back(p.Name());
            }
        }

        if (!requiredParticipantNames.empty())
        {
            systemController.active = true;
            requiredParticipantNames.push_back(systemMasterName);
            RunSystemMaster(registryUri);
        }
        else
        {
            systemController.active = false;
        }
        return registryUri;
    }

    void SystemCleanup()
    {
        systemController.participant.reset();
        registry.reset();
        requiredParticipantNames.clear();
    }

    // Test flow reused for comm ready tests
    void ExecuteCommReadyTest(std::vector<TestParticipant>& participants)
    {
        try
        {
            auto registryUri = SetupSystem("silkit://localhost:0", participants);
            RunParticipantThreads(participants, registryUri);
            Log() << ">> Await all done";
            for (auto& p : participants)
                p.AwaitRunning();

            // In this test flow for comm ready tests, participants can stop after entering the running state

            // systemController.stop for coordinated participant
            if (systemController.active)
            {
                ASSERT_EQ(systemController.systemStateRunning.wait_for(1s), std::future_status::ready);
                systemController.lifecycleService->Stop("End of test");
            }

            // Autonomous have to call stop themselves
            for (auto& p : participants)
            {
                if (p.operationMode == OperationMode::Autonomous)
                    p.lifecycleService->Stop("End test");
            }

            Log() << ">> Joining participant threads";
            JoinParticipantThreads();
            SystemCleanup();
        }
        catch (const std::exception& error)
        {
            std::stringstream ss;
            ss << "Something went wrong: " << error.what();
            AbortAndFailTest(ss.str());
        }
    }

    // Test flow reused for stop/shutdown/abort tests
    void ExecuteHandlerTest(std::string handlerToTest)
    {
        try
        {
            const std::string topic1 = "Topic1";

            // One large message
            testParam_numMsgToPublishPerController = 1u;
            testParam_messageSizeInBytes = size_t{10000000};

            std::vector<TestParticipant> participants;
            participants.reserve(2);
            participants.push_back({"Pub", {topic1}, {}, OperationMode::Autonomous, TimeMode::Async});
            auto& pubParticipant = participants.at(0);
            if (handlerToTest == "Stop")
                pubParticipant.SetPublishInStopHandler(true);
            else if (handlerToTest == "Shutdown")
                pubParticipant.SetPublishInShutdownHandler(true);
            else if (handlerToTest == "Abort")
                pubParticipant.SetPublishInAbortHandler(true);

            participants.push_back({"Sub", {}, {topic1}, OperationMode::Autonomous, TimeMode::Async});
            auto& subParticipant = participants.at(1);

            auto registryUri = SetupSystem("silkit://localhost:0", participants);
            RunParticipantThreads(participants, registryUri);

            for (auto& p : participants)
            {
                p.AwaitRunning();
            }

            if (handlerToTest == "Abort")
            {
                subParticipant.systemController->AbortSimulation();
            }
            else
            {
                subParticipant.lifecycleService->Stop("Stop Subscriber");
                pubParticipant.lifecycleService->Stop("Stop Publisher");
            }

            for (auto& p : participants)
            {
                p.AwaitFinalState();
            }

            auto pubBuffer = pubParticipant.GetPubBuffer();
            auto subBuffer = subParticipant.GetSubBuffer();

            EXPECT_GT(pubBuffer.size(), size_t(0));
            EXPECT_EQ(pubBuffer, subBuffer)
                << "Guarantee that all messages sent in stop handler of pub participant were"
                << " received in sub participant prior to termination";

            JoinParticipantThreads();
            SystemCleanup();
        }
        catch (const std::exception& error)
        {
            std::stringstream ss;
            ss << "Something went wrong: " << error.what();
            AbortAndFailTest(ss.str());
        }
    }

protected:
    std::vector<std::string> requiredParticipantNames;
    std::unique_ptr<SilKit::Vendor::Vector::ISilKitRegistry> registry;
    SystemMaster systemController;
    std::future<ParticipantState> finalStateSystemMaster;
    std::vector<std::thread> participantThreads;
};


// Tests that basic pubsub communication in the CommunicationReadyHandler works with autonomous participants
TEST_F(ITest_CommunicationGuarantees, test_receive_in_comm_ready_handler_autonomous)
{
    const uint32_t numPub = 10u;
    const uint32_t numSub = 10u;
    const std::string commonTopic = "Topic";

    testParam_numMsgToPublishPerController = 100u;
    numMsgToReceiveTotal = testParam_numMsgToPublishPerController * numPub * numSub;
    testParam_messageSizeInBytes = 100;

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

    std::vector<TestParticipant> autonomousAsyncParticipantsPub;
    autonomousAsyncParticipantsPub.push_back({"Pub", pubTopics, {}, OperationMode::Autonomous, TimeMode::Async});
    autonomousAsyncParticipantsPub.at(0).SetPublishInCommReady(true);
    autonomousAsyncParticipantsPub.at(0).SetWaitForCommunicationInCommReadyHandler(true);

    std::vector<TestParticipant> autonomousAsyncParticipantsSub;
    autonomousAsyncParticipantsSub.push_back({"Sub", {}, subTopics, OperationMode::Autonomous, TimeMode::Async});
    autonomousAsyncParticipantsSub.at(0).SetWaitForCommunicationInCommReadyHandler(true);

    try
    {
        auto registryUri = RunRegistry("silkit://localhost:0");
        RunParticipantThreads(autonomousAsyncParticipantsSub, registryUri);
        // If we would start all participants at once, we cannot guarantee communication because
        // the publish might happen even before the subscriber created it's controllers.
        // We can guarantee communication in the CommReadyHandler between PubSub/RPC controllers
        // that have been created by the time the participant calls StartLifecycle.
        // E.g. Subscribers that are created on Participant1 after Participant2 called StartLifecycle may
        // not see a publication stemming from the CommReadyHandler of participant2

        // In this test, we wait until the subscriber participant has called StartLifecycle. Then, all of
        // his controllers have been created.
        Log() << ">> Await controller creation";
        for (auto& p : autonomousAsyncParticipantsSub)
            p.AwaitControllerCreation();

        // Then, start the publishers
        RunParticipantThreads(autonomousAsyncParticipantsPub, registryUri);

        Log() << ">> Await all done";
        for (auto& p : autonomousAsyncParticipantsPub)
            p.AwaitRunning();

        for (auto& p : autonomousAsyncParticipantsSub)
            p.AwaitRunning();

        for (auto& p : autonomousAsyncParticipantsPub)
        {
            p.lifecycleService->Stop("End test");
        }
        for (auto& p : autonomousAsyncParticipantsSub)
        {
            p.lifecycleService->Stop("End test");
        }

        Log() << ">> Joining participant threads";
        JoinParticipantThreads();
        SystemCleanup();
    }
    catch (const std::exception& error)
    {
        std::stringstream ss;
        ss << "Something went wrong: " << error.what();
        AbortAndFailTest(ss.str());
    }
}

// Tests that basic pubsub communication in the CommunicationReadyHandler works with coordinated participants
TEST_F(ITest_CommunicationGuarantees, test_receive_in_comm_ready_handler_coordinated_sync)
{
    const uint32_t numPub = 1u;
    const uint32_t numSub = 1u;
    const std::string commonTopic = "Topic";

    testParam_numMsgToPublishPerController = 100u;
    numMsgToReceiveTotal = testParam_numMsgToPublishPerController * numPub * numSub;
    testParam_messageSizeInBytes = 100;

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

    std::vector<TestParticipant> coordinatedSyncParticipants;
    coordinatedSyncParticipants.push_back({"Pub", pubTopics, {}, OperationMode::Coordinated, TimeMode::Sync});
    coordinatedSyncParticipants.back().SetPublishInCommReady(true);
    coordinatedSyncParticipants.back().SetWaitForCommunicationInCommReadyHandler(true);

    coordinatedSyncParticipants.push_back({"Sub", {}, subTopics, OperationMode::Coordinated, TimeMode::Sync});
    coordinatedSyncParticipants.back().SetWaitForCommunicationInCommReadyHandler(true);

    ExecuteCommReadyTest(coordinatedSyncParticipants);
}

// Tests that basic pubsub communication in the CommunicationReadyHandler works with
// already active coordinated participants and a late-joining autonomous participants that published in the commReadyHandler
TEST_F(ITest_CommunicationGuarantees, test_receive_in_comm_ready_handler_mixed)
{
    const uint32_t numPub = 10u;
    const uint32_t numSub = 10u;
    const std::string commonTopic = "Topic";

    testParam_numMsgToPublishPerController = 100u;
    numMsgToReceiveTotal = testParam_numMsgToPublishPerController * numPub * numSub;
    testParam_messageSizeInBytes = 100;

    std::vector<std::string> topics;
    for (uint32_t i = 0; i < numPub; i++)
    {
        topics.push_back(commonTopic);
    }

    std::vector<TestParticipant> coordinatedSyncParticipantsSub;
    coordinatedSyncParticipantsSub.push_back({"CoordSub1", {}, topics, OperationMode::Coordinated, TimeMode::Sync});
    coordinatedSyncParticipantsSub.push_back({"CoordSub2", {}, topics, OperationMode::Coordinated, TimeMode::Sync});

    std::vector<TestParticipant> autonomousAsyncParticipantsPub;
    autonomousAsyncParticipantsPub.push_back({"Pub", topics, {}, OperationMode::Autonomous, TimeMode::Async});
    autonomousAsyncParticipantsPub.back().SetPublishInCommReady(true);
    autonomousAsyncParticipantsPub.back().SetWaitForCommunicationInCommReadyHandler(true);

    try
    {

        auto registryUri = SetupSystem("silkit://localhost:0", coordinatedSyncParticipantsSub);

        // Start the coordinated participants
        RunParticipantThreads(coordinatedSyncParticipantsSub, registryUri);
        Log() << ">> Await sim task";
        // AwaitRunning is set in the simtask and used here to await the simtask
        for (auto& p : coordinatedSyncParticipantsSub)
            p.AwaitRunning();

        // Start the publishers
        RunParticipantThreads(autonomousAsyncParticipantsPub, registryUri);

        Log() << ">> Await communication";
        for (auto& p : coordinatedSyncParticipantsSub)
            p.AwaitCommunication();

        systemController.lifecycleService->Stop("End of test");
        auto futureStatus = finalStateSystemMaster.wait_for(communicationTimeout);
        EXPECT_EQ(futureStatus, std::future_status::ready)
            << "Test Failure: Awaiting final state for system master timed out";

        for (auto& p : autonomousAsyncParticipantsPub)
        {
            p.lifecycleService->Stop("End test");
        }

        Log() << ">> Joining participant threads";
        JoinParticipantThreads();
        SystemCleanup();
    }
    catch (const std::exception& error)
    {
        std::stringstream ss;
        ss << "Something went wrong: " << error.what();
        AbortAndFailTest(ss.str());
    }
}

// Setup lots of publishers and 1 subscriber on each side that both sides are busy doing the subscription handshakes.
// The CommunicationReadyHandler should be delayed between CommunicationInitializing and CommunicationInitialized
// until all handshakes are done. E.g., with numPub = 10000:
//[2022-07-27 14:34:44.680] [PubSub1] [info] New ParticipantState: CommunicationInitializing; reason: Received SystemState::ServicesCreated
//[2022-07-27 14:35:16.269] [PubSub1] [info] New ParticipantState: CommunicationInitialized; reason: Received SystemState::CommunicationInitializing
TEST_F(ITest_CommunicationGuarantees, test_delay_comm_ready_handler)
{
    const uint32_t numPub = 100u;
    const uint32_t numSub = 1u;
    const std::string topic1 = "Topic1";
    const std::string topic2 = "Topic2";

    testParam_numMsgToPublishPerController = 2u;
    numMsgToReceiveTotal = testParam_numMsgToPublishPerController * numPub * numSub;
    testParam_messageSizeInBytes = 100;

    std::vector<std::string> pubTopics1;
    for (uint32_t i = 0; i < numPub; i++)
    {
        pubTopics1.push_back(topic1);
    }

    std::vector<std::string> pubTopics2;
    for (uint32_t i = 0; i < numPub; i++)
    {
        pubTopics2.push_back(topic2);
    }

    std::vector<TestParticipant> participants;
    participants.push_back({"PubSub1", pubTopics1, {topic2}, OperationMode::Coordinated, TimeMode::Sync});
    participants.back().SetPublishInCommReady(true);
    participants.back().SetWaitForCommunicationInCommReadyHandler(true);

    participants.push_back({"PubSub2", pubTopics2, {topic1}, OperationMode::Coordinated, TimeMode::Sync});
    participants.back().SetPublishInCommReady(true);
    participants.back().SetWaitForCommunicationInCommReadyHandler(true);

    ExecuteCommReadyTest(participants);
}

/*
 * Test that all data published in the stop handler is received by the time
 * the participant received their final state.
 **/
TEST_F(ITest_CommunicationGuarantees, test_communication_in_stop_handler)
{
    ExecuteHandlerTest("Stop");
}

/*
 * Test that all data published in the shutdown handler is received by the time
 * the participant received their final state.
 **/
TEST_F(ITest_CommunicationGuarantees, test_communication_in_shutdown_handler)
{
    ExecuteHandlerTest("Shutdown");
}

/*
 * Test that all data published in the aborting handler is received by the time
 * the participant received their final state.
 **/
TEST_F(ITest_CommunicationGuarantees, test_communication_in_abort_handler)
{
    ExecuteHandlerTest("Abort");
}


} // anonymous namespace
