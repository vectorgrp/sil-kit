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
#include "silkit/services/orchestration/all.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"
#include "silkit/services/all.hpp"
#include "silkit/services/pubsub/PubSubSpec.hpp"

#include "ConfigurationTestUtils.hpp"
#include "IParticipantInternal.hpp"
#include "CreateParticipantImpl.hpp"

namespace {

using namespace std::chrono_literals;
using namespace SilKit;
using namespace SilKit::Services::Orchestration;
using namespace SilKit::Config;
using namespace SilKit::Services::PubSub;

const std::string systemMasterName{"SystemMaster"};
const std::string topic{"Topic"};
const std::string mediaType{ "A" };
static size_t numParticipants;
std::chrono::milliseconds communicationTimeout{8000ms};
std::chrono::milliseconds asyncDelayBetweenPublication{50ms};

class ITest_HopOnHopOff : public testing::Test
{

protected:
    ITest_HopOnHopOff()
    {
    }

    struct TestParticipant
    {
        TestParticipant(const std::string& newName)
        {
            name = newName;
            id = static_cast<uint8_t>(numParticipants++);
        }

        TestParticipant(TestParticipant&&) = default;
        TestParticipant& operator=(TestParticipant&&) = default;

        struct ImmovableMembers
        {
            ImmovableMembers() = default;

            ImmovableMembers(ImmovableMembers&& other) noexcept
                : allReceived{other.allReceived.load()}
                , runAsync{other.runAsync.load()}
            {
            }

            ImmovableMembers& operator=(ImmovableMembers&& other) noexcept
            {
                if (this != &other)
                {
                    allReceived = other.allReceived.load();
                    runAsync = other.runAsync.load();
                }

                return *this;
            }

            std::atomic<bool> allReceived{false};
            std::atomic<bool> runAsync{true};
        };

        ImmovableMembers i{};

        std::string                  name;
        uint8_t id;
        std::unique_ptr<IParticipant> participant;
        IDataPublisher* publisher;
        IDataSubscriber* subscriber;
        std::set<uint8_t> receivedIds;
        std::promise<void>           allReceivedPromise;

        bool simtimePassed{false};
        std::promise<void>           simtimePassedPromise;

        SilKit::Services::Orchestration::ILifecycleService* lifecycleService{nullptr};

        void ResetReception()
        {
            receivedIds.clear();
            allReceivedPromise = std::promise<void>{};
            i.allReceived = false;
        }

        void AwaitCommunication() 
        {
            auto futureStatus = allReceivedPromise.get_future().wait_for(communicationTimeout);
            EXPECT_EQ(futureStatus, std::future_status::ready)
                << "Test Failure: Awaiting test communication timed out";
        }
    };

    struct SystemMaster
    {
        std::unique_ptr<IParticipant> participant;
        SilKit::Experimental::Services::Orchestration::ISystemController* systemController;
        ISystemMonitor*               systemMonitor;
        ILifecycleService* lifecycleService;
        std::future<ParticipantState> finalState;

        std::promise<void> systemStateRunningPromise;
        std::future<void> systemStateRunning;
    };

    void SystemStateHandler(SystemState newState)
    {
        switch (newState)
        {
        case SystemState::Error:
            std::cout << "SystemState::Error -> Aborting simulation" << std ::endl; 
            systemMaster.systemController->AbortSimulation();
            break;

        case SystemState::Running:
            systemMaster.systemStateRunningPromise.set_value();
            break;

        default: break;
        }
    }

    void AbortAndFailTest(const std::string& reason)
    {
        if (systemMaster.systemController != nullptr)
        {
            systemMaster.systemController->AbortSimulation();
        }
        FAIL() << reason;
    }

    void SyncParticipantThread(TestParticipant& participant, const std::string& registryUri)
    {
        try
        {
            participant.participant =
                SilKit::CreateParticipant(SilKit::Config::MakeEmptyParticipantConfiguration(), participant.name, registryUri);

            auto* lifecycleService = participant.participant->CreateLifecycleService(
                {SilKit::Services::Orchestration::OperationMode::Coordinated});
            auto* timeSyncService = lifecycleService->CreateTimeSyncService();

            SilKit::Services::PubSub::PubSubSpec dataSpec{topic, mediaType};
            SilKit::Services::PubSub::PubSubSpec matchingDataSpec{topic, mediaType};
            participant.publisher = participant.participant->CreateDataPublisher("TestPublisher", dataSpec, 0);
            participant.subscriber = participant.participant->CreateDataSubscriber(
                "TestSubscriber", matchingDataSpec,
                [&participant](IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent) {
                    if (!participant.i.allReceived)
                    {
                        participant.receivedIds.insert(dataMessageEvent.data[0]);
                        // No self delivery: Expect numParticipants-1 receptions
                        if (participant.receivedIds.size() == numParticipants-1)
                        {
                            participant.i.allReceived = true;
                            participant.allReceivedPromise.set_value();
                        }
                    }
                });

            timeSyncService->SetSimulationStepHandler(
                [&participant, this](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                    participant.publisher->Publish(std::vector<uint8_t>{participant.id});
                    if (!participant.simtimePassed && now > simtimeToPass)
                    {
                        participant.simtimePassed = true;
                        participant.simtimePassedPromise.set_value();
                    }
                }, 1s);
            auto finalStateFuture = lifecycleService->StartLifecycle();
            finalStateFuture.get();
        }
        catch (const SilKit::ConfigurationError& error)
        {
            std::stringstream ss;
            ss << "Invalid configuration: " << error.what() << std::endl;
            AbortAndFailTest(ss.str());
        }
        catch (const std::exception& error)
        {
            std::stringstream ss;
            ss << "Something went wrong: " << error.what() << std::endl;
            AbortAndFailTest(ss.str());
        }

    }

    void AsyncParticipantThread(TestParticipant& participant, const std::string& registryUri)
    {
        try
        {
            participant.participant =
                SilKit::CreateParticipant(SilKit::Config::MakeEmptyParticipantConfiguration(), participant.name, registryUri);
            SilKit::Services::PubSub::PubSubSpec dataSpec{topic, mediaType};
            SilKit::Services::PubSub::PubSubSpec matchingDataSpec{topic, mediaType};
            participant.publisher = participant.participant->CreateDataPublisher("TestPublisher", dataSpec, 0);
            participant.subscriber = participant.participant->CreateDataSubscriber(
                "TestSubscriber", matchingDataSpec,
                [&participant](IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent) {
                    if (!participant.i.allReceived)
                    {
                        auto participantId = dataMessageEvent.data[0];
                        if (participantId != participant.id)
                        {
                            participant.receivedIds.insert(dataMessageEvent.data[0]);
                            // No self delivery: Expect numParticipants-1 receptions
                            if (participant.receivedIds.size() == numParticipants - 1)
                            {
                                participant.i.allReceived = true;
                                participant.allReceivedPromise.set_value();
                            }
                        }
                    }
                });

            while (participant.i.runAsync)
            {
                participant.publisher->Publish(std::vector<uint8_t>{participant.id});
                std::this_thread::sleep_for(asyncDelayBetweenPublication);
            }

        }
        catch (const SilKit::ConfigurationError& error)
        {
            std::stringstream ss;
            ss << "Invalid configuration: " << error.what() << std::endl;
            AbortAndFailTest(ss.str());
        }
        catch (const std::exception& error)
        {
            std::stringstream ss;
            ss << "Something went wrong: " << error.what() << std::endl;
            AbortAndFailTest(ss.str());
        }

        // Explicitly delete the com adapter to end the async participant
        participant.participant.reset();
    }

    void RunRegistry(const std::string& registryUri)
    {
        try
        {
            registry = SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::MakeParticipantConfigurationWithLogging(Services::Logging::Level::Info));
            registry->StartListening(registryUri);
        }
        catch (const SilKit::ConfigurationError& error)
        {
            std::stringstream ss;
            ss << "Invalid configuration: " << error.what() << std::endl;
            AbortAndFailTest(ss.str());
        }
        catch (const std::exception& error)
        {
            std::stringstream ss;
            ss << "Something went wrong: " << error.what() << std::endl;
            AbortAndFailTest(ss.str());
        }
    }

    void RunSystemMaster(const std::string& registryUri)
    {
        try
        {
            systemMaster.participant = SilKit::CreateParticipantImpl(
                SilKit::Config::MakeEmptyParticipantConfigurationImpl(), systemMasterName, registryUri);

            auto participantInternal =
                dynamic_cast<SilKit::Core::IParticipantInternal*>(systemMaster.participant.get());
            systemMaster.systemController = participantInternal->GetSystemController();

            systemMaster.systemMonitor = systemMaster.participant->CreateSystemMonitor();
            systemMaster.lifecycleService = systemMaster.participant->CreateLifecycleService(
                {SilKit::Services::Orchestration::OperationMode::Coordinated});

            systemMaster.systemController->SetWorkflowConfiguration({syncParticipantNames});

            systemMaster.systemMonitor->AddSystemStateHandler([this](SystemState newState) {
                SystemStateHandler(newState);
            });

            systemMaster.systemStateRunning = systemMaster.systemStateRunningPromise.get_future();

            systemMaster.finalState = systemMaster.lifecycleService->StartLifecycle();
        }
        catch (const SilKit::ConfigurationError& error)
        {
            std::stringstream ss;
            ss << "Invalid configuration: " << error.what() << std::endl;
            AbortAndFailTest(ss.str());
        }
        catch (const std::exception& error)
        {
            std::stringstream ss;
            ss << "Something went wrong: " << error.what() << std::endl;
            AbortAndFailTest(ss.str());
        }
    }

    void RunSyncParticipants(std::vector<TestParticipant>& participants, const std::string& registryUri)
    {
        try
        {
            for (auto& p : participants)
            {
                syncParticipantThreads.emplace_back(
                    [this, &p, registryUri] { SyncParticipantThread(p, registryUri); });
            }

        }
        catch (const SilKit::ConfigurationError& error)
        {
            std::stringstream ss;
            ss << "Invalid configuration: " << error.what() << std::endl;
            AbortAndFailTest(ss.str());
        }
        catch (const std::exception& error)
        {
            std::stringstream ss;
            ss << "Something went wrong: " << error.what() << std::endl;
            AbortAndFailTest(ss.str());
        }
    }

    void RunAsyncParticipants(std::vector<TestParticipant>& participants, const std::string& registryUri)
    {
        try
        {
            for (auto& p : participants)
            {
                p.i.runAsync = true;
                asyncParticipantThreads.emplace_back([this, &p, registryUri] { AsyncParticipantThread(p, registryUri); });
            }
        }
        catch (const SilKit::ConfigurationError& error)
        {
            std::stringstream ss;
            ss << "Invalid configuration: " << error.what() << std::endl;
            AbortAndFailTest(ss.str());
        }
        catch (const std::exception& error)
        {
            std::stringstream ss;
            ss << "Something went wrong: " << error.what() << std::endl;
            AbortAndFailTest(ss.str());
        }
    }

    void JoinSyncParticipantsThreads()
    {
        for (auto&& thread : syncParticipantThreads)
        {
            thread.join();
        }
        syncParticipantThreads.clear();
    }

    void JoinAsyncParticipantsThreads()
    {
        for (auto&& thread : asyncParticipantThreads)
        {
            thread.join();
        }
        asyncParticipantThreads.clear();
    }

    void SetupSystem(const std::string& registryUri, std::vector<TestParticipant>& syncParticipants)
    {
        for (auto&& p : syncParticipants)
        {
            syncParticipantNames.push_back(p.name);
        }
        syncParticipantNames.push_back(systemMasterName);

        RunRegistry(registryUri);
        RunSystemMaster(registryUri);
    }

    void ShutdownSystem()
    {
        asyncParticipantThreads.clear();
        syncParticipantThreads.clear();
        systemMaster.participant.reset();
        registry.reset();
    }

protected:
    std::vector<std::string> syncParticipantNames{};
    std::unique_ptr<SilKit::Vendor::Vector::ISilKitRegistry> registry;
    SystemMaster systemMaster;
    std::vector<std::thread> syncParticipantThreads;
    std::vector<std::thread> asyncParticipantThreads;

    std::chrono::seconds simtimeToPass{ 3s };
    bool runAsync{ true };


};


TEST_F(ITest_HopOnHopOff, test_Async_HopOnHopOff_ToSynced)
{
    numParticipants = 0;
    auto registryUri = MakeTestRegistryUri();

    std::vector<TestParticipant> syncParticipants;
    syncParticipants.push_back({ "SyncParticipant1" });
    syncParticipants.push_back({ "SyncParticipant2" });

    std::vector<TestParticipant> asyncParticipants;
    asyncParticipants.push_back({ "AsyncParticipant1" });
    asyncParticipants.push_back({ "AsyncParticipant2" });

    SetupSystem(registryUri, syncParticipants);

    RunSyncParticipants(syncParticipants, registryUri);

    std::cout << "Await simtime progress" << std::endl;
    // Await simtime progress
    for (auto& p : syncParticipants)
    {
        auto futureStatus = p.simtimePassedPromise.get_future().wait_for(communicationTimeout);
        EXPECT_EQ(futureStatus, std::future_status::ready)
            << "Test Failure: Sync participants didn't achieve sim time barrier";
    }

    for (int i = 0; i < 3; i++)
    {
        std::cout << ">> Cycle " << i + 1 << "/3" << std::endl;
        std::cout << ">> Hop on async participants" << std::endl;

        // Hop on with async participant
        RunAsyncParticipants(asyncParticipants, registryUri);

        std::cout << ">> Await successful communication of async/sync participants" << std::endl;
        // Await successful communication of async/sync participants
        for (auto& p : syncParticipants)
            p.AwaitCommunication();
        for (auto& p : asyncParticipants)
            p.AwaitCommunication();

        std::cout << ">> Hop off async participants" << std::endl;
        // Hop off: Stop while-loop of async participants
        for (auto& p : asyncParticipants)
            p.i.runAsync = false;
        JoinAsyncParticipantsThreads();

        // Reset communication and wait for reception once more for remaining sync participants
        numParticipants = syncParticipants.size();
        for (auto& p : syncParticipants)
            p.ResetReception();

        std::cout << ">> Await successful communication of remaining sync participants" << std::endl;
        for (auto& p : syncParticipants)
            p.AwaitCommunication();

        // Reset communication to repeat the cycle
        numParticipants = syncParticipants.size() + asyncParticipants.size();
        for (auto& p : syncParticipants)
            p.ResetReception();
        for (auto& p : asyncParticipants)
            p.ResetReception();
    }

    std::cout << ">> Cycles done" << std::endl;
    ASSERT_EQ(systemMaster.systemStateRunning.wait_for(1s), std::future_status::ready);
    systemMaster.lifecycleService->Stop("Stop Test.");

    JoinSyncParticipantsThreads();

    ShutdownSystem();
}


TEST_F(ITest_HopOnHopOff, test_Async_reconnect_first_joined)
{
    numParticipants = 0;
    auto registryUri = MakeTestRegistryUri();

    std::vector<TestParticipant> asyncParticipants1;
    asyncParticipants1.push_back({"AsyncParticipant1"});
    std::vector<TestParticipant> asyncParticipants2;
    asyncParticipants2.push_back({"AsyncParticipant2"});

    // No lifecylce, only registry needed (no systemController)
    RunRegistry(registryUri);

    for (int i = 0; i < 3; i++)
    {
        // Start with asyncParticipant1
        RunAsyncParticipants(asyncParticipants1, registryUri);
        // Async2 is second
        RunAsyncParticipants(asyncParticipants2, registryUri);

        // Await successful communication of async participants
        for (auto& p : asyncParticipants1)
            p.AwaitCommunication();
        for (auto& p : asyncParticipants2)
            p.AwaitCommunication();

        // Reset communication
        numParticipants = 2;
        for (auto& p : asyncParticipants1)
            p.ResetReception();
        for (auto& p : asyncParticipants2)
            p.ResetReception();

        // Hop off with asyncParticipants1
        asyncParticipants1[0].i.runAsync = false;
        asyncParticipantThreads[0].join();
        asyncParticipantThreads.erase(asyncParticipantThreads.begin());

        // Reconnect with asyncParticipant1
        RunAsyncParticipants(asyncParticipants1, registryUri);

        // Await successful communication of async participants
        for (auto& p : asyncParticipants1)
            p.AwaitCommunication();
        for (auto& p : asyncParticipants2)
            p.AwaitCommunication();

        // Reset communication
        numParticipants = 2;
        for (auto& p : asyncParticipants1)
            p.ResetReception();
        for (auto& p : asyncParticipants2)
            p.ResetReception();

        // Disconnect with both
        for (auto& p : asyncParticipants1)
            p.i.runAsync = false;
        for (auto& p : asyncParticipants2)
            p.i.runAsync = false;
        JoinAsyncParticipantsThreads();
    }

    ShutdownSystem();
}


TEST_F(ITest_HopOnHopOff, test_Async_reconnect_second_joined)
{
    numParticipants = 0;
    auto registryUri = MakeTestRegistryUri();

    std::vector<TestParticipant> asyncParticipants1;
    asyncParticipants1.push_back({"AsyncParticipant1"});
    std::vector<TestParticipant> asyncParticipants2;
    asyncParticipants2.push_back({"AsyncParticipant2"});

    // No lifecylce, only registry needed (no systemController)
    RunRegistry(registryUri);

    for (int i = 0; i < 3; i++)
    {
        // Start with asyncParticipant1
        RunAsyncParticipants(asyncParticipants1, registryUri);
        // Async2 is second
        RunAsyncParticipants(asyncParticipants2, registryUri);

        // Await successful communication of async participants
        for (auto& p : asyncParticipants1)
            p.AwaitCommunication();
        for (auto& p : asyncParticipants2)
            p.AwaitCommunication();

        // Reset communication
        numParticipants = 2;
        for (auto& p : asyncParticipants1)
            p.ResetReception();
        for (auto& p : asyncParticipants2)
            p.ResetReception();

        // Hop off with asyncParticipants2
        asyncParticipants2[0].i.runAsync = false;
        asyncParticipantThreads[1].join();
        asyncParticipantThreads.erase(asyncParticipantThreads.begin()+1);

        // Reconnect with asyncParticipant2
        RunAsyncParticipants(asyncParticipants2, registryUri);

        // Await successful communication of async participants
        for (auto& p : asyncParticipants1)
            p.AwaitCommunication();
        for (auto& p : asyncParticipants2)
            p.AwaitCommunication();

        // Reset communication
        numParticipants = 2;
        for (auto& p : asyncParticipants1)
            p.ResetReception();
        for (auto& p : asyncParticipants2)
            p.ResetReception();

        // Disconnect with both
        for (auto& p : asyncParticipants1)
            p.i.runAsync = false;
        for (auto& p : asyncParticipants2)
            p.i.runAsync = false;
        JoinAsyncParticipantsThreads();
    }

    ShutdownSystem();
}


TEST_F(ITest_HopOnHopOff, test_Async_HopOnHopOff_ToEmpty)
{
    numParticipants = 0;
    auto registryUri = MakeTestRegistryUri();

    std::vector<TestParticipant> syncParticipants;
    std::vector<TestParticipant> asyncParticipants;
    asyncParticipants.push_back({ "AsyncParticipant1" });
    asyncParticipants.push_back({ "AsyncParticipant2" });

    SetupSystem(registryUri, syncParticipants);

    for (int i = 0; i < 3; i++)
    {
        // Hop on with async participant on empty sim
        RunAsyncParticipants(asyncParticipants, registryUri);

        // Await successful communication of async/sync participants
        for (auto& p : asyncParticipants)
            p.AwaitCommunication();

        // Hop off async participants
        for (auto& p : asyncParticipants)
            p.i.runAsync = false;
        JoinAsyncParticipantsThreads();

        // Reset communication to repeat the cycle
        for (auto& p : asyncParticipants)
            p.ResetReception();
    }

    ShutdownSystem();
}

} // anonymous namespace
