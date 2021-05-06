// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "FrController.hpp"
#include "Validation.hpp"

#include <stdexcept>

#include "ib/sim/exceptions.hpp"
#include "ib/mw/IComAdapter.hpp"
#include "ib/mw/logging/ILogger.hpp"

namespace ib {
namespace sim {
namespace fr {


FrController::FrController(mw::IComAdapter* comAdapter, mw::sync::ITimeProvider* timeProvider)
    : _comAdapter{comAdapter}
    , _timeProvider{timeProvider}
{
}

void FrController::Configure(const ControllerConfig& config)
{
    Validate(config.clusterParams);
    Validate(config.nodeParams);

    _clusterParams = config.clusterParams;
    _nodeParams = config.nodeParams;
    _bufferConfigs = config.bufferConfigs;

    // tell the application that the controller is ready, depecrated callbacks
    ControllerStatus status;
    status.pocState = PocState::Ready;
    CallHandlers(status);
    //new API uses a more complete POCStatus type
    PocStatus poc{};
    poc.state = PocState::Ready;
    CallHandlers(poc);
}

void FrController::ReconfigureTxBuffer(uint16_t txBufferIdx, const TxBufferConfig& config)
{
    if (txBufferIdx >= _bufferConfigs.size())
    {
        _comAdapter->GetLogger()->Error("FrController::ReconfigureTxBuffer() was called with unconfigured txBufferIdx={}", txBufferIdx);
        throw std::out_of_range{"Unconfigured txBufferIdx!"};
    }
    _bufferConfigs[txBufferIdx] = config;
}

void FrController::UpdateTxBuffer(const TxBufferUpdate& update)
{
    if (update.txBufferIndex >= _bufferConfigs.size())
    {
        _comAdapter->GetLogger()->Error("FrController::UpdateTxBuffer() was called with unconfigured txBufferIndex={}", update.txBufferIndex);
        throw std::out_of_range{"Unconfigured txBufferIndex!"};
    }

    auto&& config = _bufferConfigs[update.txBufferIndex];

    Header header;
    header.Set(Header::Flag::PPIndicator, config.hasPayloadPreambleIndicator);
    header.Set(Header::Flag::NFIndicator, update.payloadDataValid);
    header.frameId = config.slotId;
    header.payloadLength = static_cast<decltype(header.payloadLength)>((update.payload.size() + 1) / 2); // payloadLength in 16bit words, rounded up
    header.headerCrc = config.headerCrc;

    FrMessage msg;
    msg.frame.header = header;
    msg.frame.payload = update.payload;
    msg.timestamp = _timeProvider->Now();

    switch (config.channels)
    {
    case Channel::None:
        throw std::runtime_error("Channel must not be None");
    case Channel::A:
        msg.channel = Channel::A;
        _tracer.Trace(extensions::Direction::Send, msg.timestamp, msg);
        SendIbMessage(msg);
        break;
    case Channel::B:
        msg.channel = Channel::B;
        _tracer.Trace(extensions::Direction::Send, msg.timestamp, msg);
        SendIbMessage(msg);
        break;
    case Channel::AB:
        msg.channel = Channel::A;
        _tracer.Trace(extensions::Direction::Send, msg.timestamp, msg);
        SendIbMessage(msg);

        msg.channel = Channel::B;
        _tracer.Trace(extensions::Direction::Send, msg.timestamp, msg);
        SendIbMessage(msg);
        break;
    }
}


void FrController::Run()
{
    // only supported when using a Network Simulator --> implement in FrControllerProxy

    // still send symbol to emulate startup control flow
    FrSymbol msg;
    msg.pattern = SymbolPattern::CasMts;

    msg.channel = Channel::A;
    SendIbMessage(msg);

    msg.channel = Channel::B;
    SendIbMessage(msg);

    // tell the application that POC is in NORMAL_ACTIVE state, i.e., the FlexRay cycle is
    // up and running
    ControllerStatus status;
    status.pocState = PocState::NormalActive;
    CallHandlers(status);

    PocStatus poc{};
    poc.state = PocState::NormalActive;
    CallHandlers(poc);
}

void FrController::DeferredHalt()
{
    // only supported when using a Network Simulator --> implement in FrControllerProxy
}

void FrController::Freeze()
{
    // only supported when using a Network Simulator --> implement in FrControllerProxy
}

void FrController::AllowColdstart()
{
    // only supported when using a Network Simulator --> implement in FrControllerProxy
}

void FrController::AllSlots()
{
    // only supported when using a Network Simulator --> implement in FrControllerProxy
}

void FrController::Wakeup()
{
    FrSymbol wus;
    wus.pattern = SymbolPattern::Wus;
    wus.channel = _nodeParams.pWakeupChannel;
    SendIbMessage(wus);

    // tell the application that POC is in WAKEUP state, i.e., the FlexRay cycle is
    // waking up and transitioning into READY
    ControllerStatus status;
    status.pocState = PocState::Wakeup;
    CallHandlers(status);

    PocStatus poc{};
    poc.state = PocState::Wakeup;
    CallHandlers(poc);

    // tell the application that POC is in READY state, i.e., the FlexRay cycle is
    // waking up and transitioning into READY
    status.pocState = PocState::Ready;
    CallHandlers(status);

    poc.state = PocState::Ready;
    CallHandlers(poc);
}

void FrController::RegisterMessageHandler(MessageHandler handler)
{
    RegisterHandler(handler);
}

void FrController::RegisterMessageAckHandler(MessageAckHandler handler)
{
    RegisterHandler(handler);
}

void FrController::RegisterWakeupHandler(WakeupHandler handler)
{
    _wakeupHandlers.push_back(handler);
}

void FrController::RegisterControllerStatusHandler(ControllerStatusHandler handler)
{
    RegisterHandler(handler);
}

void FrController::RegisterPocStatusHandler(PocStatusHandler handler)
{
    RegisterHandler(handler);
}

void FrController::RegisterSymbolHandler(SymbolHandler handler)
{
    RegisterHandler(handler);
}

void FrController::RegisterSymbolAckHandler(SymbolAckHandler handler)
{
    RegisterHandler(handler);
}

void FrController::RegisterCycleStartHandler(CycleStartHandler handler)
{
    // Only supported in VIBE simulation
    _comAdapter->GetLogger()->Info("CycleStartHandler callback is not supported in basic FlexRay simulation.");
}

void FrController::ReceiveIbMessage(mw::EndpointAddress from, const FrMessage& msg)
{
    if (from == _endpointAddr)
        return;

    _tracer.Trace(extensions::Direction::Receive, msg.timestamp, msg);

    CallHandlers(msg);

    FrMessageAck ack;
    ack.timestamp = msg.timestamp;
    ack.txBufferIndex = 0;
    ack.channel = msg.channel;
    ack.frame = msg.frame;

    SendIbMessage(std::move(ack));
}

void FrController::ReceiveIbMessage(mw::EndpointAddress from, const FrMessageAck& msg)
{
    if (from == _endpointAddr)
        return;

    CallHandlers(msg);
}

void FrController::ReceiveIbMessage(mw::EndpointAddress from, const FrSymbol& msg)
{
    if (from == _endpointAddr)
        return;

    // Call WakeupHandler for Wus and Wudop symbols
    switch (msg.pattern)
    {
    case SymbolPattern::CasMts:
        break;
    case SymbolPattern::Wus:
    case SymbolPattern::Wudop:
        for (auto&& handler : _wakeupHandlers)
        {
            handler(this, msg);
        }
        break;
    }

    // call SymbolHandlers
    CallHandlers(msg);

    // Acknowledge symbol reception
    FrSymbolAck ack;
    ack.timestamp = msg.timestamp;
    ack.channel = msg.channel;
    ack.pattern = msg.pattern;

    SendIbMessage(ack);
}

void FrController::ReceiveIbMessage(mw::EndpointAddress from, const FrSymbolAck& msg)
{
    if (from == _endpointAddr)
        return;

    // Switch back to the READY state if its an Ack for WUS or WUDOP
    switch (msg.pattern)
    {
    case SymbolPattern::CasMts:
    case SymbolPattern::Wudop:
        break;
    case SymbolPattern::Wus:
        ControllerStatus status;
        status.pocState = PocState::Ready;
        CallHandlers(status);

        PocStatus poc{};
        poc.state = PocState::Ready;
        CallHandlers(poc);
        break;
    }

    CallHandlers(msg);
}

void FrController::SetEndpointAddress(const mw::EndpointAddress& endpointAddress)
{
    _endpointAddr = endpointAddress;
}

auto FrController::EndpointAddress() const -> const mw::EndpointAddress&
{
    return _endpointAddr;
}

void FrController::SetTimeProvider(mw::sync::ITimeProvider* timeProvider)
{
    _timeProvider = timeProvider;
}

template<typename MsgT>
void FrController::RegisterHandler(CallbackT<MsgT> handler)
{
    auto&& handlers = std::get<CallbackVector<MsgT>>(_callbacks);
    handlers.push_back(handler);
}

template<typename MsgT>
void FrController::CallHandlers(const MsgT& msg)
{
    auto&& handlers = std::get<CallbackVector<MsgT>>(_callbacks);
    for (auto&& handler : handlers)
    {
        handler(this, msg);
    }
}

template<typename MsgT>
void FrController::SendIbMessage(MsgT&& msg)
{
    _comAdapter->SendIbMessage(_endpointAddr, std::forward<MsgT>(msg));
}


} // namespace fr
} // SimModels
} // namespace ib
