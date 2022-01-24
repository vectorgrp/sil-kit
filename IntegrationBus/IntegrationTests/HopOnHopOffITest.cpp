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
#include "ib/cfg/string_utils.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/sim/all.hpp"

#include "ib/cfg/Config.hpp"
#include "ib/cfg/ConfigBuilder.hpp"
#include "ib/extensions/CreateExtension.hpp"


namespace {

using namespace std::chrono_literals;
using namespace ib::mw;
using namespace ib::mw::sync;
using namespace ib::cfg;
using namespace ib::sim::generic;

const std::string systemMasterName{"SystemMaster"};
const std::string pubName{"Pub"};
const std::string subName{"Sub"};
const std::string linkName{"Link"};
static uint8_t    numParticipants;
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
            id = numParticipants++;
        }
        std::string                  name;
        uint8_t id;
        std::unique_ptr<IComAdapter> comAdapter;
        IGenericPublisher*           publisher;
        IGenericSubscriber*          subscriber;
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
        std::unique_ptr<IComAdapter> comAdapter;
        ISystemController*           systemController;
        ISystemMonitor*              systemMonitor;
    };

    auto BuildConfig(std::vector<TestParticipant>& syncParticipants, std::vector<TestParticipant>& asyncParticipants, Middleware middleware) -> Config
    {
        const auto level = logging::Level::Info;
        ConfigBuilder config("PubSubTestConfigGenerated");
        auto&& simulationSetup = config.SimulationSetup();

        std::vector<ParticipantBuilder*> participants;
        for (const auto& p : syncParticipants)
        {
            auto&& participant = simulationSetup.AddParticipant(p.name);
            participant.ConfigureLogger().WithFlushLevel(level).AddSink(Sink::Type::Stdout).WithLogLevel(level);
            participant.AddParticipantController().WithSyncType(SyncType::DistributedTimeQuantum);
            participant.AddGenericPublisher(pubName).WithLink(linkName);
            participant.AddGenericSubscriber(subName).WithLink(linkName);
            participants.emplace_back(&participant);
        }
        for (const auto& p : asyncParticipants)
        {
            auto&& participant = simulationSetup.AddParticipant(p.name);
            participant.ConfigureLogger().WithFlushLevel(level).AddSink(Sink::Type::Stdout).WithLogLevel(level);
            participant.AddGenericPublisher(pubName).WithLink(linkName);
            participant.AddGenericSubscriber(subName).WithLink(linkName);
            participants.emplace_back(&participant);
        }

        auto&& systemMasterParticipant = simulationSetup.AddParticipant(systemMasterName);
        systemMasterParticipant.ConfigureLogger().WithFlushLevel(level).AddSink(Sink::Type::Stdout).WithLogLevel(level);

        config.WithActiveMiddleware(middleware);
        return config.Build();
    }

    void ParticipantStatusHandler(const ParticipantStatus& newStatus)
    {
        switch (newStatus.state)
        {
        case ParticipantState::Error:
            systemMaster.systemController->Shutdown();
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
            for (auto&& participant : ibConfig.simulationSetup.participants)
            {
                if (participant.name == systemMasterName)
                    continue;
                systemMaster.systemController->Initialize(participant.name);
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
            participant.comAdapter = ib::CreateComAdapter(ibConfig, participant.name, domainId);

            IParticipantController* participantController;
            participantController = participant.comAdapter->GetParticipantController();
            participant.publisher = participant.comAdapter->CreateGenericPublisher(pubName);
            participant.subscriber = participant.comAdapter->CreateGenericSubscriber(subName);
            participant.subscriber->SetReceiveMessageHandler([&participant](IGenericSubscriber* subscriber, const std::vector<uint8_t>& data) {
                    if (!participant.allReceived)
                    {
                        participant.receivedIds.insert(data[0]);
                        if (participant.receivedIds.size() == numParticipants)
                        {
                            participant.allReceived = true;
                            participant.allReceivedPromise.set_value();
                        }
                    }
                });
            participantController->SetPeriod(1s);
            participantController->SetSimulationTask(
                [&participant, this](std::chrono::nanoseconds now) {
                    participant.publisher->Publish(std::vector<uint8_t>{participant.id});
                    if (!participant.simtimePassed && now > simtimeToPass)
                    {
                        participant.simtimePassed = true;
                        participant.simtimePassedPromise.set_value();
                    }
                });
            auto finalStateFuture = participantController->RunAsync();
            auto finalState = finalStateFuture.get();
        }
        catch (const Misconfiguration& error)
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
            participant.comAdapter = ib::CreateComAdapter(ibConfig, participant.name, domainId);

            participant.publisher = participant.comAdapter->CreateGenericPublisher(pubName);
            participant.subscriber = participant.comAdapter->CreateGenericSubscriber(subName);
            participant.subscriber->SetReceiveMessageHandler(
                [&participant](IGenericSubscriber* subscriber, const std::vector<uint8_t>& data) {
                    if (!participant.allReceived)
                    {
                        participant.receivedIds.insert(data[0]);
                        if (participant.receivedIds.size() == numParticipants)
                        {
                            participant.allReceived = true;
                            participant.allReceivedPromise.set_value();
                        }
                    }
                });

            while (runAsync)
            {
                participant.publisher->Publish(std::vector<uint8_t>{participant.id});
                std::this_thread::sleep_for(asyncDelayBetweenPublication);
            }

        }
        catch (const Misconfiguration& error)
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
        participant.comAdapter.reset();
    }

    void RunVasioRegistry(uint32_t domainId)
    {
        try
        {
            registry = ib::extensions::CreateIbRegistry(ibConfig);
            registry->ProvideDomain(domainId);
        }
        catch (const Misconfiguration& error)
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
            systemMaster.comAdapter = ib::CreateComAdapter(ibConfig, systemMasterName, domainId);
            systemMaster.systemController = systemMaster.comAdapter->GetSystemController();
            systemMaster.systemMonitor = systemMaster.comAdapter->GetSystemMonitor();

            systemMaster.systemMonitor->RegisterSystemStateHandler(
                [this](SystemState newState) { SystemStateHandler(newState); });

            systemMaster.systemMonitor->RegisterParticipantStatusHandler(
                [this](const ParticipantStatus& newStatus) {
                    ParticipantStatusHandler(newStatus);
                });
        }
        catch (const Misconfiguration& error)
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
        catch (const Misconfiguration& error)
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
        catch (const Misconfiguration& error)
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

    void SetupSystem(uint32_t domainId, std::vector<TestParticipant>& syncParticipants, std::vector<TestParticipant>& asyncParticipants, Middleware middleware)
    {
        ibConfig = BuildConfig(syncParticipants, asyncParticipants, middleware);
        if (middleware == Middleware::VAsio)
        {
            RunVasioRegistry(domainId);
        }

        RunSystemMaster(domainId);
        
    }

    void ShutdownSystem()
    {
        asyncParticipantThreads.clear();
        syncParticipantThreads.clear();
        systemMaster.comAdapter.reset();
        registry.reset();
    }

protected:
    ib::cfg::Config                              ibConfig;
    std::unique_ptr<ib::extensions::IIbRegistry> registry;
    SystemMaster                                 systemMaster;
    std::vector<std::thread>                     syncParticipantThreads;
    std::vector<std::thread>                     asyncParticipantThreads;

    std::chrono::seconds simtimeToPass{ 3s };
    bool runAsync{ true };


};

#if defined(IB_MW_HAVE_VASIO)

TEST_F(HopOnHopOffITest, test_Async_HopOnHopOff_ToSynced)
{
    numParticipants = 0;
    const auto middleware = Middleware::VAsio;
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    std::vector<TestParticipant> syncParticipants;
    syncParticipants.push_back({ "SyncParticipant1" });
    syncParticipants.push_back({ "SyncParticipant2" });

    std::vector<TestParticipant> asyncParticipants;
    asyncParticipants.push_back({ "AsyncParticipant1" });
    asyncParticipants.push_back({ "AsyncParticipant2" });

    SetupSystem(domainId, syncParticipants, asyncParticipants, middleware);

    RunSyncParticipants(syncParticipants, domainId);

    // Await simtime progress
    for (auto& p : syncParticipants)
    {
        auto futureStatus = p.simtimePassedPromise.get_future().wait_for(communicationTimeout);
        EXPECT_EQ(futureStatus, std::future_status::ready)
            << "Test Failure: Sync participants didn't achieve sim time barrier";
    }

    for (int i = 0; i < 3; i++)
    {
        // Hop on with async participant
        RunAsyncParticipants(asyncParticipants, domainId);

        // Await successful communication of async/sync participants
        for (auto& p : syncParticipants)
            p.AwaitCommunication();
        for (auto& p : asyncParticipants)
            p.AwaitCommunication();

        // Hop off: Stop while-loop of async participants
        StopAsyncParticipants();

        // Reset communication and wait for reception once more for remaining sync participants
        numParticipants = static_cast<uint8_t>(syncParticipants.size());
        for (auto& p : syncParticipants)
            p.ResetReception();
        for (auto& p : syncParticipants)
            p.AwaitCommunication();

        // Reset communication to repeat the cycle
        numParticipants = static_cast<uint8_t>(syncParticipants.size() + asyncParticipants.size());
        for (auto& p : syncParticipants)
            p.ResetReception();
        for (auto& p : asyncParticipants)
            p.ResetReception();

    }

    // Stop sync participants
    systemMaster.systemController->Stop();
    StopSyncParticipants();

    ShutdownSystem();
}


TEST_F(HopOnHopOffITest, test_Async_HopOnHopOff_ToEmpty)
{
    numParticipants = 0;
    const auto middleware = Middleware::VAsio;
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    std::vector<TestParticipant> syncParticipants;
    std::vector<TestParticipant> asyncParticipants;
    asyncParticipants.push_back({ "AsyncParticipant1" });
    asyncParticipants.push_back({ "AsyncParticipant2" });

    SetupSystem(domainId, syncParticipants, asyncParticipants, middleware);

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
