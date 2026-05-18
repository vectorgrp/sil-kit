// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/services/logging/ILogger.hpp"

#include "IServiceDiscovery.hpp"
#include "ServiceDatatypes.hpp"
#include "I2cController.hpp"

namespace SilKit {
namespace Services {
namespace I2c {

I2cController::I2cController(Core::IParticipantInternal* participant, SilKit::Config::I2cController config,
                              Services::Orchestration::ITimeProvider* timeProvider)
    : _participant(participant)
    , _config{std::move(config)}
    , _simulationBehavior{participant, this, timeProvider}
    , _logger{participant->GetLogger()}
{
}

//------------------------
// Trivial or detailed
//------------------------

void I2cController::RegisterServiceDiscovery()
{
    Core::Discovery::IServiceDiscovery* disc = _participant->GetServiceDiscovery();
    disc->RegisterServiceDiscoveryHandler([this](Core::Discovery::ServiceDiscoveryEvent::Type discoveryType,
                                                 const Core::ServiceDescriptor& remoteServiceDescriptor) {
        if (_simulationBehavior.IsTrivial())
        {
            if (discoveryType == Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated
                && IsRelevantNetwork(remoteServiceDescriptor))
            {
                Logging::Info(_logger,
                              "Controller '{}' is using the simulated network '{}' and will route all messages to "
                              "the network simulator '{}'",
                              _config.name, remoteServiceDescriptor.GetNetworkName(),
                              remoteServiceDescriptor.GetParticipantName());
                SetDetailedBehavior(remoteServiceDescriptor);
            }
        }
        else
        {
            if (discoveryType == Core::Discovery::ServiceDiscoveryEvent::Type::ServiceRemoved
                && IsRelevantNetwork(remoteServiceDescriptor))
            {
                Logging::Warn(_logger,
                              "The network simulator for controller '{}' left the simulation. The controller is no "
                              "longer simulated.",
                              _config.name);
                SetTrivialBehavior();
            }
        }
    });
}

void I2cController::SetDetailedBehavior(const Core::ServiceDescriptor& remoteServiceDescriptor)
{
    _simulationBehavior.SetDetailedBehavior(remoteServiceDescriptor);
}

void I2cController::SetTrivialBehavior()
{
    _simulationBehavior.SetTrivialBehavior();
}

auto I2cController::IsRelevantNetwork(const Core::ServiceDescriptor& remoteServiceDescriptor) const -> bool
{
    return remoteServiceDescriptor.GetServiceType() == SilKit::Core::ServiceType::Link
           && remoteServiceDescriptor.GetNetworkName() == _serviceDescriptor.GetNetworkName();
}

auto I2cController::AllowReception(const IServiceEndpoint* from) const -> bool
{
    return _simulationBehavior.AllowReception(from);
}

template <typename MsgT>
void I2cController::SendMsg(MsgT&& msg)
{
    _simulationBehavior.SendMsg(std::move(msg));
}

auto I2cController::GetControllerMode() const -> I2cControllerMode
{
    return _controllerConfig.mode;
}

//------------------------
// Public API
//------------------------

void I2cController::Init(I2cControllerConfig config)
{
    _controllerConfig = config;

    WireI2cControllerConfig wireConfig{};
    wireConfig.mode = config.mode;
    wireConfig.slaveAddress = config.slaveAddress;
    wireConfig.slaveAddressMode = config.slaveAddressMode;
    wireConfig.speedMode = config.speedMode;

    SendMsg(std::move(wireConfig));
}

void I2cController::SendFrame(const I2cFrame& frame, void* userContext)
{
    WireI2cFrameEvent event{};
    event.frame = MakeWireI2cFrame(frame);
    event.userContext = userContext;

    SendMsg(std::move(event));
}

void I2cController::SetReadResponse(Util::Span<const uint8_t> data)
{
    _readResponseData = Util::SharedVector<uint8_t>(data);
}

//------------------------
// ReceiveMsg
//------------------------

void I2cController::ReceiveMsg(const IServiceEndpoint* from, const WireI2cFrameEvent& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    auto event = ToI2cFrameEvent(msg);

    // Preserve userContext only for TX (own send) — zero it for frames from others
    const auto frameDirection = static_cast<DirectionMask>(msg.direction);
    constexpr auto txDirection = static_cast<DirectionMask>(TransmitDirection::TX);
    if ((frameDirection & txDirection) != txDirection)
    {
        event.userContext = nullptr;
    }

    _tracer.Trace(msg.direction, msg.timestamp, event);

    CallHandlers(event);

    // In trivial mode: if we're a slave and this is a Read request addressed to us, broadcast our response
    if (_simulationBehavior.IsTrivial() && _controllerConfig.mode == I2cControllerMode::Slave
        && event.direction == TransmitDirection::RX
        && event.frame.direction == I2cTransferDirection::Read
        && (event.frame.address == _controllerConfig.slaveAddress))
    {
        WireI2cFrameEvent response{};
        response.timestamp = event.timestamp;
        response.frame.address = _controllerConfig.slaveAddress;
        response.frame.addressMode = _controllerConfig.slaveAddressMode;
        response.frame.direction = I2cTransferDirection::Read;
        response.frame.data = _readResponseData;
        response.direction = TransmitDirection::TX;
        response.userContext = msg.userContext;
        SendMsg(std::move(response));
    }
}

void I2cController::ReceiveMsg(const IServiceEndpoint* from, const I2cAcknowledge& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    I2cFrameTransmitEvent event{};
    event.timestamp = msg.timestamp;
    event.address = msg.address;
    event.status = msg.status;
    event.userContext = msg.userContext;

    CallHandlers(event);
}

void I2cController::ReceiveMsg(const IServiceEndpoint* from, const WireI2cControllerConfig& msg)
{
    if (!AllowReception(from))
    {
        return;
    }

    _simulationBehavior.OnControllerConfig(from, msg);
}

//------------------------
// Handlers
//------------------------

auto I2cController::AddFrameHandler(FrameHandler handler, DirectionMask directionMask) -> HandlerId
{
    auto filter = FilterT<I2cFrameEvent>{[directionMask](const I2cFrameEvent& event) {
        return ((static_cast<DirectionMask>(event.direction) & directionMask) != 0);
    }};
    return AddHandler(std::move(handler), std::move(filter));
}

void I2cController::RemoveFrameHandler(HandlerId handlerId)
{
    if (!RemoveHandler<I2cFrameEvent>(handlerId))
    {
        _participant->GetLogger()->Warn("RemoveFrameHandler failed: Unknown HandlerId.");
    }
}

auto I2cController::AddFrameTransmitHandler(FrameTransmitHandler handler) -> HandlerId
{
    return AddHandler(std::move(handler));
}

void I2cController::RemoveFrameTransmitHandler(HandlerId handlerId)
{
    if (!RemoveHandler<I2cFrameTransmitEvent>(handlerId))
    {
        _participant->GetLogger()->Warn("RemoveFrameTransmitHandler failed: Unknown HandlerId.");
    }
}

template <typename MsgT>
HandlerId I2cController::AddHandler(CallbackT<MsgT> handler, FilterT<MsgT> filter)
{
    auto& callbacks = std::get<FilteredCallbacks<MsgT>>(_callbacks);
    return callbacks.Add(std::move(handler), std::move(filter));
}

template <typename MsgT>
bool I2cController::RemoveHandler(HandlerId handlerId)
{
    auto& callbacks = std::get<FilteredCallbacks<MsgT>>(_callbacks);
    return callbacks.Remove(handlerId);
}

template <typename MsgT>
void I2cController::CallHandlers(const MsgT& msg)
{
    auto& callbacks = std::get<FilteredCallbacks<MsgT>>(_callbacks);
    callbacks.InvokeAll(this, msg);
}

} // namespace I2c
} // namespace Services
} // namespace SilKit
