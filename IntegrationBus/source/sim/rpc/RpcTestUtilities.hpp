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


namespace ib {
namespace sim {
namespace rpc {
namespace test {

struct MockConnection
{
    MockConnection(ib::cfg::ParticipantConfiguration /*config*/, std::string /*participantName*/,
                   ib::mw::ParticipantId /*participantId*/, ib::mw::ProtocolVersion)
    {
    }

    void SetLogger(ib::mw::logging::ILogger* /*logger*/) {}
    void JoinDomain(uint32_t /*domainId*/) {}
    void JoinDomain(std::string /*domainId*/) {}

    template <class IbServiceT>
    void RegisterIbService(const std::string& /*topicName*/, ib::mw::EndpointId /*endpointId*/,
                           IbServiceT* /*receiver*/)
    {
    }

    void RegisterIbService(const std::string& /*topicName*/, ib::mw::EndpointId /*endpointId*/, RpcClient* receiver)
    {
        services.rpcClient.push_back(receiver);
    }

    void RegisterIbService(const std::string& /*topicName*/, ib::mw::EndpointId /*endpointId*/, RpcServer* receiver)
    {
        services.rpcServer.push_back(receiver);
    }

    void RegisterIbService(const std::string& /*topicName*/, ib::mw::EndpointId /*endpointId*/,
                           RpcServerInternal* receiver)
    {
        services.rpcServerInternal.push_back(receiver);
    }

    template <class IbServiceT>
    void SetHistoryLengthForLink(const std::string& /*linkName*/, size_t /*history*/, IbServiceT* /*service*/)
    {
    }

    template <typename IbMessageT>
    void SendIbMessage(const ib::mw::IIbServiceEndpoint* /*from*/, IbMessageT&& /*msg*/)
    {
    }

    void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, FunctionCall msg)
    {
        for (auto& rpcServerInternal : services.rpcServerInternal)
        {
            rpcServerInternal->ReceiveIbMessage(from, msg);
        }
        Mock_SendIbMessage(from, std::move(msg));
    }

    void SendIbMessage(const ib::mw::IIbServiceEndpoint* from, FunctionCallResponse msg)
    {
        for (auto& rpcClient : services.rpcClient)
        {
            rpcClient->ReceiveIbMessage(from, msg);
        }
        Mock_SendIbMessage(from, std::move(msg));
    }

    MOCK_METHOD(void, Mock_SendIbMessage, (const ib::mw::IIbServiceEndpoint* /*from*/, FunctionCall /*msg*/));
    MOCK_METHOD(void, Mock_SendIbMessage, (const ib::mw::IIbServiceEndpoint* /*from*/, FunctionCallResponse /*msg*/));

    template <typename IbMessageT>
    void SendIbMessage(const ib::mw::IIbServiceEndpoint* /*from*/, const std::string& /*target*/, IbMessageT&& /*msg*/)
    {
    }

    void OnAllMessagesDelivered(std::function<void()> /*callback*/) {}
    void FlushSendBuffers() {}
    void ExecuteDeferred(std::function<void()> /*callback*/) {}
    void NotifyShutdown() {}

    void RegisterMessageReceiver(
        std::function<void(ib::mw::IVAsioPeer* /*peer*/, ib::mw::ParticipantAnnouncement)> /*callback*/)
    {
    }

    void RegisterPeerShutdownCallback(std::function<void(ib::mw::IVAsioPeer* peer)> /*callback*/) {}

    void Test_SetTimeProvider(ib::mw::sync::ITimeProvider* timeProvider)
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

using MockConnectionParticipant = ib::mw::Participant<MockConnection>;

inline auto MakeMockConnectionParticipant(std::shared_ptr<ib::cfg::IParticipantConfiguration> participantConfig,
                                          const std::string& participantName)
    -> std::unique_ptr<MockConnectionParticipant>
{
    auto&& cfg = ib::mw::ValidateAndSanitizeConfig(participantConfig, participantName);
    return std::make_unique<MockConnectionParticipant>(std::move(cfg), participantName);
}

struct FixedTimeProvider : ib::mw::sync::ITimeProvider
{
    auto Now() const -> std::chrono::nanoseconds override { return now; }

    auto TimeProviderName() const -> const std::string& override
    {
        static std::string name = "FixedTimeProvider";
        return name;
    };

    HandlerId AddNextSimStepHandler(NextSimStepHandlerT) override { return {}; }

    void RemoveNextSimStepHandler(HandlerId) override {}

    std::chrono::nanoseconds now;
};

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
    static auto MakeParticipantConfiguration() -> std::shared_ptr<ib::cfg::ParticipantConfiguration>
    {
        auto configuration = std::make_shared<ib::cfg::ParticipantConfiguration>(ib::cfg::ParticipantConfiguration());

        ib::cfg::RpcClient rpcClientConfig;
        rpcClientConfig.name = "RpcClient";
        rpcClientConfig.functionName = "FunctionA";
        configuration->rpcClients.push_back(rpcClientConfig);

        ib::cfg::RpcServer rpcServerConfig;
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

} // namespace test
} // namespace rpc
} // namespace sim
} // namespace ib
