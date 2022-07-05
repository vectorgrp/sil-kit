// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <type_traits>

#include "IbMsgVersion.hpp"
#include "IbMsgSerdesName.hpp"

namespace ib {
namespace mw {
// ==================================================================
//  Workaround for C++14 (Helper Type Alias)
// ==================================================================

// Implements std::void_t from C++17
template <typename...>
struct MakeVoidT
{
    using type = void;
};
template <typename... Ts>
using VoidT = typename MakeVoidT<Ts...>::type;

// ==================================================================
//  Trait which checks that '.timestamp' works
// ==================================================================

template <typename T, typename = void>
struct HasTimestamp : std::false_type
{
};

template <typename T>
struct HasTimestamp<T, VoidT<decltype(std::declval<std::decay_t<T>>().timestamp = std::chrono::nanoseconds{})>>
    : std::true_type
{
};


// the ib messages type traits
template <class MsgT> struct IbMsgTraitTypeName { static constexpr const char *TypeName(); };
template <class MsgT> struct IbMsgTraitHistSize { static constexpr std::size_t HistSize() { return 0; } };
template <class MsgT> struct IbMsgTraitEnforceSelfDelivery { static constexpr bool IsSelfDeliveryEnforced() { return false; } };

// The final message traits
template <class MsgT> struct IbMsgTraits
    : IbMsgTraitTypeName<MsgT>
    , IbMsgTraitHistSize<MsgT>
    , IbMsgTraitEnforceSelfDelivery<MsgT>
    , IbMsgTraitVersion<MsgT>
    , IbMsgTraitSerdesName<MsgT>
{
};

#define DefineIbMsgTrait_TypeName(Namespace, MsgName) template<> struct IbMsgTraitTypeName<Namespace::MsgName>{\
    static constexpr const char* TypeName() { return #Namespace "::" #MsgName; }\
    };
#define DefineIbMsgTrait_HistSize(Namespace, MsgName, HistorySize) template<> struct IbMsgTraitHistSize<Namespace::MsgName>{\
    static constexpr std::size_t HistSize() { return HistorySize; } \
    };
#define DefineIbMsgTrait_EnforceSelfDelivery(Namespace, MsgName) template<> struct IbMsgTraitEnforceSelfDelivery<Namespace::MsgName>{\
    static constexpr bool IsSelfDeliveryEnforced() { return true; }\
    };

DefineIbMsgTrait_TypeName(ib::mw::logging, LogMsg)
DefineIbMsgTrait_TypeName(ib::mw::sync, ParticipantCommand)
DefineIbMsgTrait_TypeName(ib::mw::sync, SystemCommand)
DefineIbMsgTrait_TypeName(ib::mw::sync, ParticipantStatus)
DefineIbMsgTrait_TypeName(ib::mw::sync, WorkflowConfiguration)
DefineIbMsgTrait_TypeName(ib::mw::sync, NextSimTask)
DefineIbMsgTrait_TypeName(ib::sim::data, DataMessageEvent)
DefineIbMsgTrait_TypeName(ib::sim::rpc, FunctionCall)
DefineIbMsgTrait_TypeName(ib::sim::rpc, FunctionCallResponse)
DefineIbMsgTrait_TypeName(ib::sim::can, CanFrameEvent)
DefineIbMsgTrait_TypeName(ib::sim::can, CanFrameTransmitEvent)
DefineIbMsgTrait_TypeName(ib::sim::can, CanControllerStatus)
DefineIbMsgTrait_TypeName(ib::sim::can, CanConfigureBaudrate)
DefineIbMsgTrait_TypeName(ib::sim::can, CanSetControllerMode)
DefineIbMsgTrait_TypeName(ib::sim::eth, EthernetFrameEvent)
DefineIbMsgTrait_TypeName(ib::sim::eth, EthernetFrameTransmitEvent)
DefineIbMsgTrait_TypeName(ib::sim::eth, EthernetStatus)
DefineIbMsgTrait_TypeName(ib::sim::eth, EthernetSetMode)
DefineIbMsgTrait_TypeName(ib::sim::lin, LinSendFrameRequest)
DefineIbMsgTrait_TypeName(ib::sim::lin, LinSendFrameHeaderRequest)
DefineIbMsgTrait_TypeName(ib::sim::lin, LinTransmission)
DefineIbMsgTrait_TypeName(ib::sim::lin, LinWakeupPulse)
DefineIbMsgTrait_TypeName(ib::sim::lin, LinControllerConfig)
DefineIbMsgTrait_TypeName(ib::sim::lin, LinControllerStatusUpdate)
DefineIbMsgTrait_TypeName(ib::sim::lin, LinFrameResponseUpdate)
DefineIbMsgTrait_TypeName(ib::sim::fr, FlexrayFrameEvent)
DefineIbMsgTrait_TypeName(ib::sim::fr, FlexrayFrameTransmitEvent)
DefineIbMsgTrait_TypeName(ib::sim::fr, FlexraySymbolEvent)
DefineIbMsgTrait_TypeName(ib::sim::fr, FlexraySymbolTransmitEvent)
DefineIbMsgTrait_TypeName(ib::sim::fr, FlexrayCycleStartEvent)
DefineIbMsgTrait_TypeName(ib::sim::fr, FlexrayHostCommand)
DefineIbMsgTrait_TypeName(ib::sim::fr, FlexrayControllerConfig)
DefineIbMsgTrait_TypeName(ib::sim::fr, FlexrayTxBufferConfigUpdate)
DefineIbMsgTrait_TypeName(ib::sim::fr, FlexrayTxBufferUpdate)
DefineIbMsgTrait_TypeName(ib::sim::fr, FlexrayPocStatusEvent)
DefineIbMsgTrait_TypeName(ib::mw::service, ParticipantDiscoveryEvent)
DefineIbMsgTrait_TypeName(ib::mw::service, ServiceDiscoveryEvent)

// Messages with history
DefineIbMsgTrait_HistSize(ib::mw::sync, ParticipantStatus, 1)
DefineIbMsgTrait_HistSize(ib::mw::service, ParticipantDiscoveryEvent, 1)
DefineIbMsgTrait_HistSize(ib::sim::data, DataMessageEvent, 1)
DefineIbMsgTrait_HistSize(ib::mw::sync, WorkflowConfiguration, 1)
DefineIbMsgTrait_HistSize(ib::sim::lin, LinControllerConfig, 1)

// Messages with enforced self delivery
DefineIbMsgTrait_EnforceSelfDelivery(ib::mw::sync, ParticipantCommand)
DefineIbMsgTrait_EnforceSelfDelivery(ib::mw::sync, ParticipantStatus)
DefineIbMsgTrait_EnforceSelfDelivery(ib::mw::sync, SystemCommand)

} // mw
} // namespace ib
