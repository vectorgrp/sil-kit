// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "LinControllerProxy.hpp"

#include <iostream>

#include "ib/mw/IComAdapter.hpp"

namespace ib {
namespace sim {
namespace lin {

LinControllerProxy::LinControllerProxy(mw::IComAdapter* comAdapter)
: _comAdapter(comAdapter)
{
}

void LinControllerProxy::SetMasterMode()
{
    _controllerMode = ControllerMode::Master;
    sendControllerConfig();
}

void LinControllerProxy::SetSlaveMode()
{
    _controllerMode = ControllerMode::Slave;
    sendControllerConfig();
}

void LinControllerProxy::SetBaudRate(uint32_t baudrate)
{
    _baudrate = baudrate;
    sendControllerConfig();
}

void LinControllerProxy::sendControllerConfig()
{
    ControllerConfig config;
    config.controllerMode = _controllerMode;
    config.baudrate = _baudrate;

    SendIbMessage(config);
}

void LinControllerProxy::SetSlaveConfiguration(const SlaveConfiguration& config)
{
    SendIbMessage(config);
}

void LinControllerProxy::SetResponse(LinId linId, const Payload& payload)
{
    SlaveResponse response;
    response.linId = linId;
    response.payload = payload;
    response.checksumModel = ChecksumModel::Undefined;

    SendIbMessage(response);
}

void LinControllerProxy::SetResponseWithChecksum(LinId linId, const Payload& payload, ChecksumModel checksumModel)
{
    if (checksumModel == ChecksumModel::Undefined)
    {
        std::cerr << "WARNING: LinControllerProxy::SetResponseWithChecksum() was called with ChecksumModel::Undefined, which does NOT alter the checksum model!\n";
    }

    SlaveResponse response;
    response.linId = linId;
    response.payload = payload;
    response.checksumModel = checksumModel;

    SendIbMessage(response);
}

void LinControllerProxy::RemoveResponse(LinId /*linId*/)
{
    throw std::runtime_error("LinControllerProxy::RemoveResponse not implemented");
}

void LinControllerProxy::SendMessage(const LinMessage& msg)
{
    auto msgCopy = msg;
    msgCopy.status = MessageStatus::TxSuccess;

    SendIbMessage(msgCopy);
}

void LinControllerProxy::RequestMessage(const RxRequest& request)
{
    SendIbMessage(request);
}

void LinControllerProxy::RegisterTxCompleteHandler(TxCompleteHandler handler)
{
    RegisterHandler(handler);
}

void LinControllerProxy::RegisterReceiveMessageHandler(ReceiveMessageHandler handler)
{
    RegisterHandler(handler);
}

void LinControllerProxy::ReceiveIbMessage(mw::EndpointAddress from, const LinMessage& msg)
{
    if (from.participant == _endpointAddr.participant || from.endpoint != _endpointAddr.endpoint)
        return;

    CallHandlers(msg);
}

void LinControllerProxy::ReceiveIbMessage(mw::EndpointAddress from, const TxAcknowledge& msg)
{
    if (from.participant == _endpointAddr.participant || from.endpoint != _endpointAddr.endpoint)
        return;

    if (_controllerMode != ControllerMode::Master)
    {
        std::cerr << "LinControllerProxy"
                  << "{P=" << _endpointAddr.participant << ", E=" << _endpointAddr.endpoint << "}"
                  << " in non-Master mode received TxAcknowledge!";
        return;
    }

    CallHandlers(msg.status);
}

void LinControllerProxy::ReceiveIbMessage(mw::EndpointAddress from, const WakeupRequest& msg)
{

}

void LinControllerProxy::SetEndpointAddress(const mw::EndpointAddress& endpointAddress)
{
    _endpointAddr = endpointAddress;
}

auto LinControllerProxy::EndpointAddress() const -> const mw::EndpointAddress&
{
    return _endpointAddr;
}

template<typename MsgT>
void LinControllerProxy::RegisterHandler(CallbackT<MsgT> handler)
{
    auto&& handlers = std::get<CallbackVector<MsgT>>(_callbacks);
    handlers.push_back(handler);
}

template<typename MsgT>
void LinControllerProxy::CallHandlers(const MsgT& msg)
{
    auto&& handlers = std::get<CallbackVector<MsgT>>(_callbacks);
    for (auto&& handler : handlers)
    {
        handler(this, msg);
    }
}

template <typename MsgT>
void LinControllerProxy::SendIbMessage(MsgT&& msg)
{
    _comAdapter->SendIbMessage(_endpointAddr, std::forward<MsgT>(msg));
}


} // namespace lin
} // namespace sim
} // namespace ib
