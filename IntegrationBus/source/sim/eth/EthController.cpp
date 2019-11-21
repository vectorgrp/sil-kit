// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthController.hpp"

#include "ib/mw/IComAdapter.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include <algorithm>

namespace ib {
namespace sim {
namespace eth {

EthController::EthController(mw::IComAdapter* comAdapter, cfg::EthernetController config)
    : _comAdapter(comAdapter)
{
    _tracingIsEnabled = (!config.pcapFile.empty() || !config.pcapPipe.empty());

    if (!config.pcapFile.empty())
    {
        _tracer.OpenFile(config.pcapFile);
    }

    if (!config.pcapPipe.empty())
    {
        _comAdapter->GetLogger()->Info("Waiting for a reader to connect to PCAP pipe {} ... ", config.pcapPipe);
        _tracer.OpenPipe(config.pcapPipe);
        _comAdapter->GetLogger()->Debug("PCAP pipe: {} is connected successfully", config.pcapPipe);
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

auto EthController::SendMessage(EthMessage msg) -> EthTxId
{
    auto txId = MakeTxId();
    _pendingAcks.emplace_back(msg.ethFrame.GetSourceMac(), txId);

    msg.transmitId = txId;
    _comAdapter->SendIbMessage(_endpointAddr, std::move(msg));

    return txId;
}

void EthController::RegisterReceiveMessageHandler(ReceiveMessageHandler handler)
{
    RegisterHandler(handler);
}

void EthController::RegisterMessageAckHandler(MessageAckHandler handler)
{
    RegisterHandler(handler);
}

void EthController::RegisterStateChangedHandler(StateChangedHandler /*handler*/)
{
    // only supported when using a Network Simulator --> implement in EthControllerProxy
}

void EthController::RegisterBitRateChangedHandler(BitRateChangedHandler /*handler*/)
{
    // only supported when using a Network Simulator --> implement in EthControllerProxy
}


void EthController::ReceiveIbMessage(mw::EndpointAddress from, const EthMessage& msg)
{
    if (from == _endpointAddr)
        return;

    if (_tracingIsEnabled) _tracer.Trace(msg);

    CallHandlers(msg);

    EthTransmitAcknowledge ack;
    ack.timestamp  = msg.timestamp;
    ack.transmitId = msg.transmitId;
    ack.sourceMac = msg.ethFrame.GetSourceMac();
    ack.status     = EthTransmitStatus::Transmitted;

    _comAdapter->SendIbMessage(_endpointAddr, ack);
}

void EthController::ReceiveIbMessage(mw::EndpointAddress from, const EthTransmitAcknowledge& msg)
{
    if (from == _endpointAddr)
        return;

    auto pendingAcksIter = std::find(_pendingAcks.begin(), _pendingAcks.end(),
        std::make_pair(msg.sourceMac, msg.transmitId));
    if (pendingAcksIter != _pendingAcks.end())
    {
        _pendingAcks.erase(pendingAcksIter);
        CallHandlers(msg);
    }
}

void EthController::SetEndpointAddress(const mw::EndpointAddress& endpointAddress)
{
    _endpointAddr = endpointAddress;
}

auto EthController::EndpointAddress() const -> const mw::EndpointAddress&
{
    return _endpointAddr;
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
        handler(this, msg);
    }
}

} // namespace eth
} // namespace sim
} // namespace ib
