// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>
#include <iostream>
#include <typeinfo>

#include "ITimeProvider.hpp"
#include "CreateParticipant.hpp"
#include "Participant.hpp"
#include "Participant_impl.hpp"
#include "ProtocolVersion.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"


namespace SilKit {
namespace Services {
namespace Rpc {
namespace Tests {

struct MockConnection
{
    MockConnection(SilKit::Config::ParticipantConfiguration /*config*/, std::string /*participantName*/,
                   SilKit::ParticipantId /*participantId*/, SilKit::Core::ProtocolVersion)
    {
    }

    void SetLogger(SilKit::Services::Logging::ILogger* /*logger*/) {}
    void SetTimeSyncService(SilKit::Services::Orchestration::TimeSyncService* /*timeSyncService*/) {}
    void JoinDomain(uint32_t /*domainId*/) {}
    void JoinDomain(std::string /*domainId*/) {}

    template <class SilKitServiceT>
    void RegisterSilKitService(const std::string& /*topicName*/, SilKit::Core::EndpointId /*endpointId*/,
                           SilKitServiceT* /*receiver*/)
    {
    }

    void RegisterSilKitService(const std::string& /*topicName*/, SilKit::Core::EndpointId /*endpointId*/, RpcClient* receiver)
    {
        services.rpcClient.push_back(receiver);
    }

    void RegisterSilKitService(const std::string& /*topicName*/, SilKit::Core::EndpointId /*endpointId*/, RpcServer* receiver)
    {
        services.rpcServer.push_back(receiver);
    }

    void RegisterSilKitService(const std::string& /*topicName*/, SilKit::Core::EndpointId /*endpointId*/,
                           RpcServerInternal* receiver)
    {
        services.rpcServerInternal.push_back(receiver);
    }

    template <class SilKitServiceT>
    void SetHistoryLengthForLink(const std::string& /*linkName*/, size_t /*history*/, SilKitServiceT* /*service*/)
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
            rpcServerInternal->ReceiveSilKitMessage(from, msg);
        }
        Mock_SendMsg(from, std::move(msg));
    }

    void SendMsg(const SilKit::Core::IServiceEndpoint* from, FunctionCallResponse msg)
    {
        for (auto& rpcClient : services.rpcClient)
        {
            rpcClient->ReceiveSilKitMessage(from, msg);
        }
        Mock_SendMsg(from, std::move(msg));
    }

    MOCK_METHOD(void, Mock_SendMsg, (const SilKit::Core::IServiceEndpoint* /*from*/, FunctionCall /*msg*/));
    MOCK_METHOD(void, Mock_SendMsg, (const SilKit::Core::IServiceEndpoint* /*from*/, FunctionCallResponse /*msg*/));

    template <typename SilKitMessageT>
    void SendMsg(const SilKit::Core::IServiceEndpoint* /*from*/, const std::string& /*target*/, SilKitMessageT&& /*msg*/)
    {
    }

    void OnAllMessagesDelivered(std::function<void()> /*callback*/) {}
    void FlushSendBuffers() {}
    void ExecuteDeferred(std::function<void()> /*callback*/) {}
    void NotifyShutdown() {}

    void RegisterMessageReceiver(
        std::function<void(SilKit::Core::IVAsioPeer* /*peer*/, SilKit::Core::ParticipantAnnouncement)> /*callback*/)
    {
    }

    void RegisterPeerShutdownCallback(std::function<void(SilKit::Core::IVAsioPeer* peer)> /*callback*/) {}

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
    auto&& cfg = SilKit::Core::ValidateAndSanitizeConfig(participantConfig, participantName);
    return std::make_unique<MockConnectionParticipant>(std::move(cfg), participantName);
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
            _rpcClient = participant->CreateRpcClient("RpcClient");
        }
        return _rpcClient;
    }

    auto CreateRpcServer() -> IRpcServer*
    {
        if (_rpcServer == nullptr)
        {
            _rpcServer = participant->CreateRpcServer("RpcServer");
        }
        return _rpcServer;
    }

private:
    static auto MakeParticipantConfiguration() -> std::shared_ptr<SilKit::Config::ParticipantConfiguration>
    {
        auto configuration = std::make_shared<SilKit::Config::ParticipantConfiguration>(SilKit::Config::ParticipantConfiguration());

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
