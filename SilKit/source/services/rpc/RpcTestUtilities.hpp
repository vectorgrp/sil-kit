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

#include <vector>
#include <typeinfo>


#include "ITimeProvider.hpp"
#include "CreateParticipantT.hpp"
#include "Participant.hpp"
#include "Participant_impl.hpp"
#include "ProtocolVersion.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ILoggerInternal.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {
namespace Tests {

struct MockConnection
{
    MockConnection(SilKit::Core::IParticipantInternal*, VSilKit::IMetricsManager*,
                   SilKit::Config::ParticipantConfiguration /*config*/, std::string /*participantName*/,
                   SilKit::Core::ParticipantId /*participantId*/,
                   SilKit::Services::Orchestration::ITimeProvider* /*timeProvider*/, SilKit::Core::ProtocolVersion)
    {
    }

    void SetLogger(SilKit::Services::Logging::ILogger* /*logger*/) {}
    void SetTimeSyncService(SilKit::Services::Orchestration::TimeSyncService* /*timeSyncService*/) {}
    void JoinSimulation(std::string /*registryUri*/) {}
    template <class SilKitServiceT>
    void RegisterSilKitService(SilKitServiceT* /*service*/)
    {
    }

    void RegisterSilKitService(RpcClient* receiver)
    {
        services.rpcClient.push_back(receiver);
    }

    void RegisterSilKitService(RpcServer* receiver)
    {
        services.rpcServer.push_back(receiver);
    }

    void RegisterSilKitService(RpcServerInternal* receiver)
    {
        services.rpcServerInternal.push_back(receiver);
    }

    template <class SilKitServiceT>
    void SetHistoryLengthForLink(size_t /*history*/, SilKitServiceT* /*service*/)
    {
    }

    template <typename SilKitMessageT>
    void SendMsg(const SilKit::Core::IServiceEndpoint* /*from*/, SilKitMessageT&& /*msg*/)
    {
    }

    void SendMsg(const SilKit::Core::IServiceEndpoint* from, FunctionCall msg)
    {
        for (auto& rpcServerInternal : services.rpcServerInternal)
        {
            rpcServerInternal->ReceiveMsg(from, msg);
        }
        Mock_SendMsg(from, std::move(msg));
    }

    void SendMsg(const SilKit::Core::IServiceEndpoint* from, FunctionCallResponse msg)
    {
        for (auto& rpcClient : services.rpcClient)
        {
            rpcClient->ReceiveMsg(from, msg);
        }
        Mock_SendMsg(from, std::move(msg));
    }

    MOCK_METHOD(void, Mock_SendMsg, (const SilKit::Core::IServiceEndpoint* /*from*/, FunctionCall /*msg*/));
    MOCK_METHOD(void, Mock_SendMsg, (const SilKit::Core::IServiceEndpoint* /*from*/, FunctionCallResponse /*msg*/));

    template <typename SilKitMessageT>
    void SendMsg(const SilKit::Core::IServiceEndpoint* /*from*/, const std::string& /*target*/,
                 SilKitMessageT&& /*msg*/)
    {
    }

    void OnAllMessagesDelivered(std::function<void()> /*callback*/) {}
    void FlushSendBuffers() {}
    void ExecuteDeferred(std::function<void()> /*callback*/) {}
    void NotifyShutdown() {}
    void EnableAggregation() {}

    void RegisterMessageReceiver(
        std::function<void(SilKit::Core::IVAsioPeer* /*peer*/, SilKit::Core::ParticipantAnnouncement)> /*callback*/)
    {
    }

    void RegisterPeerShutdownCallback(std::function<void(SilKit::Core::IVAsioPeer* peer)> /*callback*/) {}

    void AddAsyncSubscriptionsCompletionHandler(std::function<void()> /*completionHandler*/){};

    void Test_SetTimeProvider(SilKit::Services::Orchestration::ITimeProvider* timeProvider)
    {
        for (auto& service : services.rpcClient)
        {
            service->SetTimeProvider(timeProvider);
        }

        for (auto& service : services.rpcServer)
        {
            service->SetTimeProvider(timeProvider);
        }

        for (auto& service : services.rpcServerInternal)
        {
            service->SetTimeProvider(timeProvider);
        }
    }

    size_t GetNumberOfConnectedParticipants()
    {
        return 0;
    }

    size_t GetNumberOfRemoteReceivers(const SilKit::Core::IServiceEndpoint* /*service*/,
                                      const std::string& /*msgTypeName*/)
    {
        return 0;
    };

    std::vector<std::string> GetParticipantNamesOfRemoteReceivers(const SilKit::Core::IServiceEndpoint* /*service*/,
                                                                  const std::string& /*msgTypeName*/)
    {
        return {};
    };

    bool ParticipantHasCapability(const std::string& /*participantName*/, const std::string& /*capability*/) const
    {
        return true;
    }

    struct
    {
        std::vector<RpcClient*> rpcClient;
        std::vector<RpcServer*> rpcServer;
        std::vector<RpcServerInternal*> rpcServerInternal;
    } services;
};

using MockConnectionParticipant = SilKit::Core::Participant<MockConnection>;

inline auto MakeMockConnectionParticipant(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                                          const std::string& participantName)
    -> std::unique_ptr<MockConnectionParticipant>
{
    return SilKit::Core::CreateParticipantT<MockConnection>(std::move(participantConfig), participantName,
                                                            "silkit://mock.connection.silkit:0");
}

struct Callbacks
{
    MOCK_METHOD(void, CallHandler, (IRpcServer* /*rpcServer*/, RpcCallEvent /*event*/));
    MOCK_METHOD(void, CallResultHandler, (IRpcClient* /*rpcClient*/, RpcCallResultEvent /*event*/));
};

class RpcTestBase : public testing::Test
{
public:
    RpcTestBase()
        : participant(MakeMockConnectionParticipant(MakeParticipantConfiguration(), "RpcClientTest"))
    {
    }

    auto CreateRpcClient() -> IRpcClient*
    {
        if (_rpcClient == nullptr)
        {
            SilKit::Services::Rpc::RpcSpec dataSpec{"FunctionA", "application/octet-stream"};
            _rpcClient = participant->CreateRpcClient("RpcClient", dataSpec, nullptr);
        }
        return _rpcClient;
    }

    auto CreateRpcServer() -> IRpcServer*
    {
        if (_rpcServer == nullptr)
        {
            SilKit::Services::Rpc::RpcSpec dataSpec{"FunctionA", "application/octet-stream"};
            _rpcServer = participant->CreateRpcServer("RpcServer", dataSpec, nullptr);
        }
        return _rpcServer;
    }

private:
    static auto MakeParticipantConfiguration() -> std::shared_ptr<SilKit::Config::ParticipantConfiguration>
    {
        auto configuration =
            std::make_shared<SilKit::Config::ParticipantConfiguration>(SilKit::Config::ParticipantConfiguration());

        SilKit::Config::RpcClient rpcClientConfig;
        rpcClientConfig.name = "RpcClient";
        rpcClientConfig.functionName = "FunctionA";
        configuration->rpcClients.push_back(rpcClientConfig);

        SilKit::Config::RpcServer rpcServerConfig;
        rpcServerConfig.name = "RpcServer";
        rpcServerConfig.functionName = "FunctionA";
        configuration->rpcServers.push_back(rpcServerConfig);

        return configuration;
    }

public:
    std::unique_ptr<MockConnectionParticipant> participant;

    Callbacks callbacks;

public:
    const std::vector<std::uint8_t> sampleData = {0x01, 0x02, 0x03, 0x04};

private:
    IRpcClient* _rpcClient = nullptr;
    IRpcServer* _rpcServer = nullptr;
};

} // namespace Tests
} // namespace Rpc
} // namespace Services
} // namespace SilKit
