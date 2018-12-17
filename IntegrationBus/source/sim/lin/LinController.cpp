// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "LinController.hpp"

#include <cassert>
#include <algorithm>
#include <iostream>

#include "ib/mw/IComAdapter.hpp"

namespace ib {
namespace sim {
namespace lin {

namespace {
    constexpr LinId   GotosleepId{0x3c};
    constexpr Payload GotosleepPayload{8, {0x0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};
}

LinController::LinController(::ib::mw::IComAdapter* comAdapter)
: _comAdapter(comAdapter)
{
}

void LinController::SetMasterMode()
{
    if (_controllerMode != ControllerMode::Inactive)
    {
        throw std::runtime_error{"LinController::SetMasterMode() must only be called on unconfigured controllers!"};
    }
    _configuredControllerMode = ControllerMode::Master;
    _controllerMode = ControllerMode::Master;
}

void LinController::SetSlaveMode()
{
    if (_controllerMode != ControllerMode::Inactive)
    {
        throw std::runtime_error{"LinController::SetSlaveMode() must only be called on unconfigured controllers!"};
    }

    // set slave mode
    _configuredControllerMode = ControllerMode::Slave;
    _controllerMode = ControllerMode::Slave;

    // Announce this controller at LIN masters
    ControllerConfig config{};
    config.controllerMode = _controllerMode;

    SendIbMessage(config);
}

void LinController::SetBaudRate(uint32_t /*rate*/)
{
    // Baudrate is only considered when using a LIN Network Simulator, i.e., in the LinControllerProxy
}

void LinController::SetSleepMode()
{
    if (_configuredControllerMode == ControllerMode::Inactive)
    {
        throw std::runtime_error{"LinController:SetSleepMode() must not be called before SetMasterMode() or SetSlaveMode()"};
    }

    _controllerMode = ControllerMode::Sleep;

    ControllerConfig config{};
    config.controllerMode = _controllerMode;

    SendIbMessage(config);
}

void LinController::SetOperational()
{
    if (_controllerMode != ControllerMode::Sleep)
    {
        throw std::runtime_error{"LinController:SetOperational() must only be called when controller is in sleep mode"};
    }

    // restore configured controller mode
    _controllerMode = _configuredControllerMode;

    ControllerConfig config{};
    config.controllerMode = _controllerMode;

    SendIbMessage(config);
}

void LinController::SetSlaveConfiguration(const SlaveConfiguration& config)
{
    SendIbMessage(config);
}

void LinController::SetResponse(LinId linId, const Payload& payload)
{
    if (_controllerMode != ControllerMode::Slave)
    {
        throw std::runtime_error{"LinController::SetResponse() should only be called in SlaveMode"};
    }

    SlaveResponse response;
    response.linId = linId;
    response.payload = payload;
    response.checksumModel = ChecksumModel::Undefined;

    SendIbMessage(response);
}

void LinController::SetResponseWithChecksum(LinId linId, const Payload& payload, ChecksumModel checksumModel)
{
    if (_controllerMode != ControllerMode::Slave)
    {
        throw std::runtime_error{"LinController::SetResponseWithChecksum() should only be called in SlaveMode"};
    }
    if (checksumModel == ChecksumModel::Undefined)
    {
        std::cerr << "WARNING: LinController::SetResponseWithChecksum() was called with ChecksumModel::Undefined, which does NOT alter the checksum model!\n";
    }

    SlaveResponse response;
    response.linId = linId;
    response.payload = payload;
    response.checksumModel = checksumModel;

    SendIbMessage(response);
}

void LinController::RemoveResponse(LinId linId)
{
    // Fixme: Implement Remove Response
    std::cerr << "LinController::RemoveResponse() is not implemented\n";
}

void LinController::SendWakeupRequest()
{
    if (_controllerMode != ControllerMode::Sleep)
    {
        std::cerr << "ERROR: LinController::SendWakeupRequest() must only be called in sleep mode!" << std::endl;
        throw std::logic_error("LinController::SendWakeupRequest() must only be called in sleep mode!");
    }

    SendIbMessage(WakeupRequest{});
}

void LinController::SendMessage(const LinMessage& msg)
{
    if (_controllerMode != ControllerMode::Master)
    {
        std::cerr << "ERROR: LinController::SendMessage() must only be called in master mode!" << std::endl;
        throw std::logic_error("LinController::SendMessage() must only be called in master mode!");
    }

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
        std::cerr << "ERROR: LinController::RequestMessage() must only be called in master mode!" << std::endl;
        throw std::logic_error("LinController::RequestMessage() must only be called in master mode!");
    }

    // we answer the call immediately based on the cached responses
    // setup a reply
    LinMessage reply{};
    reply.linId = msg.linId;

    auto numResponses = 0;
    for (auto&& keyValue: _linSlaves)
    {
        auto&& slave = keyValue.second;

        if (slave.config.controllerMode != ControllerMode::Slave)
            continue; // only operational slaves are considered.

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

void LinController::SendGoToSleep()
{
    if (_controllerMode != ControllerMode::Master)
    {
        std::cerr << "ERROR: LinController::SendGoToSleep() must only be called in master mode!" << std::endl;
        throw std::logic_error("LinController::SendGoToSleep() must only be called in master mode!");
    }


    LinMessage gotosleep;

    gotosleep.status = MessageStatus::TxSuccess;
    gotosleep.checksumModel = ChecksumModel::Classic;
    gotosleep.linId = GotosleepId;
    gotosleep.payload = GotosleepPayload;

    SendIbMessage(gotosleep);
}

void LinController::RegisterTxCompleteHandler(TxCompleteHandler handler)
{
    RegisterHandler(std::move(handler));
}

void LinController::RegisterReceiveMessageHandler(ReceiveMessageHandler handler)
{
    RegisterHandler(std::move(handler));
}

template<typename MsgT>
void LinController::RegisterHandler(CallbackT<MsgT>&& handler)
{
    auto&& handlers = std::get<CallbackVector<MsgT>>(_callbacks);
    handlers.emplace_back(std::move(handler));
}

void LinController::RegisterWakeupRequestHandler(WakeupRequestHandler handler)
{
    _wakeuprequestHandlers.emplace_back(std::move(handler));
}

void LinController::RegisterSleepCommandHandler(SleepCommandHandler handler)
{
    _gotosleepHandlers.emplace_back(std::move(handler));
}

void LinController::ReceiveIbMessage(ib::mw::EndpointAddress from, const LinMessage& msg)
{
    if (from == _endpointAddr)
        return;

    if (msg.payload.size > 8)
    {
        std::cerr << "Warning: LinController received LinMessage with payload length " << static_cast<unsigned int>(msg.payload.size)
                  << " from {" << from.participant << "," << from.endpoint << "}\n";
        return;
    }


    switch (_controllerMode)
    {
    case ControllerMode::Inactive:
        return;

    case ControllerMode::Master:
        std::cerr << "WARNING: LinController in MasterMode received a LinMessage, probably originating from another master. This indicates an erroneous setup!\n";
        //[[fallthrough]]

    case ControllerMode::Slave:
        CallHandlers(msg);
        if (msg.linId == GotosleepId)
        {
            if (msg.payload == GotosleepPayload)
            {
                for (auto&& handler : _gotosleepHandlers)
                {
                    handler(this);
                }
            }
            else
            {
                std::cerr << "WARNING: unsuported diagnostic message with payload with\n";
            }
        }
        return;

    default:
        std::cerr << "WARNING: Unhandled ControllerMode in LinController::ReceiveIbMessage(..., LinMessage)\n";
        return;
    }
}

void LinController::ReceiveIbMessage(ib::mw::EndpointAddress from, const WakeupRequest& msg)
{
    for (auto&& handler : _wakeuprequestHandlers)
    {
        handler(this);
    }
}

void LinController::ReceiveIbMessage(mw::EndpointAddress from, const ControllerConfig& msg)
{
    if (from == _endpointAddr)
        return;

    // only controllers in master mode are responsible for managing responses.
    if (_controllerMode != ControllerMode::Master)
        return;

    if (msg.controllerMode == ControllerMode::Master)
    {
        std::cerr << "WARNING: LinController received ControllerConfig with master mode, which will be ignored\n";
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
        auto&& responseConfig = msg.responseConfigs[i];
        if (responseConfig.payloadLength > 8)
        {
            std::cerr << "Warning: LinController received SlaveResponseConfig with payload length " << static_cast<unsigned int>(responseConfig.payloadLength)
                      << " from {" << from.participant << "," << from.endpoint << "}\n";
            continue;
        }

        static_cast<SlaveResponseConfig&>(linSlave.responses[i]) = responseConfig;
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

    // Update ChecksumModel if defined.
    if (msg.checksumModel != ChecksumModel::Undefined)
    {
        linSlave.responses[msg.linId].checksumModel = msg.checksumModel;
    }
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
