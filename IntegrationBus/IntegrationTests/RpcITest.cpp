// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <cstdlib>

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
using namespace ib::sim::rpc;

const std::string systemMasterName{ "SystemMaster" };

class RpcITest : public testing::Test
{

protected:
    RpcITest()
    {

    }

    struct RpcClientInfo
    {
        RpcClientInfo(const std::string& newFunctionName, const std::string& newMimeType, 
                          size_t newMessageSizeInBytes,  uint32_t newNumCalls, uint32_t newNumCallsToReturn)
        {
            expectIncreasingData = true;
            functionName = newFunctionName;
            dxf.mimeType = newMimeType;
            messageSizeInBytes = newMessageSizeInBytes;
            numCalls = newNumCalls;
            numCallsToReturn = newNumCallsToReturn;
        }
        RpcClientInfo(const std::string& newFunctionName, const std::string& newMimeType, size_t newMessageSizeInBytes,
                      uint32_t newNumCalls, uint32_t newNumCallsToReturn, const std::vector<std::vector<uint8_t>>& newExpectedReturnDataUnordered)
        {
            expectIncreasingData = false;
            functionName = newFunctionName;
            dxf.mimeType = newMimeType;
            messageSizeInBytes = newMessageSizeInBytes;
            numCalls = newNumCalls;
            numCallsToReturn = newNumCallsToReturn;
            expectedReturnDataUnordered = newExpectedReturnDataUnordered;
        }

        std::string                       functionName;
        RpcExchangeFormat                 dxf;
        size_t                            messageSizeInBytes;
        uint32_t                          numCalls;
        uint32_t                          numCallsToReturn;
        bool                              expectIncreasingData;
        std::vector<std::vector<uint8_t>> expectedReturnDataUnordered;
        uint32_t                          callCounter{0};
        uint32_t                          callReturnedCounter{0};
        bool                              allCalled{false};
        bool                              allCallsReturned{false};
        IRpcClient*                       rpcClient;
    };

    struct RpcServerInfo
    {
        RpcServerInfo(const std::string& newFunctionName, const std::string& newMimeType, size_t newMessageSizeInBytes,
                           uint32_t newNumCallsToReceive)
        {
            expectIncreasingData = true;
            functionName = newFunctionName;
            dxf.mimeType = newMimeType;
            messageSizeInBytes = newMessageSizeInBytes;
            numCallsToReceive = newNumCallsToReceive;
        }
        RpcServerInfo(const std::string& newFunctionName, const std::string& newMimeType, size_t newMessageSizeInBytes,
                           uint32_t newNumCallsToReceive, const std::vector<std::vector<uint8_t>>& newExpectedDataUnordered)
        {
            expectIncreasingData = false;
            functionName = newFunctionName;
            dxf.mimeType = newMimeType;
            messageSizeInBytes = newMessageSizeInBytes;
            numCallsToReceive = newNumCallsToReceive;
            expectedDataUnordered = newExpectedDataUnordered;
        }

        std::string                       functionName;
        RpcExchangeFormat                 dxf;
        size_t                            messageSizeInBytes;
        uint32_t                          numCallsToReceive;
        bool                              expectIncreasingData;
        std::vector<std::vector<uint8_t>> expectedDataUnordered;
        uint32_t                          receiveCallCounter{ 0 };
        bool                              allReceived{false};
        IRpcServer*                       rpcServer;
    };

    struct ClientParticipant
    {
        ClientParticipant(const std::string& newName) { name = newName; }
        ClientParticipant(const std::string& newName, const std::vector<RpcClientInfo>& newRpcClients)
        {
            name = newName;
            rpcClients = newRpcClients;
        }

        std::string                    name;
        std::vector<RpcClientInfo>     rpcClients;
        std::unique_ptr<IComAdapter>   comAdapter;
        std::promise<void>             startedPromise;
        bool                           allCalled{false};
        std::promise<void>             allCalledPromise;
        std::promise<void>             allCallsReturnedPromise;
        bool                           allCallsReturned{false};
    };
    struct ServerParticipant
    {
        ServerParticipant(const std::string& newName) { name = newName; }
        ServerParticipant(const std::string& newName, const std::vector<RpcServerInfo>& newRpcServers)
        {
            name = newName;
            rpcServers = newRpcServers;
        }

        std::string                     name;
        std::vector<RpcServerInfo>      rpcServers;
        std::unique_ptr<IComAdapter>    comAdapter;
        std::promise<void>              startedPromise;
        std::promise<void>              allReceivedPromise;
        bool                            allReceived{ false };

    };

    struct SystemMaster
    {
        std::unique_ptr<IComAdapter> comAdapter;
        ISystemController*           systemController;
        ISystemMonitor*              systemMonitor;
    };

    auto BuildConfig(std::vector<ClientParticipant>& clients, std::vector<ServerParticipant>& servers, Middleware middleware, bool sync) -> Config
    {
        const auto loglevel = logging::Level::Info;
        ConfigBuilder config("RpcTestConfigGenerated");
        auto&& simulationSetup = config.SimulationSetup();

        auto syncType = SyncType::Unsynchronized;
        if (sync)
        {
            syncType = SyncType::DistributedTimeQuantum;
        }

        uint32_t participantCount = static_cast<uint32_t>(clients.size() + servers.size());
        std::vector<ParticipantBuilder*> participants;
        for (const auto& client : clients)
        {
            for (const auto& c : client.rpcClients)
            {
                simulationSetup.AddOrGetLink(Link::Type::Rpc, c.functionName);
            }
            auto&& participant = simulationSetup.AddParticipant(client.name);
            participant.ConfigureLogger().WithFlushLevel(loglevel).AddSink(Sink::Type::Stdout).WithLogLevel(loglevel);
            if (sync)
            {
                participant.AddParticipantController().WithSyncType(syncType);
            }
            for (const auto& c : client.rpcClients)
            {
                participant.AddRpcClient(c.functionName).WithLink(c.functionName);
            }
            participants.emplace_back(&participant);
        }
        for (const auto& server : servers)
        {
            for (const auto& s : server.rpcServers)
            {
                simulationSetup.AddOrGetLink(Link::Type::Rpc, s.functionName);
            }
            auto&& participant = simulationSetup.AddParticipant(server.name);
            participant.ConfigureLogger().WithFlushLevel(loglevel).AddSink(Sink::Type::Stdout).WithLogLevel(loglevel);
            if (sync)
            {
                participant.AddParticipantController().WithSyncType(syncType);
            }
            for (const auto& s : server.rpcServers)
            {
                participant.AddRpcServer(s.functionName).WithLink(s.functionName);
            }
            participants.emplace_back(&participant);
        }

        auto&& systemMasterParticipant = simulationSetup.AddParticipant(systemMasterName);
        systemMasterParticipant.ConfigureLogger().WithFlushLevel(loglevel).AddSink(Sink::Type::Stdout).WithLogLevel(loglevel);

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

    void ClientThread(ClientParticipant& participant, uint32_t domainId, bool sync)
    {
        try
        {
            participant.comAdapter = ib::CreateComAdapter(ibConfig, participant.name, domainId);

            IParticipantController* participantController;
            if (sync)
            {
                participantController = participant.comAdapter->GetParticipantController();
            }

            for (auto& c : participant.rpcClients)
            {
                c.rpcClient = participant.comAdapter->CreateRpcClient(
                    c.functionName, c.dxf,
                    [this, &participant, &c](IRpcClient* client, IRpcCallHandle* callHandle,
                        const CallStatus callStatus, const std::vector<uint8_t>& returnData)
                    {
                        if (!c.allCallsReturned)
                        {
                            if (callStatus == CallStatus::Success)
                            {
                                if (c.expectIncreasingData)
                                {
                                    auto expectedData = std::vector<uint8_t>(c.messageSizeInBytes, c.callReturnedCounter + rpcFuncIncrement);
                                    EXPECT_EQ(returnData, expectedData);
                                }
                                else
                                {
                                    auto foundDataIter = std::find(c.expectedReturnDataUnordered.begin(),
                                        c.expectedReturnDataUnordered.end(), returnData);
                                    EXPECT_EQ(foundDataIter != c.expectedReturnDataUnordered.end(), true);
                                    if (foundDataIter != c.expectedReturnDataUnordered.end())
                                    {
                                        c.expectedReturnDataUnordered.erase(foundDataIter);
                                    }
                                }
                            }

                            c.callReturnedCounter++;
                            if (c.callReturnedCounter >= c.numCallsToReturn)
                            {
                                c.allCallsReturned = true;
                            }
                        }

                        if (!participant.allCallsReturned &&
                            std::all_of(participant.rpcClients.begin(), participant.rpcClients.end(),
                                [](RpcClientInfo c) { return c.allCallsReturned; }))
                        {
                            participant.allCallsReturned = true;
                            participant.allCallsReturnedPromise.set_value();
                        }
                    });
            }
            auto callTask = [this, participantController, &participant]() {
                for (auto& c : participant.rpcClients)
                {
                    if (!c.allCalled)
                    {
                        auto argumentData = std::vector<uint8_t>(c.messageSizeInBytes, c.callCounter);
                        auto callHandle = c.rpcClient->Call(argumentData);

                        c.callCounter++;
                        if (c.callCounter >= c.numCalls)
                        {
                            c.allCalled = true;
                        }
                    }
                }
            };

            if (sync)
            {
                participantController->SetPeriod(1s);
                participantController->SetSimulationTask([participantController, &participant, callTask](std::chrono::nanoseconds now)
                {
                    if (now >= 10s)
                    {
                        callTask();
                        if (!participant.allCalled &&
                            std::all_of(participant.rpcClients.begin(), participant.rpcClients.end(),
                                        [](RpcClientInfo c) { return c.allCalled; }))
                        {
                            participant.allCalled = true;
                            participant.allCalledPromise.set_value();
                        }
                    }
                });
                participant.startedPromise.set_value();
                auto finalStateFuture = participantController->RunAsync();
                auto finalState = finalStateFuture.get();
            }
            else
            {
                participant.startedPromise.set_value();
                while (std::none_of(participant.rpcClients.begin(), participant.rpcClients.end(),
                                   [](RpcClientInfo c) { return c.allCalled; }))
                {
                    // NB: For async, the position of the sleep (before or after callTask) is critical:
                    //  Before: Allows the discovery to complete and the server gets the first message
                    //  After:  The first message might get lost
                    // No way around that without counterpart discovery.
                    std::this_thread::sleep_for(asyncDelayBetweenCalls);
                    callTask();
                }
                participant.allCalledPromise.set_value();

                auto futureStatus = participant.allCallsReturnedPromise.get_future().wait_for(communicationTimeout);
                EXPECT_EQ(futureStatus, std::future_status::ready)
                    << "Test Failure: Awaiting call return timed out on async participant";
            }

            for (const auto& c : participant.rpcClients)
            {
                EXPECT_EQ(c.callCounter, c.numCalls);
            }
            for (const auto& c : participant.rpcClients)
            {
                EXPECT_EQ(c.callReturnedCounter, c.numCallsToReturn);
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

    void ServerThread(ServerParticipant& participant, uint32_t domainId, bool sync)
    {
        try
        {
            participant.comAdapter = ib::CreateComAdapter(ibConfig, participant.name, domainId);
            IParticipantController* participantController;
            if (sync)
            {
                participantController = participant.comAdapter->GetParticipantController();
            }

            for (auto& s : participant.rpcServers)
            {
                if (std::all_of(participant.rpcServers.begin(), participant.rpcServers.end(),
                    [](RpcServerInfo s) {
                    return s.numCallsToReceive == 0; }))
                {
                    participant.allReceived = true;
                    participant.allReceivedPromise.set_value();
                }

                s.rpcServer = participant.comAdapter->CreateRpcServer(s.functionName, s.dxf,
                    [this, sync, &participant, &s, participantController](IRpcServer*       server,
                                                                           IRpcCallHandle* callHandle,
                                                                           const std::vector<uint8_t>& argumentData)
                    {
                        std::vector<uint8_t> returnData{argumentData};
                        std::transform(std::begin(returnData), std::end(returnData), std::begin(returnData), [this](uint8_t x) {return x + rpcFuncIncrement; });

                        server->SubmitResult(callHandle, returnData);

                        if (!s.allReceived)
                        {
                            if (s.expectIncreasingData)
                            {
                                auto expectedData = std::vector<uint8_t>(s.messageSizeInBytes, s.receiveCallCounter);
                                EXPECT_EQ(argumentData, expectedData);
                            }
                            else
                            {
                                auto foundDataIter = std::find(s.expectedDataUnordered.begin(), s.expectedDataUnordered.end(), argumentData);
                                EXPECT_EQ(foundDataIter != s.expectedDataUnordered.end(), true);
                                if (foundDataIter != s.expectedDataUnordered.end()) {
                                    s.expectedDataUnordered.erase(foundDataIter);
                                }
                            }
                            
                            s.receiveCallCounter++;
                            if (s.receiveCallCounter >= s.numCallsToReceive)
                            {
                                s.allReceived = true;
                            }
                        }

                        if (!participant.allReceived && 
                            std::all_of(participant.rpcServers.begin(), participant.rpcServers.end(),
                                        [](RpcServerInfo s) { return s.allReceived; }))
                        {
                            participant.allReceived = true;
                            participant.allReceivedPromise.set_value();
                        }
                    }
                    );
            }

            if (sync)
            {
                participantController->SetPeriod(1s);
                participantController->SetSimulationTask([](std::chrono::nanoseconds now) {
                    });

                participant.startedPromise.set_value();
                auto finalStateFuture = participantController->RunAsync();
                auto finalState = finalStateFuture.get();
            }
            else
            {
                participant.startedPromise.set_value();
                auto futureStatus = participant.allReceivedPromise.get_future().wait_for(communicationTimeout);
                EXPECT_EQ(futureStatus, std::future_status::ready) << "Test Failure: Awaiting reception timed out on async participant";
            }

            for (const auto& s : participant.rpcServers)
            {
                EXPECT_EQ(s.receiveCallCounter, s.numCallsToReceive);
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

    void RunClients(std::vector<ClientParticipant>& clients, uint32_t domainId, bool sync)
    {
        try
        {
            for (auto& clientParticipant : clients)
            {
                rpcThreads.emplace_back(
                    [this, &clientParticipant, domainId, sync] { ClientThread(clientParticipant, domainId, sync); });
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

    void RunServers(std::vector<ServerParticipant>& servers, uint32_t domainId, bool sync)
    {
        try
        {
            for (auto& serverParticipant : servers)
            {
                rpcThreads.emplace_back(
                    [this, &serverParticipant, domainId, sync] { ServerThread(serverParticipant, domainId, sync); });
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

    void JoinRpcThreads()
    {
        for (auto&& thread : rpcThreads)
        {
            thread.join();
        }
    }

    void SetupSystem(uint32_t domainId, bool sync, std::vector<ClientParticipant>& clients,
                     std::vector<ServerParticipant>& servers, Middleware middleware)
    {
        ibConfig = BuildConfig(clients, servers, middleware, sync);
        if (middleware == Middleware::VAsio)
        {
            RunVasioRegistry(domainId);
        }
        if (sync)
        {
            RunSystemMaster(domainId);
        }
    }

    void WaitForAllStarted(std::vector<ClientParticipant>& clients,
                           std::vector<ServerParticipant>& servers, bool sync)
    {
        if (sync)
        {
            for (auto& s : servers)
            {
                auto futureStatus = s.startedPromise.get_future().wait_for(communicationTimeout);
                EXPECT_EQ(futureStatus, std::future_status::ready)
                    << "Test Failure: Awaiting startup timed out on subscriber";
            }
            for (auto& p : clients)
            {
                auto futureStatus = p.startedPromise.get_future().wait_for(communicationTimeout);
                EXPECT_EQ(futureStatus, std::future_status::ready)
                    << "Test Failure: Awaiting startup timed out on publisher";
            }
        }
    }

    void StopSimOnallCalledAndReceived(std::vector<ClientParticipant>& clients, std::vector<ServerParticipant>& servers, bool sync)
    {
        if (sync)
        {
            for (auto& p : clients)
            {
                auto futureStatus = p.allCalledPromise.get_future().wait_for(communicationTimeout);
                EXPECT_EQ(futureStatus, std::future_status::ready)
                    << "Test Failure: Awaiting call detached timed out on client";
            }
            for (auto& s : servers)
            {
                auto futureStatus = s.allReceivedPromise.get_future().wait_for(communicationTimeout);
                EXPECT_EQ(futureStatus, std::future_status::ready)
                    << "Test Failure: Awaiting call reception timed out on server";
            }
            for (auto& p : clients)
            {
                auto futureStatus = p.allCallsReturnedPromise.get_future().wait_for(communicationTimeout);
                EXPECT_EQ(futureStatus, std::future_status::ready)
                    << "Test Failure: Awaiting call return timed out on client";
            }
            systemMaster.systemController->Stop();
        }
    }

    void ShutdownSystem()
    {
        rpcThreads.clear();
        systemMaster.comAdapter.reset();
        registry.reset();
    }

protected:
    ib::cfg::Config                              ibConfig;
    std::unique_ptr<ib::extensions::IIbRegistry> registry;
    SystemMaster                                 systemMaster;
    std::vector<std::thread>                     rpcThreads;

    const uint8_t rpcFuncIncrement = 100;
    std::chrono::milliseconds communicationTimeout{10000ms};
    std::chrono::milliseconds asyncDelayBetweenCalls{500ms};
};

#if defined(IB_MW_HAVE_VASIO)

//--------------------------------------
// Sync tests: Publish in SimulationTask
//--------------------------------------

// One client participant, one server participant
TEST_F(RpcITest, test_1client_1server_sync_vasio)
{
    const auto middleware = Middleware::VAsio;
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numCalls = 3;
    const uint32_t numCallsToReceive = numCalls;
    const uint32_t numCallsToReturn = numCalls;
    const size_t messageSize = 3;
    const bool sync = true;

    std::vector<ClientParticipant> clients;
    std::vector<ServerParticipant> servers;
    clients.push_back({"Client1", {{"TestFuncA", "A", messageSize, numCalls, numCallsToReturn}}});
    servers.push_back({"Server1", {{"TestFuncA", "A", messageSize, numCallsToReceive}}});

    SetupSystem(domainId, sync, clients, servers, middleware);

    RunServers(servers, domainId, sync);
    RunClients(clients, domainId, sync);

    WaitForAllStarted(clients, servers, sync);

    StopSimOnallCalledAndReceived(clients, servers, sync);

    JoinRpcThreads();

    ShutdownSystem();
}

// Large messages
TEST_F(RpcITest, test_1client_1server_largemsg_sync_vasio)
{
    const auto     middleware = Middleware::VAsio;
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numCalls = 3;
    const uint32_t numCallsToReceive = numCalls;
    const uint32_t numCallsToReturn = numCalls;
    const size_t   messageSize = 250000;
    const bool     sync = true;

    std::vector<ClientParticipant> clients;
    std::vector<ServerParticipant> servers;
    clients.push_back({"Client1", {{"TestFuncA", "A", messageSize, numCalls, numCallsToReturn}}});
    servers.push_back({"Server1", {{"TestFuncA", "A", messageSize, numCallsToReceive}}});

    SetupSystem(domainId, sync, clients, servers, middleware);

    RunServers(servers, domainId, sync);
    RunClients(clients, domainId, sync);

    WaitForAllStarted(clients, servers, sync);

    StopSimOnallCalledAndReceived(clients, servers, sync);

    JoinRpcThreads();

    ShutdownSystem();
}

// 100 functions and one client/server participant
TEST_F(RpcITest, test_1client_1server_100functions_sync_vasio)
{
    const auto     middleware = Middleware::VAsio;
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numCalls = 3;
    const uint32_t numCallsToReceive = numCalls;
    const uint32_t numCallsToReturn = numCalls;
    const size_t   messageSize = 3;
    const bool     sync = true;
    const int      numFunctions = 100;

    std::vector<ClientParticipant> clients;
    std::vector<ServerParticipant> servers;
    clients.push_back({"Client1"});
    servers.push_back({"Server1"});
    for (int i = 0; i < numFunctions; i++)
    {
        std::string        functionName = std::to_string(i);
        RpcClientInfo  cInfo{ functionName, "A", messageSize, numCalls, numCallsToReturn };
        RpcServerInfo  sInfo{ functionName, "A", messageSize, numCalls };
        clients[0].rpcClients.push_back(std::move(cInfo));
        servers[0].rpcServers.push_back(std::move(sInfo));
    }

    SetupSystem(domainId, sync, clients, servers, middleware);

    RunServers(servers, domainId, sync);
    RunClients(clients, domainId, sync);

    WaitForAllStarted(clients, servers, sync);

    StopSimOnallCalledAndReceived(clients, servers, sync);

    JoinRpcThreads();

    ShutdownSystem();
}

// One client participant, two server participants
TEST_F(RpcITest, test_1client_2server_sync_vasio)
{
    const auto     middleware = Middleware::VAsio;
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numCalls = 3;
    const uint32_t numCallsToReceive = numCalls;
    const uint32_t numCallsToReturn = numCalls*2;
    const size_t   messageSize = 3;
    const bool     sync = true;

    std::vector<std::vector<uint8_t>> expectedReturnDataUnordered;
    for (uint32_t d = 0; d < numCalls; d++)
    {
        expectedReturnDataUnordered.emplace_back(std::vector<uint8_t>(messageSize, d + rpcFuncIncrement));
        expectedReturnDataUnordered.emplace_back(std::vector<uint8_t>(messageSize, d + rpcFuncIncrement));
    }

    std::vector<ClientParticipant> clients;
    std::vector<ServerParticipant> servers;
    clients.push_back({"Client1", {{"TestFuncA", "A", messageSize, numCalls, numCallsToReturn, expectedReturnDataUnordered}}});
    servers.push_back({"Server1", {{"TestFuncA", "A", messageSize, numCallsToReceive}}});
    servers.push_back({"Server2", {{"TestFuncA", "A", messageSize, numCallsToReceive}}});

    SetupSystem(domainId, sync, clients, servers, middleware);

    RunServers(servers, domainId, sync);
    RunClients(clients, domainId, sync);

    WaitForAllStarted(clients, servers, sync);

    StopSimOnallCalledAndReceived(clients, servers, sync);

    JoinRpcThreads();

    ShutdownSystem();
}

// Two client participants, one server participant
TEST_F(RpcITest, test_2client_1server_sync_vasio)
{
    const auto     middleware = Middleware::VAsio;
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numCalls = 3;
    const uint32_t numCallsToReceive = numCalls*2;
    const uint32_t numCallsToReturn = numCalls;
    const size_t   messageSize = 3;
    const bool     sync = true;

    std::vector<ClientParticipant> clients;
    std::vector<ServerParticipant> servers;
    clients.push_back({"Client1", {{"TestFuncA", "A", messageSize, numCalls, numCallsToReturn}}});
    clients.push_back({"Client2", {{"TestFuncA", "A", messageSize, numCalls, numCallsToReturn}}});

    std::vector<std::vector<uint8_t>> expectedDataUnordered;
    for (uint32_t d = 0; d < numCalls; d++)
    {
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(messageSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(messageSize, d));
    }
    servers.push_back({"Server1", {{"TestFuncA", "A", messageSize, numCallsToReceive, expectedDataUnordered}}});

    SetupSystem(domainId, sync, clients, servers, middleware);

    RunServers(servers, domainId, sync);
    RunClients(clients, domainId, sync);

    WaitForAllStarted(clients, servers, sync);

    StopSimOnallCalledAndReceived(clients, servers, sync);

    JoinRpcThreads();

    ShutdownSystem();
}

// Wrong functionName -> Expect no reception
TEST_F(RpcITest, test_1client_1server_wrongFunctionName_sync_vasio)
{
    const auto     middleware = Middleware::VAsio;
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numCalls = 3;
    const uint32_t numCallsToReceive = 0;
    const uint32_t numCallsToReturn = numCalls; // NB: We still expect the calls to return, but not with Callstatus::Success and without data
    const size_t   messageSize = 3;
    const bool     sync = true;

    std::vector<ClientParticipant> clients;
    std::vector<ServerParticipant> servers;
    clients.push_back({"Client1", {{"TestFuncA", "A", messageSize, numCalls, numCallsToReturn}}});
    servers.push_back({"Server1", {{"TestFuncB", "A", messageSize, numCallsToReceive}}});

    SetupSystem(domainId, sync, clients, servers, middleware);

    RunServers(servers, domainId, sync);
    RunClients(clients, domainId, sync);

    WaitForAllStarted(clients, servers, sync);

    StopSimOnallCalledAndReceived(clients, servers, sync);

    JoinRpcThreads();

    ShutdownSystem();
}

// Wrong dataExchangeFormat -> Expect no reception
TEST_F(RpcITest, test_1client_1server_wrongDataExchangeFormat_sync_vasio)
{
    const auto     middleware = Middleware::VAsio;
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numCalls = 3;
    const uint32_t numCallsToReceive = 0;
    const uint32_t numCallsToReturn = numCalls; // NB: We still expect the calls to return, but not with Callstatus::Success and without data
    const size_t   messageSize = 3;
    const bool     sync = true;

    std::vector<ClientParticipant> clients;
    std::vector<ServerParticipant> servers;
    clients.push_back({"Client1", {{"TestFuncA", "A", messageSize, numCalls, numCallsToReturn}}});
    servers.push_back({"Server1", {{"TestFuncA", "B", messageSize, numCallsToReceive}}});

    SetupSystem(domainId, sync, clients, servers, middleware);

    RunServers(servers, domainId, sync);
    RunClients(clients, domainId, sync);

    WaitForAllStarted(clients, servers, sync);

    StopSimOnallCalledAndReceived(clients, servers, sync);

    JoinRpcThreads();

    ShutdownSystem();
}

// Wildcard dataExchangeFormat on server
TEST_F(RpcITest, test_1client_1server_wildcardDxf_sync_vasio)
{
    const auto     middleware = Middleware::VAsio;
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numCalls = 3;
    const uint32_t numCallsToReceive = numCalls;
    const uint32_t numCallsToReturn = numCalls;
    const size_t   messageSize = 3;
    const bool     sync = true;

    std::vector<ClientParticipant> clients;
    std::vector<ServerParticipant> servers;
    clients.push_back({"Client1", {{"TestFuncA", "A", messageSize, numCalls, numCallsToReturn}}});
    servers.push_back({"Server1", {{"TestFuncA", "*", messageSize, numCallsToReceive}}});

    SetupSystem(domainId, sync, clients, servers, middleware);

    RunServers(servers, domainId, sync);
    RunClients(clients, domainId, sync);

    WaitForAllStarted(clients, servers, sync);

    StopSimOnallCalledAndReceived(clients, servers, sync);

    JoinRpcThreads();

    ShutdownSystem();
}

//-----------------------------------------------------
// Async tests: No participantController/SimulationTask
//-----------------------------------------------------

// Async: Start servers first, call with delay to ensure reception
TEST_F(RpcITest, test_1client_1server_async_vasio)
{
    const auto     middleware = Middleware::VAsio;
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numCalls = 3;
    const uint32_t numCallsToReceive = numCalls;
    const uint32_t numCallsToReturn = numCalls;
    const size_t   messageSize = 3;
    const bool sync = false;

    std::vector<ClientParticipant> clients;
    std::vector<ServerParticipant> servers;
    clients.push_back ({"Client1", { { "TestFuncA", "A", messageSize, numCalls, numCallsToReturn } } });
    servers.push_back({ "Server1", { { "TestFuncA", "A", messageSize, numCallsToReceive } } });

    SetupSystem(domainId, sync, clients, servers, middleware);

    RunServers(servers, domainId, sync);
    for (auto& s : servers)
    {
        auto futureStatus = s.startedPromise.get_future().wait_for(communicationTimeout);
        EXPECT_EQ(futureStatus, std::future_status::ready)
            << "Test Failure: Awaiting reception timed out on subscriber";
    }
    RunClients(clients, domainId, sync);

    JoinRpcThreads();

    ShutdownSystem();
}

#endif

} // anonymous namespace
