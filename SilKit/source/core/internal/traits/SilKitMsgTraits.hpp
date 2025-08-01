// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>

#include "SilKitMsgVersion.hpp"
#include "SilKitMsgSerdesName.hpp"

namespace SilKit {
namespace Core {
// ==================================================================
//  Trait which checks that '.timestamp' works
// ==================================================================

template <typename T, typename = void>
struct HasTimestamp : std::false_type
{
};

template <typename T>
struct HasTimestamp<T, std::void_t<decltype(std::declval<std::decay_t<T>>().timestamp = std::chrono::nanoseconds{})>>
    : std::true_type
{
};


// the silkit messages type traits
template <class MsgT>
struct SilKitMsgTraitTypeName
{
    static constexpr const char* TypeName();
};
template <class MsgT>
struct SilKitMsgTraitHistSize
{
    static constexpr std::size_t HistSize()
    {
        return 0;
    }
};
template <class MsgT>
struct SilKitMsgTraitEnforceSelfDelivery
{
    static constexpr bool IsSelfDeliveryEnforced()
    {
        return false;
    }
};
template <class MsgT>
struct SilKitMsgTraitForbidSelfDelivery
{
    static constexpr bool IsSelfDeliveryForbidden()
    {
        return false;
    }
};

// The final message traits
template <class MsgT>
struct SilKitMsgTraits
    : SilKitMsgTraitTypeName<MsgT>
    , SilKitMsgTraitHistSize<MsgT>
    , SilKitMsgTraitEnforceSelfDelivery<MsgT>
    , SilKitMsgTraitVersion<MsgT>
    , SilKitMsgTraitSerdesName<MsgT>
    , SilKitMsgTraitForbidSelfDelivery<MsgT>
{
};

#define DefineSilKitMsgTrait_TypeName(Namespace, MsgName) \
    template <> \
    struct SilKitMsgTraitTypeName<Namespace::MsgName> \
    { \
        static constexpr const char* TypeName() \
        { \
            return #Namespace "::" #MsgName; \
        } \
    }

#define DefineSilKitMsgTrait_HistSize(Namespace, MsgName, HistorySize) \
    template <> \
    struct SilKitMsgTraitHistSize<Namespace::MsgName> \
    { \
        static constexpr std::size_t HistSize() \
        { \
            return HistorySize; \
        } \
    }

#define DefineSilKitMsgTrait_EnforceSelfDelivery(Namespace, MsgName) \
    template <> \
    struct SilKitMsgTraitEnforceSelfDelivery<Namespace::MsgName> \
    { \
        static constexpr bool IsSelfDeliveryEnforced() \
        { \
            return true; \
        } \
    }

#define DefineSilKitMsgTrait_ForbidSelfDelivery(Namespace, MsgName) \
    template <> \
    struct SilKitMsgTraitForbidSelfDelivery<Namespace::MsgName> \
    { \
        static constexpr bool IsSelfDeliveryForbidden() \
        { \
            return true; \
        } \
    }

DefineSilKitMsgTrait_TypeName(SilKit::Services::Logging, LogMsg);
DefineSilKitMsgTrait_TypeName(VSilKit, MetricsUpdate);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Orchestration, SystemCommand);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Orchestration, ParticipantStatus);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Orchestration, WorkflowConfiguration);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Orchestration, NextSimTask);
DefineSilKitMsgTrait_TypeName(SilKit::Services::PubSub, WireDataMessageEvent);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Rpc, FunctionCall);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Rpc, FunctionCallResponse);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Can, WireCanFrameEvent);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Can, CanFrameTransmitEvent);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Can, CanControllerStatus);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Can, CanConfigureBaudrate);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Can, CanSetControllerMode);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Ethernet, WireEthernetFrameEvent);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Ethernet, EthernetFrameTransmitEvent);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Ethernet, EthernetStatus);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Ethernet, EthernetSetMode);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Lin, LinSendFrameRequest);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Lin, LinSendFrameHeaderRequest);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Lin, LinTransmission);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Lin, LinWakeupPulse);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Lin, WireLinControllerConfig);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Lin, LinControllerStatusUpdate);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Lin, LinFrameResponseUpdate);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Flexray, WireFlexrayFrameEvent);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Flexray, WireFlexrayFrameTransmitEvent);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Flexray, FlexraySymbolEvent);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Flexray, FlexraySymbolTransmitEvent);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Flexray, FlexrayCycleStartEvent);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Flexray, FlexrayHostCommand);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Flexray, FlexrayControllerConfig);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Flexray, FlexrayTxBufferConfigUpdate);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Flexray, WireFlexrayTxBufferUpdate);
DefineSilKitMsgTrait_TypeName(SilKit::Services::Flexray, FlexrayPocStatusEvent);
DefineSilKitMsgTrait_TypeName(SilKit::Core::Discovery, ParticipantDiscoveryEvent);
DefineSilKitMsgTrait_TypeName(SilKit::Core::Discovery, ServiceDiscoveryEvent);
DefineSilKitMsgTrait_TypeName(SilKit::Core::RequestReply, RequestReplyCall);
DefineSilKitMsgTrait_TypeName(SilKit::Core::RequestReply, RequestReplyCallReturn);

// Messages with history
DefineSilKitMsgTrait_HistSize(SilKit::Services::Orchestration, ParticipantStatus, 1);
DefineSilKitMsgTrait_HistSize(SilKit::Core::Discovery, ParticipantDiscoveryEvent, 1);
DefineSilKitMsgTrait_HistSize(SilKit::Services::PubSub, WireDataMessageEvent, 1);
DefineSilKitMsgTrait_HistSize(SilKit::Services::Orchestration, WorkflowConfiguration, 1);
DefineSilKitMsgTrait_HistSize(SilKit::Services::Lin, WireLinControllerConfig, 1);

// Messages with enforced self delivery
DefineSilKitMsgTrait_EnforceSelfDelivery(SilKit::Services::Orchestration, ParticipantStatus);
DefineSilKitMsgTrait_EnforceSelfDelivery(SilKit::Services::Lin, LinSendFrameHeaderRequest);

// Messages with forbidden self delivery
DefineSilKitMsgTrait_ForbidSelfDelivery(SilKit::Services::Orchestration, SystemCommand);

} // namespace Core
} // namespace SilKit
