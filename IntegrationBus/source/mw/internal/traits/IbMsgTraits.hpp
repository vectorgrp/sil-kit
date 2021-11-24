// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once


namespace ib {
namespace mw {

// the ib messages type traits
template <class MsgT> struct IbMsgTraitTypeName { static constexpr const char *TypeName(); };
template <class MsgT> struct IbMsgTraitHistSize { static constexpr std::size_t HistSize() { return 0; } };

// The final message traits
template <class MsgT> struct IbMsgTraits
    : IbMsgTraitTypeName<MsgT>
    , IbMsgTraitHistSize<MsgT>
{
};

#define DefineIbMsgTrait_TypeName(Namespace, MsgName) template<> struct IbMsgTraitTypeName<Namespace::MsgName>{\
    static constexpr const  char* TypeName() { return #Namespace "::" #MsgName;}\
    };
#define DefineIbMsgTrait_HistSize(Namespace, MsgName, HistorySize) template<> struct IbMsgTraitHistSize<Namespace::MsgName>{\
    static constexpr std::size_t HistSize() { return HistorySize;} \
    };

DefineIbMsgTrait_TypeName(ib::mw::logging, LogMsg)
DefineIbMsgTrait_TypeName(ib::mw::sync, Tick)
DefineIbMsgTrait_TypeName(ib::mw::sync, TickDone)
DefineIbMsgTrait_TypeName(ib::mw::sync, QuantumRequest)
DefineIbMsgTrait_TypeName(ib::mw::sync, QuantumGrant)
DefineIbMsgTrait_TypeName(ib::mw::sync, ParticipantCommand)
DefineIbMsgTrait_TypeName(ib::mw::sync, SystemCommand)
DefineIbMsgTrait_TypeName(ib::mw::sync, ParticipantStatus)
DefineIbMsgTrait_TypeName(ib::mw::sync, NextSimTask)
DefineIbMsgTrait_TypeName(ib::sim::generic, GenericMessage)
DefineIbMsgTrait_TypeName(ib::sim::can, CanMessage)
DefineIbMsgTrait_TypeName(ib::sim::can, CanTransmitAcknowledge)
DefineIbMsgTrait_TypeName(ib::sim::can, CanControllerStatus)
DefineIbMsgTrait_TypeName(ib::sim::can, CanConfigureBaudrate)
DefineIbMsgTrait_TypeName(ib::sim::can, CanSetControllerMode)
DefineIbMsgTrait_TypeName(ib::sim::eth, EthMessage)
DefineIbMsgTrait_TypeName(ib::sim::eth, EthTransmitAcknowledge)
DefineIbMsgTrait_TypeName(ib::sim::eth, EthStatus)
DefineIbMsgTrait_TypeName(ib::sim::eth, EthSetMode)
DefineIbMsgTrait_TypeName(ib::sim::io, AnalogIoMessage)
DefineIbMsgTrait_TypeName(ib::sim::io, DigitalIoMessage)
DefineIbMsgTrait_TypeName(ib::sim::io, PatternIoMessage)
DefineIbMsgTrait_TypeName(ib::sim::io, PwmIoMessage)
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
DefineIbMsgTrait_TypeName(ib::sim::fr, ControllerStatus)
DefineIbMsgTrait_TypeName(ib::sim::fr, PocStatus)

//Messages with history
DefineIbMsgTrait_HistSize(ib::mw::sync, ParticipantStatus, 1)


} // mw
} // namespace ib
