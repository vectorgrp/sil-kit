// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "LinController.hpp"

#include <cassert>
#include <algorithm>
#include <iostream>

#include "ib/mw/IComAdapter.hpp"
#include "ib/mw/logging/spdlog.hpp"

namespace ib {
namespace sim {
namespace lin {

namespace {
    constexpr LinId   GotosleepId{0x3c};
    constexpr Payload GotosleepPayload{8, {0x0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};
}

LinController::LinController(mw::IComAdapter* comAdapter)
    : _comAdapter{comAdapter}
    , _logger{comAdapter->GetLogger()}
{
}

void LinController::SetMasterMode()
{
    if (_controllerMode != ControllerMode::Inactive)
    {
        std::string errorMsg{"LinController::SetMasterMode() must only be called on unconfigured controllers!"};
        _logger->error(errorMsg);
        throw std::runtime_error{errorMsg};
    }
    _configuredControllerMode = ControllerMode::Master;
    _controllerMode = ControllerMode::Master;
}

void LinController::SetSlaveMode()
{
    if (_controllerMode != ControllerMode::Inactive)
    {
        std::string errorMsg{"LinController::SetSlaveMode() must only be called on unconfigured controllers!"};
        _logger->error(errorMsg);
        throw std::runtime_error{errorMsg};
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
        std::string errorMsg{"LinController:SetSleepMode() must not be called before SetMasterMode() or SetSlaveMode()"};
        _logger->error(errorMsg);
        throw std::runtime_error{errorMsg};
    }

    _controllerMode = ControllerMode::Sleep;

    ControllerConfig config{};
    config.controllerMode = _controllerMode;

    SendIbMessage(config);
}

void LinController::SetOperationalMode()
{
    if (_controllerMode != ControllerMode::Sleep)
    {
        std::string errorMsg{"LinController:SetOperationalMode() must only be called when controller is in sleep mode"};
        _logger->error(errorMsg);
        throw std::runtime_error{errorMsg};
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
        std::string errorMsg{"LinController::SetResponse() should only be called in SlaveMode"};
        _logger->error(errorMsg);
        throw std::runtime_error{errorMsg};
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
        std::string errorMsg{"LinController::SetResponseWithChecksum() should only be called in SlaveMode"};
        _logger->error(errorMsg);
        throw std::runtime_error{errorMsg};
    }
    if (checksumModel == ChecksumModel::Undefined)
    {
        std::string warnMsg("LinController::SetResponseWithChecksum() was called with ChecksumModel::Undefined, which does NOT alter the checksum model");
        _logger->warn(warnMsg);
    }

    SlaveResponse response;
    response.linId = linId;
    response.payload = payload;
    response.checksumModel = checksumModel;

    SendIbMessage(response);
}

void LinController::RemoveResponse(LinId linId)
{
    SlaveConfiguration slaveConfig;

    SlaveResponseConfig responseConfig;
    responseConfig.linId = linId;
    responseConfig.responseMode = ResponseMode::Unused;
    responseConfig.checksumModel = ChecksumModel::Undefined;
    responseConfig.payloadLength = 0;


    slaveConfig.responseConfigs.emplace_back(std::move(responseConfig));
    SendIbMessage(slaveConfig);
}

void LinController::SendWakeupRequest()
{
    if (_controllerMode != ControllerMode::Sleep)
    {
        std::string errorMsg{"LinController::SendWakeupRequest() must only be called in sleep mode!"};
        _logger->error(errorMsg);
        throw std::logic_error{errorMsg};
    }

    SendIbMessage(WakeupRequest{});
}

void LinController::SendMessage(const LinMessage& msg)
{
    if (_controllerMode != ControllerMode::Master)
    {
        std::string errorMsg{"LinController::SendMessage() must only be called in master mode!"};
        _logger->error(errorMsg);
        throw std::logic_error{errorMsg};
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
        std::string errorMsg{"LinController::RequestMessage() must only be called in master mode!"};
        _logger->error(errorMsg);
        throw std::logic_error{errorMsg};
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
        std::string errorMsg{"LinController::SendGoToSleep() must only be called in master mode!"};
        _logger->error(errorMsg);
        throw std::logic_error{errorMsg};
    }


    LinMessage gotosleep;

    gotosleep.status = MessageStatus::TxSuccess;
    gotosleep.checksumModel = ChecksumModel::Classic;
    gotosleep.linId = GotosleepId;
    gotosleep.payload = GotosleepPayload;

    SendMessage(gotosleep);
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
        _logger->warn(
            "LinController received LinMessage with payload length {} from {{{}, {}}}",
            static_cast<unsigned int>(msg.payload.size),
            from.participant,
            from.endpoint);
        return;
    }


    switch (_controllerMode)
    {
    case ControllerMode::Inactive:
        return;

    case ControllerMode::Master:
        _logger->warn("LinController in MasterMode received a LinMessage, probably originating from another master. This indicates an erroneous setup!");
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
                _logger->warn("LinController received diagnostic message with unsupported payload");
            }
        }
        return;

    case ControllerMode::Sleep:
        _logger->warn("LinController received LIN Message with id={} while controller is in sleep mode. Message is ignored.", static_cast<unsigned int>(msg.linId));
        return;

    default:
        _logger->warn("Unhandled ControllerMode in LinController::ReceiveIbMessage(..., LinMessage)");
        return;
    }
}

void LinController::ReceiveIbMessage(ib::mw::EndpointAddress /*from*/, const WakeupRequest& /*msg*/)
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
    if (_configuredControllerMode != ControllerMode::Master)
        return;

    if (msg.controllerMode == ControllerMode::Master)
    {
        _logger->warn("LinController received ControllerConfig with master mode, which will be ignored");
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
    if (_configuredControllerMode != ControllerMode::Master)
        return;

    if (!IsKnownSlave(from))
    {
        _logger->warn("LinController received SlaveConfiguration for unkonwn LIN Slave {{{}, {}}}", from.participant, from.endpoint);
        return;
    }

    auto&& linSlave = GetLinSlave(from);
    for (auto&& responseConfig : msg.responseConfigs)
    {
        auto linId = responseConfig.linId;
        if (linId >= linSlave.responses.size())
        {
            linSlave.responses.resize(linId + 1);
        }

         if (responseConfig.payloadLength > 8)
         {
             _logger->warn(
                 "LinController received SlaveResponseConfig with payload length {} from {{{}, {}}}",
                 static_cast<unsigned int>(responseConfig.payloadLength),
                 from.participant,
                 from.endpoint);
             continue;
         }

         static_cast<SlaveResponseConfig&>(linSlave.responses[linId]) = responseConfig;
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
        _logger->warn("LinController received SlaveConfiguration for unkonwn LIN Slave {{{}, {}}}", from.participant, from.endpoint);
        return;
    }

    auto&& linSlave = GetLinSlave(from);
    if (msg.linId >= linSlave.responses.size())
    {
        _logger->warn(
            "LinController received SlaveResponse configuration from {{{}, {}}} for unconfigured LIN ID {}",
            from.participant,
            from.endpoint,
            msg.linId);
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
