// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include "GetTestPid.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"


#include "ib/IntegrationBus.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/vendor/CreateIbRegistry.hpp"
#include "ib/sim/all.hpp"

#include "MockParticipantConfiguration.hpp"

namespace {

using namespace std::chrono_literals;
using namespace ib::mw;
using namespace ib::mw::sync;
using namespace ib::cfg;
using namespace ib::sim::data;

const std::string systemMasterName{"SystemMaster"};
const std::string pubName{"Pub"};
const std::string subName{"Sub"};
const std::string topic{"Topic"};
const std::string mediaType{ "A" };
static size_t numParticipants;
std::chrono::milliseconds communicationTimeout{2000ms};
std::chrono::milliseconds asyncDelayBetweenPublication{50ms};

class HopOnHopOffITest : public testing::Test
{

protected:
    HopOnHopOffITest()
    {
    }

    struct TestParticipant
    {
        TestParticipant(const std::string& newName)
        {
            name = newName;
            id = static_cast<uint8_t>(numParticipants++);
        }
        std::string                  name;
        uint8_t id;
        std::unique_ptr<IParticipant> participant;
        IDataPublisher* publisher;
        IDataSubscriber* subscriber;
        std::set<uint8_t> receivedIds;
        bool allReceived{ false };
        std::promise<void>           allReceivedPromise;

        bool simtimePassed{false};
        std::promise<void>           simtimePassedPromise;

        void ResetReception()
        {
            receivedIds.clear();
            allReceivedPromise = std::promise<void>{};
            allReceived = false;
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
        ISystemController*           systemController;
        ISystemMonitor*              systemMonitor;
    };

    void ParticipantStatusHandler(const ParticipantStatus& newStatus)
    {
        switch (newStatus.state)
        {
        case ParticipantState::Error:
            //systemMaster.systemController->Shutdown();
            break;

        default:
            break;
        }
    }

    void SystemStateHandler(SystemState newState)
    {
        switch (newState)
        {
        case SystemState::Idle:
            for (auto&& name : syncParticipantNames)
            {
                if (name == systemMasterName)
                    continue;
                systemMaster.systemController->Initialize(name);
            }
            break;

        case SystemState::Initialized:
            systemMaster.systemController->Run();
            break;

        case SystemState::Stopped:
            systemMaster.systemController->Shutdown();
            break;

        case SystemState::Error:
            systemMaster.systemController->Shutdown();
            break;

        default:
            break;
        }
    }

    void ShutdownAndFailTest(const std::string& reason)
    {
        systemMaster.systemController->Shutdown();
        FAIL() << reason;
    }

    void SyncParticipantThread(TestParticipant& participant, uint32_t domainId)
    {
        try
        {
            participant.participant =
                ib::CreateParticipant(ib::cfg::MockParticipantConfiguration(), participant.name, domainId, true);

            auto* lifecycleService = participant.participant->GetLifecycleService();
            auto* timeSyncService = lifecycleService->GetTimeSyncService();

            participant.publisher = participant.participant->CreateDataPublisher("TestPublisher", topic, mediaType, {}, 0);
            participant.subscriber = participant.participant->CreateDataSubscriber(
				"TestSubscriber", topic, mediaType, {},
                [&participant](IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent) {
                    if (!participant.allReceived)
                    {
                        participant.receivedIds.insert(dataMessageEvent.data[0]);
                        // No self delivery: Expect numParticipants-1 receptions
                        if (participant.receivedIds.size() == numParticipants-1)
                        {
                            participant.allReceived = true;
                            participant.allReceivedPromise.set_value();
                        }
                    }
                },
                nullptr);

            timeSyncService->SetPeriod(1s);
            timeSyncService->SetSimulationTask(
                [&participant, this](std::chrono::nanoseconds now) {
                    participant.publisher->Publish(std::vector<uint8_t>{participant.id});
                    if (!participant.simtimePassed && now > simtimeToPass)
                    {
                        participant.simtimePassed = true;
                        participant.simtimePassedPromise.set_value();
                    }
                });
            auto finalStateFuture = lifecycleService->ExecuteLifecycleNoSyncTime(false, false);
            finalStateFuture.get();
        }
        catch (const ib::ConfigurationError& error)
        {
            std::stringstream ss;
            ss << "Invalid configuration: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
        catch (const std::exception& error)
        {
            std::stringstream ss;
            ss << "Something went wrong: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }

    }

    void AsyncParticipantThread(TestParticipant& participant, uint32_t domainId)
    {
        try
        {
            participant.participant =
                ib::CreateParticipant(ib::cfg::MockParticipantConfiguration(), participant.name, domainId, false);
            participant.publisher = participant.participant->CreateDataPublisher("TestPublisher", topic, mediaType, {}, 0);
            participant.subscriber = participant.participant->CreateDataSubscriber(
                "TestSubscriber", topic, mediaType, {},
                [&participant](IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent) {
                    if (!participant.allReceived)
                    {
                        participant.receivedIds.insert(dataMessageEvent.data[0]);
                        // No self delivery: Expect numParticipants-1 receptions
                        if (participant.receivedIds.size() == numParticipants - 1)
                        {
                            participant.allReceived = true;
                            participant.allReceivedPromise.set_value();
                        }
                    }
                },
                nullptr);

            while (runAsync)
            {
                participant.publisher->Publish(std::vector<uint8_t>{participant.id});
                std::this_thread::sleep_for(asyncDelayBetweenPublication);
            }

        }
        catch (const ib::ConfigurationError& error)
        {
            std::stringstream ss;
            ss << "Invalid configuration: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
        catch (const std::exception& error)
        {
            std::stringstream ss;
            ss << "Something went wrong: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }

        // Explicitly delete the com adapter to end the async participant
        participant.participant.reset();
    }

    void RunRegistry(uint32_t domainId)
    {
        try
        {
            registry = ib::vendor::CreateIbRegistry(ib::cfg::MockParticipantConfiguration());
            registry->ProvideDomain(domainId);
        }
        catch (const ib::ConfigurationError& error)
        {
            std::stringstream ss;
            ss << "Invalid configuration: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
        catch (const std::exception& error)
        {
            std::stringstream ss;
            ss << "Something went wrong: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
    }

    void RunSystemMaster(uint32_t domainId)
    {
        try
        {
            systemMaster.participant =
                ib::CreateParticipant(ib::cfg::MockParticipantConfiguration(), systemMasterName, domainId, false);

            systemMaster.systemController = systemMaster.participant->GetSystemController();
            systemMaster.systemMonitor = systemMaster.participant->GetSystemMonitor();

            systemMaster.systemController->SetRequiredParticipants(syncParticipantNames);

            systemMaster.systemMonitor->RegisterSystemStateHandler(
                [this](SystemState newState) { SystemStateHandler(newState); });

            systemMaster.systemMonitor->RegisterParticipantStatusHandler(
                [this](const ParticipantStatus& newStatus) {
                    ParticipantStatusHandler(newStatus);
                });
        }
        catch (const ib::ConfigurationError& error)
        {
            std::stringstream ss;
            ss << "Invalid configuration: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
        catch (const std::exception& error)
        {
            std::stringstream ss;
            ss << "Something went wrong: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
    }

    void RunSyncParticipants(std::vector<TestParticipant>& participants, uint32_t domainId)
    {
        try
        {
            for (auto& p : participants)
            {
                syncParticipantThreads.emplace_back(
                    [this, &p, domainId] { SyncParticipantThread(p, domainId); });
            }

        }
        catch (const ib::ConfigurationError& error)
        {
            std::stringstream ss;
            ss << "Invalid configuration: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
        catch (const std::exception& error)
        {
            std::stringstream ss;
            ss << "Something went wrong: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
    }

    void RunAsyncParticipants(std::vector<TestParticipant>& participants, uint32_t domainId)
    {
        runAsync = true;

        try
        {
            for (auto& p : participants)
            {
                asyncParticipantThreads.emplace_back([this, &p, domainId] { AsyncParticipantThread(p, domainId); });
            }
        }
        catch (const ib::ConfigurationError& error)
        {
            std::stringstream ss;
            ss << "Invalid configuration: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
        catch (const std::exception& error)
        {
            std::stringstream ss;
            ss << "Something went wrong: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
    }

    void StopSyncParticipants()
    {
        for (auto&& thread : syncParticipantThreads)
        {
            thread.join();
        }
        syncParticipantThreads.clear();
    }

    void StopAsyncParticipants()
    {
        runAsync = false;
        for (auto&& thread : asyncParticipantThreads)
        {
            thread.join();
        }
        asyncParticipantThreads.clear();
    }

    void SetupSystem(uint32_t domainId, std::vector<TestParticipant>& syncParticipants)
    {
        for (auto&& p : syncParticipants)
        {
            syncParticipantNames.push_back(p.name);
        }

        RunRegistry(domainId);
        RunSystemMaster(domainId);
        
    }

    void ShutdownSystem()
    {
        asyncParticipantThreads.clear();
        syncParticipantThreads.clear();
        systemMaster.participant.reset();
        registry.reset();
    }

protected:
    std::vector<std::string> syncParticipantNames;
    std::unique_ptr<ib::vendor::IIbRegistry> registry;
    SystemMaster systemMaster;
    std::vector<std::thread> syncParticipantThreads;
    std::vector<std::thread> asyncParticipantThreads;

    std::chrono::seconds simtimeToPass{ 3s };
    bool runAsync{ true };


};

#if defined(IB_MW_HAVE_VASIO)

TEST_F(HopOnHopOffITest, test_Async_HopOnHopOff_ToSynced)
{
    numParticipants = 0;
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    std::vector<TestParticipant> syncParticipants;
    syncParticipants.push_back({ "SyncParticipant1" });
    syncParticipants.push_back({ "SyncParticipant2" });

    std::vector<TestParticipant> asyncParticipants;
    asyncParticipants.push_back({ "AsyncParticipant1" });
    asyncParticipants.push_back({ "AsyncParticipant2" });

    SetupSystem(domainId, syncParticipants);

    RunSyncParticipants(syncParticipants, domainId);

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
        RunAsyncParticipants(asyncParticipants, domainId);

        std::cout << ">> Await successful communication of async/sync participants" << std::endl;
        // Await successful communication of async/sync participants
        for (auto& p : syncParticipants)
            p.AwaitCommunication();
        for (auto& p : asyncParticipants)
            p.AwaitCommunication();

        std::cout << ">> Hop off async participants" << std::endl;
        // Hop off: Stop while-loop of async participants
        StopAsyncParticipants();

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

    // Stop sync participants
    systemMaster.systemController->Shutdown();
    StopSyncParticipants();

    ShutdownSystem();
}


TEST_F(HopOnHopOffITest, test_Async_HopOnHopOff_ToEmpty)
{
    numParticipants = 0;
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    std::vector<TestParticipant> syncParticipants;
    std::vector<TestParticipant> asyncParticipants;
    asyncParticipants.push_back({ "AsyncParticipant1" });
    asyncParticipants.push_back({ "AsyncParticipant2" });

    SetupSystem(domainId, syncParticipants);

    for (int i = 0; i < 3; i++)
    {
        // Hop on with async participant on empty sim
        RunAsyncParticipants(asyncParticipants, domainId);

        // Await successful communication of async/sync participants
        for (auto& p : asyncParticipants)
            p.AwaitCommunication();

        // Hop off async participants
        StopAsyncParticipants();

        // Reset communication to repeat the cycle
        for (auto& p : asyncParticipants)
            p.ResetReception();
    }

    ShutdownSystem();
}

#endif

} // anonymous namespace
