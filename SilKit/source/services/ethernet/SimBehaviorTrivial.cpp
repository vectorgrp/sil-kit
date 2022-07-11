// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "EthController.hpp"
#include "SimBehaviorTrivial.hpp"

namespace {
auto GetSourceMac(const SilKit::Services::Ethernet::EthernetFrame& frame) -> SilKit::Services::Ethernet::EthernetMac
{
    SilKit::Services::Ethernet::EthernetMac source{};
    std::copy(frame.raw.begin() + sizeof(SilKit::Services::Ethernet::EthernetMac),
              frame.raw.begin() + 2 * sizeof(SilKit::Services::Ethernet::EthernetMac), source.begin());

    return source;
}
} // namespace

namespace SilKit {
namespace Services {
namespace Ethernet {

SimBehaviorTrivial::SimBehaviorTrivial(Core::IParticipantInternal* participant, EthController* ethController,
                    Services::Orchestration::ITimeProvider* timeProvider)
    : _participant{participant}
    , _parentController{ethController}
    , _parentServiceEndpoint{dynamic_cast<Core::IServiceEndpoint*>(ethController)}
    , _timeProvider{timeProvider}
{
    (void)_parentController;
}

template <typename MsgT>
void SimBehaviorTrivial::ReceiveMsg(const MsgT& msg)
{
    auto receivingController = dynamic_cast<Core::IMessageReceiver<MsgT>*>(_parentController);
    assert(receivingController);
    receivingController->ReceiveMsg(_parentServiceEndpoint, msg);
}

auto SimBehaviorTrivial::AllowReception(const Core::IServiceEndpoint* /*from*/) const -> bool 
{ 
    return true; 
}

void SimBehaviorTrivial::SendMsg(EthernetFrameEvent&& ethFrameEvent)
{
    EthernetState controllerState = _parentController->GetState();

    if (controllerState == EthernetState::LinkUp)
    {
        // Trivial Sim: Set the timestamp, trace, send out the event and directly generate the ack
        ethFrameEvent.timestamp = _timeProvider->Now();
        _tracer.Trace(SilKit::Services::TransmitDirection::TX, ethFrameEvent.timestamp, ethFrameEvent.frame);
        _participant->SendMsg(_parentServiceEndpoint, ethFrameEvent);

        EthernetFrameTransmitEvent ack;
        ack.timestamp = ethFrameEvent.timestamp;
        ack.transmitId = ethFrameEvent.transmitId;
        ack.sourceMac = GetSourceMac(ethFrameEvent.frame);
        ack.status = EthernetTransmitStatus::Transmitted;
        ReceiveMsg(ack);
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
        ReceiveMsg(ack);
    }

    
}

void SimBehaviorTrivial::SendMsg(EthernetSetMode&& ethSetMode)
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
    ReceiveMsg(statusReply);
}

void SimBehaviorTrivial::OnReceiveAck(const EthernetFrameTransmitEvent& /*msg*/)
{
}

} // namespace Ethernet
} // namespace Services
} // namespace SilKit
