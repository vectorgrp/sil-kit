// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "I2cController.hpp"
#include "SimBehaviorTrivial.hpp"
#include "Assert.hpp"

#include "silkit/services/logging/ILogger.hpp"

namespace SilKit {
namespace Services {
namespace I2c {

SimBehaviorTrivial::SimBehaviorTrivial(Core::IParticipantInternal* participant, I2cController* controller,
                                       Services::Orchestration::ITimeProvider* timeProvider)
    : _participant{participant}
    , _parentController{controller}
    , _parentServiceEndpoint{dynamic_cast<Core::IServiceEndpoint*>(controller)}
    , _timeProvider{timeProvider}
{
}

template <typename MsgT>
void SimBehaviorTrivial::ReceiveMsg(const MsgT& msg)
{
    auto receivingController = dynamic_cast<Core::IMessageReceiver<MsgT>*>(_parentController);
    SILKIT_ASSERT(receivingController);
    receivingController->ReceiveMsg(_parentServiceEndpoint, msg);
}

auto SimBehaviorTrivial::AllowReception(const Core::IServiceEndpoint* /*from*/) const -> bool
{
    return true;
}

auto SimBehaviorTrivial::HasSlave(I2cAddress address) const -> bool
{
    return _slaveRegistry.count(address) > 0;
}

void SimBehaviorTrivial::SendMsg(WireI2cFrameEvent&& event)
{
    auto now = _timeProvider->Now();
    event.timestamp = now;

    // Broadcast to all other controllers on the network as RX
    WireI2cFrameEvent eventRx = event;
    eventRx.direction = TransmitDirection::RX;
    _participant->SendMsg(_parentServiceEndpoint, eventRx);

    // Self-deliver as TX (for tracing and TX-direction frame handlers)
    WireI2cFrameEvent eventTx = event;
    eventTx.direction = TransmitDirection::TX;
    ReceiveMsg(eventTx);

    // Generate acknowledgment only when the master initiates a transfer
    if (_parentController->GetControllerMode() == I2cControllerMode::Master)
    {
        I2cAcknowledge ack{};
        ack.timestamp = now;
        ack.address = event.frame.address;
        ack.userContext = event.userContext;

        const bool isGeneralCall = (event.frame.address == 0x00);
        if (isGeneralCall)
        {
            // General Call: per I2C spec slaves cannot NACK; Read to 0x00 is invalid
            if (event.frame.direction == I2cTransferDirection::Read)
            {
                ack.status = I2cTransmitStatus::BusError;
            }
            else
            {
                ack.status = I2cTransmitStatus::Transmitted;
            }
        }
        else if (HasSlave(event.frame.address))
        {
            ack.status = I2cTransmitStatus::Transmitted;
        }
        else
        {
            ack.status = I2cTransmitStatus::AddressNak;
        }

        ReceiveMsg(ack);
    }
}

void SimBehaviorTrivial::SendMsg(WireI2cControllerConfig&& config)
{
    // Broadcast config to all other controllers on the network
    _participant->SendMsg(_parentServiceEndpoint, config);
    // Also update own registry in case this controller is a slave
    OnControllerConfig(_parentServiceEndpoint, config);
}

void SimBehaviorTrivial::OnControllerConfig(const Core::IServiceEndpoint* /*from*/,
                                            const WireI2cControllerConfig& config)
{
    if (config.mode == I2cControllerMode::Slave)
    {
        _slaveRegistry[config.slaveAddress] = true;
    }
    else
    {
        // Master or Inactive: remove any previously registered slave entry for this controller
        // (We can't key by endpoint here, so we track the address that was registered)
        // For simplicity: remove exact address if it was previously a slave
        _slaveRegistry.erase(config.slaveAddress);
    }
}

} // namespace I2c
} // namespace Services
} // namespace SilKit
