// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once


namespace ib {
namespace mw {

template <class MsgT> struct IbMsgTraits;


#define DefineIbMsgTraits(Namespace, MsgName) template<> struct IbMsgTraits<Namespace::MsgName> { static constexpr const char* TypeName() { return #Namespace "::" #MsgName; } };

DefineIbMsgTraits(ib::mw::logging, LogMsg)
DefineIbMsgTraits(ib::mw::sync, Tick)
DefineIbMsgTraits(ib::mw::sync, TickDone)
DefineIbMsgTraits(ib::mw::sync, QuantumRequest)
DefineIbMsgTraits(ib::mw::sync, QuantumGrant)
DefineIbMsgTraits(ib::mw::sync, ParticipantCommand)
DefineIbMsgTraits(ib::mw::sync, SystemCommand)
DefineIbMsgTraits(ib::mw::sync, ParticipantStatus)
DefineIbMsgTraits(ib::mw::sync, NextSimTask)
DefineIbMsgTraits(ib::sim::generic, GenericMessage)
DefineIbMsgTraits(ib::sim::can, CanMessage)
DefineIbMsgTraits(ib::sim::can, CanTransmitAcknowledge)
DefineIbMsgTraits(ib::sim::can, CanControllerStatus)
DefineIbMsgTraits(ib::sim::can, CanConfigureBaudrate)
DefineIbMsgTraits(ib::sim::can, CanSetControllerMode)
DefineIbMsgTraits(ib::sim::eth, EthMessage)
DefineIbMsgTraits(ib::sim::eth, EthTransmitAcknowledge)
DefineIbMsgTraits(ib::sim::eth, EthStatus)
DefineIbMsgTraits(ib::sim::eth, EthSetMode)
DefineIbMsgTraits(ib::sim::io, AnalogIoMessage)
DefineIbMsgTraits(ib::sim::io, DigitalIoMessage)
DefineIbMsgTraits(ib::sim::io, PatternIoMessage)
DefineIbMsgTraits(ib::sim::io, PwmIoMessage)
DefineIbMsgTraits(ib::sim::lin, SendFrameRequest)
DefineIbMsgTraits(ib::sim::lin, SendFrameHeaderRequest)
DefineIbMsgTraits(ib::sim::lin, Transmission)
DefineIbMsgTraits(ib::sim::lin, WakeupPulse)
DefineIbMsgTraits(ib::sim::lin, ControllerConfig)
DefineIbMsgTraits(ib::sim::lin, ControllerStatusUpdate)
DefineIbMsgTraits(ib::sim::lin, FrameResponseUpdate)
DefineIbMsgTraits(ib::sim::fr, FrMessage)
DefineIbMsgTraits(ib::sim::fr, FrMessageAck)
DefineIbMsgTraits(ib::sim::fr, FrSymbol)
DefineIbMsgTraits(ib::sim::fr, FrSymbolAck)
DefineIbMsgTraits(ib::sim::fr, CycleStart)
DefineIbMsgTraits(ib::sim::fr, HostCommand)
DefineIbMsgTraits(ib::sim::fr, ControllerConfig)
DefineIbMsgTraits(ib::sim::fr, TxBufferConfigUpdate)
DefineIbMsgTraits(ib::sim::fr, TxBufferUpdate)
DefineIbMsgTraits(ib::sim::fr, ControllerStatus)


} // mw
} // namespace ib
