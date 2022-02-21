// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

namespace ib {
namespace mw {

// the ib messages type traits
template <class MsgT> struct IbMsgTraitTypeName { static constexpr const char *TypeName(); };
template <class MsgT> struct IbMsgTraitHistSize { static constexpr std::size_t HistSize() { return 0; } };
template <class MsgT> struct IbMsgTraitEnforceSelfDelivery { static constexpr bool IsSelfDeliveryEnforced() { return false; } };

// The final message traits
template <class MsgT> struct IbMsgTraits
    : IbMsgTraitTypeName<MsgT>
    , IbMsgTraitHistSize<MsgT>
    , IbMsgTraitEnforceSelfDelivery<MsgT>
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
DefineIbMsgTrait_TypeName(ib::mw::sync, ExpectedParticipants)
DefineIbMsgTrait_TypeName(ib::mw::sync, NextSimTask)
DefineIbMsgTrait_TypeName(ib::sim::generic, GenericMessage)
DefineIbMsgTrait_TypeName(ib::sim::data, DataMessage)
DefineIbMsgTrait_TypeName(ib::sim::rpc, FunctionCall)
DefineIbMsgTrait_TypeName(ib::sim::rpc, FunctionCallResponse)
DefineIbMsgTrait_TypeName(ib::sim::can, CanMessage)
DefineIbMsgTrait_TypeName(ib::sim::can, CanTransmitAcknowledge)
DefineIbMsgTrait_TypeName(ib::sim::can, CanControllerStatus)
DefineIbMsgTrait_TypeName(ib::sim::can, CanConfigureBaudrate)
DefineIbMsgTrait_TypeName(ib::sim::can, CanSetControllerMode)
DefineIbMsgTrait_TypeName(ib::sim::eth, EthMessage)
DefineIbMsgTrait_TypeName(ib::sim::eth, EthTransmitAcknowledge)
DefineIbMsgTrait_TypeName(ib::sim::eth, EthStatus)
DefineIbMsgTrait_TypeName(ib::sim::eth, EthSetMode)
DefineIbMsgTrait_TypeName(ib::sim::lin, SendFrameRequest)
DefineIbMsgTrait_TypeName(ib::sim::lin, SendFrameHeaderRequest)
DefineIbMsgTrait_TypeName(ib::sim::lin, Transmission)
DefineIbMsgTrait_TypeName(ib::sim::lin, WakeupPulse)
DefineIbMsgTrait_TypeName(ib::sim::lin, ControllerConfig)
DefineIbMsgTrait_TypeName(ib::sim::lin, ControllerStatusUpdate)
DefineIbMsgTrait_TypeName(ib::sim::lin, FrameResponseUpdate)
DefineIbMsgTrait_TypeName(ib::sim::fr, FrMessage)
DefineIbMsgTrait_TypeName(ib::sim::fr, FrMessageAck)
DefineIbMsgTrait_TypeName(ib::sim::fr, FrSymbol)
DefineIbMsgTrait_TypeName(ib::sim::fr, FrSymbolAck)
DefineIbMsgTrait_TypeName(ib::sim::fr, CycleStart)
DefineIbMsgTrait_TypeName(ib::sim::fr, HostCommand)
DefineIbMsgTrait_TypeName(ib::sim::fr, ControllerConfig)
DefineIbMsgTrait_TypeName(ib::sim::fr, TxBufferConfigUpdate)
DefineIbMsgTrait_TypeName(ib::sim::fr, TxBufferUpdate)
DefineIbMsgTrait_TypeName(ib::sim::fr, PocStatus)
DefineIbMsgTrait_TypeName(ib::mw::service, ServiceAnnouncement)
DefineIbMsgTrait_TypeName(ib::mw::service, ServiceDiscoveryEvent)

//Messages with history
DefineIbMsgTrait_HistSize(ib::mw::sync, ParticipantStatus, 1)
DefineIbMsgTrait_HistSize(ib::mw::service, ServiceAnnouncement, 1)
DefineIbMsgTrait_HistSize(ib::sim::data, DataMessage, 1)
DefineIbMsgTrait_HistSize(ib::mw::sync, ExpectedParticipants, 1)

//Messages with enforced self delivery
DefineIbMsgTrait_EnforceSelfDelivery(ib::mw::sync, ParticipantCommand)
DefineIbMsgTrait_EnforceSelfDelivery(ib::mw::sync, ParticipantStatus)
DefineIbMsgTrait_EnforceSelfDelivery(ib::mw::sync, SystemCommand)

} // mw
} // namespace ib
