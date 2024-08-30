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

#pragma once

#include <thread>

#include "silkit/SilKit.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/all.hpp"

#include "ConfigurationTestUtils.hpp"

#include "IntegrationTestInfrastructure.hpp"
#include "IParticipantInternal.hpp"
#include "IServiceDiscovery.hpp"

using namespace std::chrono_literals;
using namespace SilKit;
using namespace SilKit::Services::Orchestration;
using namespace SilKit::Config;
using namespace SilKit::Services::Rpc;

const uint8_t rpcFuncIncrement = 100u;
const size_t defaultMsgSize = 3;
const uint32_t defaultNumCalls = 3;

class ITest_Internals_Rpc : public testing::Test
{
protected:
    ITest_Internals_Rpc() {}

    struct RpcClientInfo
    {
        RpcClientInfo(const std::string& newControllerName, const std::string& newFunctionName,
                      const std::string& newMediaType, const std::vector<SilKit::Services::MatchingLabel>& newLabels,
                      size_t newMessageSizeInBytes, uint32_t newNumCalls, uint32_t newNumCallsToReturn)
        {
            expectIncreasingData = true;
            controllerName = newControllerName;
            functionName = newFunctionName;
            mediaType = newMediaType;
            labels = newLabels;
            messageSizeInBytes = newMessageSizeInBytes;
            numCalls = newNumCalls;
            numCallsToReturn = newNumCallsToReturn;
        }

        RpcClientInfo(const std::string& newControllerName, const std::string& newFunctionName,
                      const std::string& newMediaType, const std::vector<SilKit::Services::MatchingLabel>& newLabels,
                      size_t newMessageSizeInBytes, uint32_t newNumCalls, uint32_t newNumCallsToReturn,
                      const std::vector<std::vector<uint8_t>>& newExpectedReturnDataUnordered)
        {
            expectIncreasingData = false;
            controllerName = newControllerName;
            functionName = newFunctionName;
            mediaType = newMediaType;
            labels = newLabels;
            messageSizeInBytes = newMessageSizeInBytes;
            numCalls = newNumCalls;
            numCallsToReturn = newNumCallsToReturn;
            expectedReturnDataUnordered = newExpectedReturnDataUnordered;
        }

        bool expectIncreasingData;
        std::string controllerName;
        std::string functionName;
        std::string mediaType;
        std::vector<SilKit::Services::MatchingLabel> labels;
        size_t messageSizeInBytes;
        uint32_t numCalls;
        uint32_t numCallsToReturn;
        std::vector<std::vector<uint8_t>> expectedReturnDataUnordered;
        uint32_t numCallsToTimeout{0};
        std::chrono::nanoseconds timeout{};
    };

    class RpcClientState
    {
    public:
        RpcClientState(RpcClientInfo info)
            : info{std::move(info)}
        {
        }

        RpcClientInfo info;

        std::atomic<uint32_t> callCounter{0};
        std::atomic<uint32_t> callReturnedSuccessCounter{0};
        std::atomic<bool> allCalled{false};
        std::atomic<bool> allCallsReturned{false};
        std::atomic<uint32_t> callReturnedTimeoutCounter{0};

        IRpcClient* rpcClient = nullptr;

        void Call()
        {
            if (!allCalled)
            {
                auto argumentData = std::vector<uint8_t>(info.messageSizeInBytes, static_cast<uint8_t>(callCounter));
                if (info.numCallsToTimeout > 0)
                {
                    rpcClient->CallWithTimeout(argumentData, info.timeout,
                                               reinterpret_cast<void*>(uintptr_t(callCounter)));
                }
                else
                {
                    rpcClient->Call(argumentData, reinterpret_cast<void*>(uintptr_t(callCounter)));
                }

                callCounter++;
                if (callCounter >= info.numCalls)
                {
                    allCalled = true;
                }
            }
        }

        void OnCallReturned(const std::vector<uint8_t>& returnData)
        {
            if (info.expectIncreasingData)
            {
                auto expectedData = std::vector<uint8_t>(
                    info.messageSizeInBytes, static_cast<uint8_t>(callReturnedSuccessCounter + rpcFuncIncrement));
                EXPECT_EQ(returnData, expectedData);
            }
            else
            {
                auto foundDataIter = std::find(info.expectedReturnDataUnordered.begin(),
                                               info.expectedReturnDataUnordered.end(), returnData);
                EXPECT_EQ(foundDataIter != info.expectedReturnDataUnordered.end(), true);
                if (foundDataIter != info.expectedReturnDataUnordered.end())
                {
                    info.expectedReturnDataUnordered.erase(foundDataIter);
                }
            }

            callReturnedSuccessCounter++;
            if (callReturnedSuccessCounter + callReturnedTimeoutCounter
                >= info.numCallsToReturn + info.numCallsToTimeout)
            {
                allCallsReturned = true;
            }
        }

        void OnCallTimeout()
        {
            callReturnedTimeoutCounter++;
            if (callReturnedSuccessCounter + callReturnedTimeoutCounter
                >= info.numCallsToReturn + info.numCallsToTimeout)
            {
                allCallsReturned = true;
            }
        }
    };

    struct RpcServerInfo
    {
        RpcServerInfo(const std::string& newControllerName, const std::string& newFunctionName,
                      const std::string& newMediaType, const std::vector<SilKit::Services::MatchingLabel>& newLabels,
                      size_t newMessageSizeInBytes, uint32_t newNumCallsToReceive)
        {
            expectIncreasingData = true;
            controllerName = newControllerName;
            functionName = newFunctionName;
            mediaType = newMediaType;
            labels = newLabels;
            messageSizeInBytes = newMessageSizeInBytes;
            numCallsToReceive = newNumCallsToReceive;
        }
        RpcServerInfo(const std::string& newControllerName, const std::string& newFunctionName,
                      const std::string& newMediaType, const std::vector<SilKit::Services::MatchingLabel>& newLabels,
                      size_t newMessageSizeInBytes, uint32_t newNumCallsToReceive,
                      const std::vector<std::vector<uint8_t>>& newExpectedDataUnordered)
        {
            expectIncreasingData = false;
            controllerName = newControllerName;
            functionName = newFunctionName;
            mediaType = newMediaType;
            labels = newLabels;
            messageSizeInBytes = newMessageSizeInBytes;
            numCallsToReceive = newNumCallsToReceive;
            expectedDataUnordered = newExpectedDataUnordered;
        }

        std::string controllerName;
        std::string functionName;
        std::string mediaType;
        std::vector<SilKit::Services::MatchingLabel> labels;
        size_t messageSizeInBytes;
        uint32_t numCallsToReceive;
        bool expectIncreasingData;
        std::vector<std::vector<uint8_t>> expectedDataUnordered;
        IRpcServer* rpcServer = nullptr;
        bool doNotReply{false};
    };

    struct RpcServerState
    {
        RpcServerState(RpcServerInfo info)
            : info{std::move(info)}
        {
        }

        RpcServerInfo info;

        std::atomic<uint32_t> receiveCallCounter{0};
        std::atomic<bool> allReceived{false};

        IRpcServer* rpcServer = nullptr;

        void ReceiveCall(const std::vector<uint8_t>& argumentData)
        {
            if (!allReceived)
            {
                if (info.expectIncreasingData)
                {
                    auto expectedData =
                        std::vector<uint8_t>(info.messageSizeInBytes, static_cast<uint8_t>(receiveCallCounter));
                    EXPECT_EQ(argumentData, expectedData);
                }
                else
                {
                    auto foundDataIter =
                        std::find(info.expectedDataUnordered.begin(), info.expectedDataUnordered.end(), argumentData);
                    EXPECT_EQ(foundDataIter != info.expectedDataUnordered.end(), true);
                    if (foundDataIter != info.expectedDataUnordered.end())
                    {
                        info.expectedDataUnordered.erase(foundDataIter);
                    }
                }

                receiveCallCounter++;
                if (receiveCallCounter >= info.numCallsToReceive)
                {
                    allReceived = true;
                }
            }
        }
    };

    struct RpcParticipant
    {
        RpcParticipant(const std::string& newName, const std::vector<std::string>& newExpectedFunctionNames)
        {
            name = newName;
            expectedFunctionNames = newExpectedFunctionNames;
        }

        RpcParticipant(const std::string& newName, std::vector<RpcServerInfo> newRpcServers,
                       std::vector<RpcClientInfo> newRpcClients, std::vector<std::string> newExpectedFunctionNames)
            : expectedFunctionNames{std::move(newExpectedFunctionNames)}
        {
            name = newName;

            std::for_each(newRpcServers.begin(), newRpcServers.end(), [this](const auto& info) { AddRpcServer(info); });

            std::for_each(newRpcClients.begin(), newRpcClients.end(), [this](const auto& info) { AddRpcClient(info); });
        }

        std::string name;
        std::vector<std::unique_ptr<RpcClientState>> rpcClients;
        std::vector<std::unique_ptr<RpcServerState>> rpcServers;
        std::unique_ptr<IParticipant> participant;
        SilKit::Core::IParticipantInternal* participantImpl = nullptr;

        std::vector<std::string> expectedFunctionNames;
        bool allCalled{false};
        std::promise<void> allCalledPromise;
        std::promise<void> allCallsReturnedPromise;
        bool allCallsReturned{false};
        std::promise<void> allDiscoveredPromise;
        bool allDiscovered{false};

        std::promise<void> allReceivedPromise;
        bool allReceived{false};

        std::chrono::milliseconds communicationTimeout{20000ms};

        void AddRpcClient(const RpcClientInfo& info)
        {
            rpcClients.emplace_back(std::make_unique<RpcClientState>(info));
        }

        void AddRpcServer(const RpcServerInfo& info)
        {
            rpcServers.emplace_back(std::make_unique<RpcServerState>(info));
        }

        void PrepareAllReceivedPromise()
        {
            if (std::all_of(rpcServers.begin(), rpcServers.end(),
                            [](const auto& s) -> bool { return s->info.numCallsToReceive == 0; }))
            {
                allReceived = true;
                allReceivedPromise.set_value();
            }
        }

        void CheckAllCalledPromise()
        {
            if (!allCalled && std::all_of(rpcClients.begin(), rpcClients.end(), [](const auto& c) -> bool {
                return c->allCalled;
            }))
            {
                allCalled = true;
                allCalledPromise.set_value();
            }
        }

        void CheckAllCallsReceivedPromise()
        {
            if (!allReceived && std::all_of(rpcServers.begin(), rpcServers.end(), [](const auto& s) -> bool {
                return s->allReceived;
            }))
            {
                allReceived = true;
                allReceivedPromise.set_value();
            }
        }

        void CheckAllCallsReturnedPromise()
        {
            if (!allCallsReturned
                && std::all_of(rpcClients.begin(), rpcClients.end(),
                               [](const auto& clientInfo) -> bool { return clientInfo->allCallsReturned; }))
            {
                allCallsReturned = true;
                allCallsReturnedPromise.set_value();
            }
        }

        void WaitForAllCalled()
        {
            auto futureStatus = allCalledPromise.get_future().wait_for(communicationTimeout);
            EXPECT_EQ(futureStatus, std::future_status::ready) << "Test Failure: Awaiting clients to call timed out";
        }
        void WaitForAllCallsReturned()
        {
            auto futureStatus = allCallsReturnedPromise.get_future().wait_for(communicationTimeout);
            EXPECT_EQ(futureStatus, std::future_status::ready) << "Test Failure: Awaiting call return timed out";
        }
        void WaitForAllCallsReceived()
        {
            auto futureStatus = allReceivedPromise.get_future().wait_for(communicationTimeout);
            EXPECT_EQ(futureStatus, std::future_status::ready) << "Test Failure: Awaiting reception timed out";
        }
        void WaitForAllDiscovered()
        {
            auto futureStatus = allDiscoveredPromise.get_future().wait_for(communicationTimeout);
            EXPECT_EQ(futureStatus, std::future_status::ready) << "Test Failure: Awaiting server discovery timed out";
        }

        void OnNewServiceDiscovery(const SilKit::Core::ServiceDescriptor sd)
        {
            if (!allDiscovered)
            {
                std::string functionName =
                    sd.GetSupplementalData()[SilKit::Core::Discovery::supplKeyRpcServerFunctionName];
                auto foundFunctionNameIter =
                    std::find(expectedFunctionNames.begin(), expectedFunctionNames.end(), functionName);
                if (foundFunctionNameIter != expectedFunctionNames.end())
                {
                    expectedFunctionNames.erase(foundFunctionNameIter);
                }

                if (expectedFunctionNames.empty())
                {
                    allDiscovered = true;
                    allDiscoveredPromise.set_value();
                }
            }
        }
    };

    void ParticipantThread(RpcParticipant& participant, const std::string& registryUri, bool sync)
    {
        try
        {
            participant.participant = SilKit::CreateParticipantImpl(
                SilKit::Config::MakeParticipantConfigurationWithLoggingImpl(SilKit::Services::Logging::Level::Warn),
                participant.name, registryUri);
            participant.participantImpl =
                dynamic_cast<SilKit::Core::IParticipantInternal*>(participant.participant.get());

            participant.participantImpl->GetServiceDiscovery()->RegisterServiceDiscoveryHandler(
                [&participant](auto type, auto&& serviceDescr) {
                if (type == SilKit::Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated)
                {
                    if (serviceDescr.GetNetworkType() == SilKit::Config::NetworkType::RPC)
                    {
                        participant.OnNewServiceDiscovery(serviceDescr);
                    }
                }
            });


            // Create Clients
            for (const auto& c : participant.rpcClients)
            {
                auto callReturnHandler = [&participant, &c](IRpcClient* /*client*/, RpcCallResultEvent event) {
                    if (!c->allCallsReturned)
                    {
                        if (event.callStatus == RpcCallStatus::Success)
                        {
                            c->OnCallReturned(SilKit::Util::ToStdVector(event.resultData));
                        }
                        if (event.callStatus == RpcCallStatus::Timeout)
                        {
                            c->OnCallTimeout();
                        }
                    }
                    participant.CheckAllCallsReturnedPromise();
                };

                SilKit::Services::Rpc::RpcSpec dataSpec{c->info.functionName, c->info.mediaType};
                for (const auto& label : c->info.labels)
                {
                    dataSpec.AddLabel(label);
                }

                c->rpcClient =
                    participant.participant->CreateRpcClient(c->info.controllerName, dataSpec, callReturnHandler);
            }

            // Create Servers
            for (const auto& s : participant.rpcServers)
            {
                participant.PrepareAllReceivedPromise();

                auto processCalls = [&participant, &s](IRpcServer* server, RpcCallEvent event) {
                    // Increment data
                    auto returnData = SilKit::Util::ToStdVector(event.argumentData);
                    for (auto& d : returnData)
                    {
                        d += rpcFuncIncrement;
                    }
                    if (!s->info.doNotReply)
                    {
                        server->SubmitResult(event.callHandle, returnData);
                    }

                    // Evaluate data and reception count
                    s->ReceiveCall(SilKit::Util::ToStdVector(event.argumentData));
                    participant.CheckAllCallsReceivedPromise();
                };

                SilKit::Services::Rpc::RpcSpec dataSpec{s->info.functionName, s->info.mediaType};
                for (const auto& label : s->info.labels)
                {
                    dataSpec.AddLabel(label);
                }

                s->rpcServer = participant.participant->CreateRpcServer(s->info.controllerName, dataSpec, processCalls);
            }

            if (sync)
            {
                auto* lifecycleService = participant.participant->CreateLifecycleService(
                    {SilKit::Services::Orchestration::OperationMode::Coordinated});
                auto* timeSyncService = lifecycleService->CreateTimeSyncService();

                timeSyncService->SetSimulationStepHandler(
                    [&participant](std::chrono::nanoseconds /*now*/, std::chrono::nanoseconds /*duration*/) {
                    for (const auto& client : participant.rpcClients)
                    {
                        client->Call();
                    }
                    participant.CheckAllCalledPromise();
                }, 1s);
                auto finalStateFuture = lifecycleService->StartLifecycle();
                finalStateFuture.get();
            }
            else
            {
                // Call by client
                if (!participant.rpcClients.empty())
                {
                    while (std::none_of(participant.rpcClients.begin(), participant.rpcClients.end(),
                                        [](const auto& c) -> bool { return c->allCalled; }))
                    {
                        std::this_thread::sleep_for(50ms);
                        for (auto& client : participant.rpcClients)
                        {
                            client->Call();
                        };
                    }
                    participant.allCalledPromise.set_value();
                }
                // Calls received by server
                if (!participant.rpcServers.empty())
                {
                    participant.WaitForAllCallsReceived();
                }
                // Calls returned on client
                if (!participant.rpcClients.empty())
                {
                    participant.WaitForAllCallsReturned();
                }
            }

            for (const auto& c : participant.rpcClients)
            {
                EXPECT_EQ(c->callCounter, c->info.numCalls);
                EXPECT_EQ(c->callReturnedSuccessCounter, c->info.numCallsToReturn);
                EXPECT_EQ(c->callReturnedTimeoutCounter, c->info.numCallsToTimeout);
            }
            for (const auto& s : participant.rpcServers)
            {
                EXPECT_EQ(s->receiveCallCounter, s->info.numCallsToReceive);
            }
        }
        catch (const std::exception& error)
        {
            _testSystem.ShutdownOnException(error);
        }
    }

    void RunParticipants(std::vector<RpcParticipant>& rpcs, const std::string& registryUri, bool sync)
    {
        try
        {
            for (auto& participant : rpcs)
            {
                _rpcThreads.emplace_back(
                    [this, &participant, registryUri, sync] { ParticipantThread(participant, registryUri, sync); });
            }
        }
        catch (const std::exception& error)
        {
            _testSystem.ShutdownOnException(error);
        }
    }

    void JoinRpcThreads()
    {
        for (auto&& thread : _rpcThreads)
        {
            thread.join();
        }
    }

    void WaitForAllServersDiscovered(std::vector<RpcParticipant>& rpcs)
    {
        for (auto& r : rpcs)
        {
            if (!r.rpcClients.empty())
            {
                r.WaitForAllDiscovered();
            }
        }
    }

    void StopSimOnallCalledAndReceived(std::vector<RpcParticipant>& rpcs, bool sync)
    {
        if (sync)
        {
            for (auto& r : rpcs)
            {
                if (!r.rpcClients.empty())
                {
                    r.WaitForAllCalled();
                }
            }
            for (auto& r : rpcs)
            {
                if (!r.rpcServers.empty())
                {
                    r.WaitForAllCallsReceived();
                }
            }
            for (auto& r : rpcs)
            {
                if (!r.rpcClients.empty())
                {
                    r.WaitForAllCallsReturned();
                }
            }
            _testSystem.SystemMasterStop();
        }
    }

    void ShutdownSystem()
    {
        _rpcThreads.clear();
        _testSystem.ShutdownInfrastructure();
    }

    void RunSyncTest(std::vector<RpcParticipant>& rpcs)
    {
        std::vector<std::string> requiredParticipantNames;
        for (const auto& p : rpcs)
        {
            requiredParticipantNames.push_back(p.name);
        }

        _testSystem.SetupRegistryAndSystemMaster("silkit://localhost:0", true, std::move(requiredParticipantNames));
        RunParticipants(rpcs, _testSystem.GetRegistryUri(), true);
        WaitForAllServersDiscovered(rpcs);
        StopSimOnallCalledAndReceived(rpcs, true);
        JoinRpcThreads();
        ShutdownSystem();
    }

    void RunAsyncTest(std::vector<RpcParticipant>& rpcs)
    {
        _testSystem.SetupRegistryAndSystemMaster("silkit://localhost:0", false, {});
        RunParticipants(rpcs, _testSystem.GetRegistryUri(), false);
        WaitForAllServersDiscovered(rpcs);
        JoinRpcThreads();
        ShutdownSystem();
    }

protected:
    std::vector<std::thread> _rpcThreads;

private:
    TestInfrastructure _testSystem;
};
