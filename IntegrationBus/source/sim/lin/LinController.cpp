// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "LinController.hpp"

#include <cassert>
#include <algorithm>
#include <iostream>

#include "ib/mw/IComAdapter.hpp"

namespace ib {
namespace sim {
namespace lin {

LinController::LinController(::ib::mw::IComAdapter* comAdapter)
: _comAdapter(comAdapter)
{
}

void LinController::SetMasterMode()
{
    _controllerMode = ControllerMode::Master;
}

void LinController::SetSlaveMode()
{
    _controllerMode = ControllerMode::Slave;

    ControllerConfig config;
    config.controllerMode = _controllerMode;

    SendIbMessage(config);
}

void LinController::SetBaudRate(uint32_t /*rate*/)
{
    // Baudrate is only considered when using a LIN Network Simulator, i.e., in the LinControllerProxy
}

void LinController::SetSlaveConfiguration(const SlaveConfiguration& config)
{
    SendIbMessage(config);
}

void LinController::SetResponse(LinId linId, const Payload& payload)
{
    SlaveResponse response;
    response.linId = linId;
    response.payload = payload;

    SendIbMessage(response);
}

void LinController::RemoveResponse(LinId linId)
{
    // Fixme: Implement Remove Response
    std::cerr << "LinController::RemoveResponse() is not implemented\n";
}

void LinController::SendMessage(const LinMessage& msg)
{
    auto msgCopy{msg};

    msgCopy.status = MessageStatus::TxSuccess;

    SendIbMessage(msgCopy);

    // We always indicate that the Frame has been transmitted successfully, i.e., there are no conflicts.
    CallHandlers(MessageStatus::TxSuccess);
}

void LinController::RequestMessage(const RxRequest& msg)
{
    if (_controllerMode != ControllerMode::Master)
    {
        std::cerr << "ERROR: LinController::RequestMessage must only be called in master mode!" << std::endl;
        throw std::logic_error("LinController::RequestMessage must only be called in master mode!");
    }

    // we answer the call immediately based on the cached responses
    // setup a reply
    LinMessage reply{};
    reply.linId = msg.linId;

    auto numResponses = 0;
    for (auto&& keyValue: _linSlaves)
    {
        auto&& slave = keyValue.second;

        if (msg.linId >= slave.responses.size())
            continue;

        const auto& response = slave.responses[msg.linId];

        if (response.responseMode == ResponseMode::TxUnconditional)
        {
            reply.payload = response.payload;
            reply.checksumModel = response.checksumModel;

            numResponses++;
        }
    }

    if (numResponses == 0)
    {
        reply.status = MessageStatus::RxNoResponse;
    }
    else if (numResponses == 1)
    {
        reply.status = MessageStatus::RxSuccess;
    }
    else if (numResponses > 1)
    {
        reply.status = MessageStatus::RxResponseError;
    }

    // dispatch the reply locally...
    CallHandlers(reply);
    // ... and remotely
    SendIbMessage(reply);
}

void LinController::RegisterTxCompleteHandler(TxCompleteHandler handler)
{
    RegisterHandler(handler);
}

void LinController::RegisterReceiveMessageHandler(ReceiveMessageHandler handler)
{
    RegisterHandler(handler);
}

template<typename MsgT>
void LinController::RegisterHandler(CallbackT<MsgT> handler)
{
    auto&& handlers = std::get<CallbackVector<MsgT>>(_callbacks);
    handlers.push_back(handler);
}

void LinController::ReceiveIbMessage(ib::mw::EndpointAddress from, const LinMessage& msg)
{
    if (from == _endpointAddr)
        return;

    switch (_controllerMode)
    {
    case ControllerMode::Inactive:
        return;

    case ControllerMode::Master:
        std::cerr << "WARNING: LinController in MasterMode received a LinMessage, probably originating from another master. This indicates an erroneous setup!\n";
        //[[fallthrough]]

    case ControllerMode::Slave:
        CallHandlers(msg);
        return;

    default:
        std::cerr << "WARNING: Unhandled ControllerMode in LinController::ReceiveIbMessage(..., LinMessage)\n";
        return;
    }
}

void LinController::ReceiveIbMessage(mw::EndpointAddress from, const ControllerConfig& msg)
{
    if (from == _endpointAddr)
        return;

    // only controllers in master mode are responsible for managing responses.
    if (_controllerMode != ControllerMode::Master)
        return;

    if (msg.controllerMode != ControllerMode::Slave)
    {
        std::cerr << "WARNING: LinController received ControllerConfig with mode != Slave, which will be ignored\n";
        return;
    }

    auto&& linSlave = GetLinSlave(from);
    linSlave.config = msg;
}

void LinController::ReceiveIbMessage(mw::EndpointAddress from, const SlaveConfiguration& msg)
{
    if (from == _endpointAddr)
        return;

    // only controllers in master mode are responsible for managing responses.
    if (_controllerMode != ControllerMode::Master)
        return;

    if (!IsKnownSlave(from))
    {
        std::cerr << "Warning: LinController received SlaveConfiguration for unkonwn LIN Slave {" << from.participant << ", " << from.endpoint << "}\n";
        return;
    }

    auto&& linSlave = GetLinSlave(from);
    linSlave.responses.resize(msg.responseConfigs.size());

    for (size_t i = 0u; i < msg.responseConfigs.size(); i++)
    {
        static_cast<SlaveResponseConfig&>(linSlave.responses[i]) = msg.responseConfigs[i];
    }
}

void LinController::ReceiveIbMessage(mw::EndpointAddress from, const SlaveResponse& msg)
{
    if (from == _endpointAddr)
        return;


    // only controllers in master mode are responsible for managing responses.
    if (_controllerMode != ControllerMode::Master)
        return;

    if (!IsKnownSlave(from))
    {
        std::cerr << "Warning: LinController received SlaveConfiguration for unkonwn LIN Slave {" << from.participant << ", " << from.endpoint << "}\n";
        return;
    }

    auto&& linSlave = GetLinSlave(from);
    if (msg.linId >= linSlave.responses.size())
    {
        std::cerr << "Warning: LinController received SlaveResponse configuration from {" << from.participant << ", " << from.endpoint << "} for unconfigured LIN ID" << msg.linId << "\n";
        return;
    }

    linSlave.responses[msg.linId].payload = msg.payload;
}


void LinController::SetEndpointAddress(const ::ib::mw::EndpointAddress& endpointAddress)
{
    _endpointAddr = endpointAddress;
}

auto LinController::EndpointAddress() const -> const ::ib::mw::EndpointAddress&
{
    return _endpointAddr;
}

template<typename MsgT>
void LinController::CallHandlers(const MsgT& msg)
{
    auto&& handlers = std::get<CallbackVector<MsgT>>(_callbacks);
    for (auto&& handler : handlers)
    {
        handler(this, msg);
    }
}

template <typename MsgT>
void LinController::SendIbMessage(MsgT&& msg)
{
    _comAdapter->SendIbMessage(_endpointAddr, std::forward<MsgT>(msg));
}


} // namespace lin
} // namespace sim
} // namespace ib
