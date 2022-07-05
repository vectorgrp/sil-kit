// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthController.hpp"
#include "SimBehaviorTrivial.hpp"

namespace {
auto GetSourceMac(const ib::sim::eth::EthernetFrame& frame) -> ib::sim::eth::EthernetMac
{
    ib::sim::eth::EthernetMac source{};
    std::copy(frame.raw.begin() + sizeof(ib::sim::eth::EthernetMac),
              frame.raw.begin() + 2 * sizeof(ib::sim::eth::EthernetMac), source.begin());

    return source;
}
} // namespace

namespace ib {
namespace sim {
namespace eth {

SimBehaviorTrivial::SimBehaviorTrivial(mw::IParticipantInternal* participant, EthController* ethController,
                    mw::sync::ITimeProvider* timeProvider)
    : _participant{participant}
    , _parentController{ethController}
    , _parentServiceEndpoint{dynamic_cast<mw::IIbServiceEndpoint*>(ethController)}
    , _timeProvider{timeProvider}
{
    (void)_parentController;
}

template <typename MsgT>
void SimBehaviorTrivial::ReceiveIbMessage(const MsgT& msg)
{
    auto receivingController = dynamic_cast<mw::IIbMessageReceiver<MsgT>*>(_parentController);
    assert(receivingController);
    receivingController->ReceiveIbMessage(_parentServiceEndpoint, msg);
}

auto SimBehaviorTrivial::AllowReception(const mw::IIbServiceEndpoint* /*from*/) const -> bool 
{ 
    return true; 
}

void SimBehaviorTrivial::SendIbMessage(EthernetFrameEvent&& ethFrameEvent)
{
    EthernetState controllerState = _parentController->GetState();

    if (controllerState == EthernetState::LinkUp)
    {
        // Trivial Sim: Set the timestamp, trace, send out the event and directly generate the ack
        ethFrameEvent.timestamp = _timeProvider->Now();
        _tracer.Trace(ib::sim::TransmitDirection::TX, ethFrameEvent.timestamp, ethFrameEvent.frame);
        _participant->SendIbMessage(_parentServiceEndpoint, ethFrameEvent);

        EthernetFrameTransmitEvent ack;
        ack.timestamp = ethFrameEvent.timestamp;
        ack.transmitId = ethFrameEvent.transmitId;
        ack.sourceMac = GetSourceMac(ethFrameEvent.frame);
        ack.status = EthernetTransmitStatus::Transmitted;
        ReceiveIbMessage(ack);
    }
    else
    {
        EthernetFrameTransmitEvent ack;
        ack.timestamp = _timeProvider->Now();
        ack.transmitId = ethFrameEvent.transmitId;
        ack.sourceMac = GetSourceMac(ethFrameEvent.frame);
        if (controllerState == EthernetState::Inactive)
        {
            ack.status = EthernetTransmitStatus::ControllerInactive;
        }
        else if(controllerState == EthernetState::LinkDown)
        {
            ack.status = EthernetTransmitStatus::LinkDown;
        }
        ReceiveIbMessage(ack);
    }

    
}

void SimBehaviorTrivial::SendIbMessage(EthernetSetMode&& ethSetMode)
{
    // Trivial: Reply EthernetSetMode locally with an EthernetStatus
    EthernetStatus statusReply{};
    statusReply.timestamp = _timeProvider->Now();

    if (ethSetMode.mode == EthernetMode::Active)
    {
        statusReply.state = EthernetState::LinkUp;
    }
    else if (ethSetMode.mode == EthernetMode::Inactive)
    {
        statusReply.state = EthernetState::Inactive;
    }
    ReceiveIbMessage(statusReply);
}

void SimBehaviorTrivial::OnReceiveAck(const EthernetFrameTransmitEvent& /*msg*/)
{
}

} // namespace eth
} // namespace sim
} // namespace ib
