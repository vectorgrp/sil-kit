// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "core/internal/internal_fwd.hpp"

#include "silkit/services/logging/LoggingDatatypes.hpp"
#include "core/internal/LoggingTopics.hpp"

namespace SilKit {
namespace Core {

// Trait template (default)
template <class SilKitComponentT>
struct TopicTraits
{
    static constexpr SilKit::Services::Logging::Topic Topic()
    {
        return SilKit::Services::Logging::Topic::None;
    }
};

// The final service traits
template <class SilKitComponentT>
struct SilKitTopicTrait : TopicTraits<SilKitComponentT>
{
};

#define DefineSilKitLoggingTrait_Topic(Component,TopicName) \
    template <> \
    struct TopicTraits<Component> \
    { \
        static constexpr SilKit::Services::Logging::Topic Topic() \
        { \
            return TopicName; \
        } \
    }

DefineSilKitLoggingTrait_Topic(SilKit::Services::Orchestration::TimeSyncService, SilKit::Services::Logging::Topic::TimeSync);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Orchestration::TimeConfiguration, SilKit::Services::Logging::Topic::TimeConfig);

template <class SilKitConnectionT>
struct TopicTraits<SilKit::Core::Participant<SilKitConnectionT>>
{
    static constexpr SilKit::Services::Logging::Topic Topic()
    {
        return SilKit::Services::Logging::Topic::Participant;
    }
};

DefineSilKitLoggingTrait_Topic(SilKit::Core::IParticipantInternal, SilKit::Services::Logging::Topic::Participant);
DefineSilKitLoggingTrait_Topic(SilKit::Core::RequestReply::ParticipantReplies, SilKit::Services::Logging::Topic::Participant);

DefineSilKitLoggingTrait_Topic(SilKit::Services::Orchestration::LifecycleService, SilKit::Services::Logging::Topic::LifeCycle);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Orchestration::LifecycleManagement, SilKit::Services::Logging::Topic::LifeCycle);

DefineSilKitLoggingTrait_Topic(SilKit::Services::Orchestration::SystemMonitor, SilKit::Services::Logging::Topic::SystemState);
DefineSilKitLoggingTrait_Topic(VSilKit::SystemStateTracker, SilKit::Services::Logging::Topic::SystemState);


template <class MsgT>
struct TopicTraits<SilKit::Core::SilKitLink<MsgT>> 
{
    static constexpr SilKit::Services::Logging::Topic Topic()
    {
        return SilKit::Services::Logging::Topic::Asio;
    }
};

DefineSilKitLoggingTrait_Topic(SilKit::Core::VAsioConnection, SilKit::Services::Logging::Topic::Asio);
DefineSilKitLoggingTrait_Topic(SilKit::Core::VAsioRegistry, SilKit::Services::Logging::Topic::Asio);
DefineSilKitLoggingTrait_Topic(SilKit::Core::ConnectKnownParticipants, SilKit::Services::Logging::Topic::Asio);
DefineSilKitLoggingTrait_Topic(SilKit::Core::RemoteConnectionManager, SilKit::Services::Logging::Topic::Asio);
DefineSilKitLoggingTrait_Topic(SilKit::Core::VAsioPeer, SilKit::Services::Logging::Topic::Asio);
DefineSilKitLoggingTrait_Topic(VSilKit::ConnectPeer, SilKit::Services::Logging::Topic::Asio);
DefineSilKitLoggingTrait_Topic(VSilKit::AsioGenericRawByteStream, SilKit::Services::Logging::Topic::Asio);


DefineSilKitLoggingTrait_Topic(SilKit::Dashboard::DashboardRestClient, SilKit::Services::Logging::Topic::Dashboard);
DefineSilKitLoggingTrait_Topic(SilKit::Dashboard::DashboardSystemServiceClient, SilKit::Services::Logging::Topic::Dashboard);
DefineSilKitLoggingTrait_Topic(SilKit::Dashboard::DashboardInstance, SilKit::Services::Logging::Topic::Dashboard);

DefineSilKitLoggingTrait_Topic(VSilKit::MetricsProcessor, SilKit::Services::Logging::Topic::Metrics);

DefineSilKitLoggingTrait_Topic(SilKit::Experimental::NetworkSimulation::NetworkSimulatorInternal, SilKit::Services::Logging::Topic::NetSim);
DefineSilKitLoggingTrait_Topic(SilKit::Experimental::NetworkSimulation::SimulatedNetworkInternal, SilKit::Services::Logging::Topic::NetSim);
DefineSilKitLoggingTrait_Topic(SilKit::Experimental::NetworkSimulation::SimulatedNetworkRouter, SilKit::Services::Logging::Topic::NetSim);

DefineSilKitLoggingTrait_Topic(SilKit::Services::Lin::SimBehaviorTrivial, SilKit::Services::Logging::Topic::Lin);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Lin::LinController, SilKit::Services::Logging::Topic::Lin);

DefineSilKitLoggingTrait_Topic(SilKit::Services::Can::SimBehaviorTrivial, SilKit::Services::Logging::Topic::Can);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Can::CanController, SilKit::Services::Logging::Topic::Can);

DefineSilKitLoggingTrait_Topic(SilKit::Services::Ethernet::EthController, SilKit::Services::Logging::Topic::Ethernet);

DefineSilKitLoggingTrait_Topic(SilKit::Services::Flexray::FlexrayController, SilKit::Services::Logging::Topic::Flexray);

DefineSilKitLoggingTrait_Topic(SilKit::Services::PubSub::DataSubscriberInternal, SilKit::Services::Logging::Topic::Pubsub);

DefineSilKitLoggingTrait_Topic(SilKit::Services::Rpc::RpcServerInternal, SilKit::Services::Logging::Topic::Rpc);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Rpc::RpcDiscoverer, SilKit::Services::Logging::Topic::Rpc);

 
DefineSilKitLoggingTrait_Topic(SilKit::Tracing::ReplayScheduler, SilKit::Services::Logging::Topic::Tracing);
DefineSilKitLoggingTrait_Topic(SilKit::Tracing::PcapSink, SilKit::Services::Logging::Topic::Tracing);

// Message type to topic mappings (used by TraceMessageCommon)
DefineSilKitLoggingTrait_Topic(SilKit::Services::Can::WireCanFrameEvent, SilKit::Services::Logging::Topic::Can);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Can::CanFrameTransmitEvent, SilKit::Services::Logging::Topic::Can);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Can::CanControllerStatus, SilKit::Services::Logging::Topic::Can);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Can::CanConfigureBaudrate, SilKit::Services::Logging::Topic::Can);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Can::CanSetControllerMode, SilKit::Services::Logging::Topic::Can);

DefineSilKitLoggingTrait_Topic(SilKit::Services::Ethernet::WireEthernetFrameEvent, SilKit::Services::Logging::Topic::Ethernet);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Ethernet::EthernetFrameTransmitEvent, SilKit::Services::Logging::Topic::Ethernet);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Ethernet::EthernetStatus, SilKit::Services::Logging::Topic::Ethernet);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Ethernet::EthernetSetMode, SilKit::Services::Logging::Topic::Ethernet);

DefineSilKitLoggingTrait_Topic(SilKit::Services::Flexray::WireFlexrayFrameEvent, SilKit::Services::Logging::Topic::Flexray);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Flexray::WireFlexrayFrameTransmitEvent, SilKit::Services::Logging::Topic::Flexray);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Flexray::FlexraySymbolEvent, SilKit::Services::Logging::Topic::Flexray);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Flexray::FlexraySymbolTransmitEvent, SilKit::Services::Logging::Topic::Flexray);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Flexray::FlexrayCycleStartEvent, SilKit::Services::Logging::Topic::Flexray);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Flexray::FlexrayHostCommand, SilKit::Services::Logging::Topic::Flexray);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Flexray::FlexrayControllerConfig, SilKit::Services::Logging::Topic::Flexray);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Flexray::FlexrayTxBufferConfigUpdate, SilKit::Services::Logging::Topic::Flexray);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Flexray::WireFlexrayTxBufferUpdate, SilKit::Services::Logging::Topic::Flexray);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Flexray::FlexrayPocStatusEvent, SilKit::Services::Logging::Topic::Flexray);

DefineSilKitLoggingTrait_Topic(SilKit::Services::Lin::LinSendFrameRequest, SilKit::Services::Logging::Topic::Lin);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Lin::LinSendFrameHeaderRequest, SilKit::Services::Logging::Topic::Lin);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Lin::LinTransmission, SilKit::Services::Logging::Topic::Lin);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Lin::LinWakeupPulse, SilKit::Services::Logging::Topic::Lin);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Lin::WireLinControllerConfig, SilKit::Services::Logging::Topic::Lin);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Lin::LinControllerStatusUpdate, SilKit::Services::Logging::Topic::Lin);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Lin::LinFrameResponseUpdate, SilKit::Services::Logging::Topic::Lin);

DefineSilKitLoggingTrait_Topic(SilKit::Services::PubSub::WireDataMessageEvent, SilKit::Services::Logging::Topic::Pubsub);

DefineSilKitLoggingTrait_Topic(SilKit::Services::Rpc::FunctionCall, SilKit::Services::Logging::Topic::Rpc);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Rpc::FunctionCallResponse, SilKit::Services::Logging::Topic::Rpc);

DefineSilKitLoggingTrait_Topic(SilKit::Core::Discovery::ParticipantDiscoveryEvent, SilKit::Services::Logging::Topic::ServiceDiscovery);
DefineSilKitLoggingTrait_Topic(SilKit::Core::Discovery::ServiceDiscoveryEvent, SilKit::Services::Logging::Topic::ServiceDiscovery);

DefineSilKitLoggingTrait_Topic(SilKit::Core::RequestReply::RequestReplyCall, SilKit::Services::Logging::Topic::RequestReply);
DefineSilKitLoggingTrait_Topic(SilKit::Core::RequestReply::RequestReplyCallReturn, SilKit::Services::Logging::Topic::RequestReply);

DefineSilKitLoggingTrait_Topic(SilKit::Services::Orchestration::SystemCommand, SilKit::Services::Logging::Topic::LifeCycle);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Orchestration::ParticipantStatus, SilKit::Services::Logging::Topic::LifeCycle);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Orchestration::WorkflowConfiguration, SilKit::Services::Logging::Topic::LifeCycle);
DefineSilKitLoggingTrait_Topic(SilKit::Services::Orchestration::NextSimTask, SilKit::Services::Logging::Topic::TimeSync);

DefineSilKitLoggingTrait_Topic(VSilKit::MetricsUpdate, SilKit::Services::Logging::Topic::Metrics);

} // namespace Core

template <class ServiceT>
SilKit::Services::Logging::Topic TopicOf(ServiceT&&)
{
    return SilKit::Core::SilKitTopicTrait<std::decay_t<ServiceT>>::Topic(); // todo: replace decay_t with std::remove_reference_t once we require C++20
}

} // namespace SilKit

namespace VSilKit
{
using SilKit::TopicOf;
}