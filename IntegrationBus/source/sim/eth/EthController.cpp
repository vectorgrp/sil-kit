// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthController.hpp"
#include "PcapSink.hpp"

#include "ib/mw/logging/ILogger.hpp"

#include <algorithm>

namespace ib {
namespace sim {
namespace eth {

EthController::EthController(mw::IParticipantInternal* participant, cfg::EthernetController /*config*/,
                             mw::sync::ITimeProvider* timeProvider, IEthernetController* facade)
    : _participant{participant}
    , _timeProvider{timeProvider}
    , _facade{facade}
{
    if (facade == nullptr)
    {
        _facade = this;
    }
}

void EthController::Activate()
{
    // only supported when using a Network Simulator --> implement in EthControllerProxy
}

void EthController::Deactivate()
{
    // only supported when using a Network Simulator --> implement in EthControllerProxy
}

auto EthController::SendFrameEvent(EthernetFrameEvent msg) -> EthernetTxId
{
    auto txId = MakeTxId();
    _pendingAcks.emplace_back(msg.ethFrame.GetSourceMac(), txId);

    msg.transmitId = txId;

    _tracer.Trace(ib::sim::TransmitDirection::TX, msg.timestamp, msg.ethFrame);

    _participant->SendIbMessage(this, std::move(msg));

    EthernetFrameTransmitEvent ack;
    ack.timestamp = msg.timestamp;
    ack.transmitId = msg.transmitId;
    ack.sourceMac = msg.ethFrame.GetSourceMac();
    ack.status = EthernetTransmitStatus::Transmitted;
    CallHandlers(ack);

    return txId;
}

auto EthController::SendFrame(EthernetFrame frame) -> EthernetTxId
{
    EthernetFrameEvent msg{};
    msg.timestamp = _timeProvider->Now();
    msg.ethFrame = std::move(frame);


    return SendFrameEvent(std::move(msg));
}

void EthController::AddFrameHandler(FrameHandler handler)
{
    RegisterHandler(handler);
}

void EthController::AddFrameTransmitHandler(FrameTransmitHandler handler)
{
    RegisterHandler(handler);
}

void EthController::AddStateChangeHandler(StateChangeHandler /*handler*/)
{
    // only supported when using a Network Simulator --> implement in EthControllerProxy
}

void EthController::AddBitrateChangeHandler(BitrateChangeHandler /*handler*/)
{
    // only supported when using a Network Simulator --> implement in EthControllerProxy
}


void EthController::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const EthernetFrameEvent& msg)
{
    _tracer.Trace(ib::sim::TransmitDirection::RX, msg.timestamp, msg.ethFrame);

    CallHandlers(msg);
}

template<typename MsgT>
void EthController::RegisterHandler(CallbackT<MsgT> handler)
{
    auto&& handlers = std::get<CallbackVector<MsgT>>(_callbacks);
    handlers.push_back(handler);
}


template<typename MsgT>
void EthController::CallHandlers(const MsgT& msg)
{
    auto&& handlers = std::get<CallbackVector<MsgT>>(_callbacks);
    for (auto&& handler : handlers)
    {
        handler(_facade, msg);
    }
}

void EthController::SetTimeProvider(ib::mw::sync::ITimeProvider* timeProvider)
{
    _timeProvider = timeProvider;
}
} // namespace eth
} // namespace sim
} // namespace ib
