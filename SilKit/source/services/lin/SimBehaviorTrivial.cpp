// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LinController.hpp"
#include "SimBehaviorTrivial.hpp"
#include "Assert.hpp"

namespace SilKit {
namespace Services {
namespace Lin {

namespace {
inline auto ToFrameResponseMode(LinFrameResponseType responseType) -> LinFrameResponseMode
{
    switch (responseType)
    {
    case LinFrameResponseType::MasterResponse: return LinFrameResponseMode::TxUnconditional;
    case LinFrameResponseType::SlaveResponse: return LinFrameResponseMode::Rx;
    case LinFrameResponseType::SlaveToSlave: return LinFrameResponseMode::Unused;
    }
    return LinFrameResponseMode::Unused;
}

inline auto ToTxFrameStatus(LinFrameStatus status) -> LinFrameStatus
{
    switch (status)
    {
    case LinFrameStatus::LIN_RX_BUSY: return LinFrameStatus::LIN_TX_BUSY;
    case LinFrameStatus::LIN_RX_ERROR: return LinFrameStatus::LIN_TX_ERROR;
    case LinFrameStatus::LIN_RX_NO_RESPONSE: return LinFrameStatus::LIN_RX_NO_RESPONSE;
    case LinFrameStatus::LIN_RX_OK: return LinFrameStatus::LIN_TX_OK;
    default: return status;
    }
}

inline auto ToTracingDir(LinFrameStatus status) -> SilKit::Services::TransmitDirection
{
    switch (status)
    {
    case LinFrameStatus::LIN_RX_ERROR: //[[fallthrough]]
    case LinFrameStatus::LIN_RX_BUSY: //[[fallthrough]]
    case LinFrameStatus::LIN_RX_NO_RESPONSE: //[[fallthrough]]
    case LinFrameStatus::LIN_RX_OK: return SilKit::Services::TransmitDirection::RX;
    case LinFrameStatus::LIN_TX_ERROR: //[[fallthrough]]
    case LinFrameStatus::LIN_TX_BUSY: //[[fallthrough]]
    case LinFrameStatus::LIN_TX_HEADER_ERROR: //[[fallthrough]]
    case LinFrameStatus::LIN_TX_OK: return SilKit::Services::TransmitDirection::TX;
    default:
        //if invalid status given, failsafe to send.
        return SilKit::Services::TransmitDirection::TX;
    }
}

} // namespace

SimBehaviorTrivial::SimBehaviorTrivial(Core::IParticipantInternal* participant, LinController* linController,
                                       Services::Orchestration::ITimeProvider* timeProvider)
    : _participant{participant}
    , _parentController{linController}
    , _parentServiceEndpoint{dynamic_cast<Core::IServiceEndpoint*>(linController)}
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

template <typename MsgT>
void SimBehaviorTrivial::SendMsgImpl(MsgT&& msg)
{
    _participant->SendMsg(_parentServiceEndpoint, std::forward<MsgT>(msg));
}

void SimBehaviorTrivial::SendMsg(LinSendFrameRequest&& msg)
{
    _parentController->SetFrameResponse(msg.frame, ToFrameResponseMode(msg.responseType));
    _parentController->SendFrameHeader(msg.frame.id);
}
void SimBehaviorTrivial::SendMsg(LinTransmission&& msg)
{
    SendMsgImpl(msg);
}
void SimBehaviorTrivial::SendMsg(LinControllerConfig&& msg)
{
    SendMsgImpl(msg);
}
void SimBehaviorTrivial::SendMsg(LinSendFrameHeaderRequest&& msg)
{
    // We answer the call immediately based on the cached responses
    LinTransmission transmission;
    auto numResponses = 0;
    std::tie(numResponses, transmission.frame) = _parentController->GetResponse(msg.id);
    transmission.frame.id = msg.id;
    transmission.timestamp = _timeProvider->Now();

    // Check for status change due to numResponses
    if (numResponses == 0)
    {
        transmission.status = LinFrameStatus::LIN_RX_NO_RESPONSE;
    }
    else if (numResponses == 1)
    {
        transmission.status = LinFrameStatus::LIN_RX_OK;
    }
    else if (numResponses > 1)
    {
        transmission.status = LinFrameStatus::LIN_RX_ERROR;
    }

    // Dispatch the LIN transmission to all connected nodes
    SendMsgImpl(transmission);

    // Dispatch the LIN transmission to our own callbacks
    LinFrameResponseMode masterResponseMode =
        _parentController->GetThisLinNode().responses[msg.id].responseMode;
    LinFrameStatus masterFrameStatus = transmission.status;
    if (masterResponseMode == LinFrameResponseMode::TxUnconditional)
    {
        masterFrameStatus = ToTxFrameStatus(masterFrameStatus);
    }

    _tracer.Trace(ToTracingDir(masterFrameStatus), transmission.timestamp, transmission.frame);

    // Dispatch the reply locally
    _parentController->CallLinFrameStatusEventHandler(LinFrameStatusEvent{transmission.timestamp, transmission.frame, masterFrameStatus});

}

void SimBehaviorTrivial::SendMsg(LinFrameResponseUpdate&& msg)
{
    SendMsgImpl(msg);
}

void SimBehaviorTrivial::SendMsg(LinControllerStatusUpdate&& msg)
{
    SendMsgImpl(msg);
}

auto SimBehaviorTrivial::CalcFrameStatus(const LinTransmission& linTransmission, bool isGoToSleepFrame)
    -> LinFrameStatus
{
    if (isGoToSleepFrame)
    {
        return LinFrameStatus::LIN_RX_OK;
    }

    // Evaluate locally known response
    auto& thisLinNode = _parentController->GetThisLinNode();
    const auto response = thisLinNode.responses[linTransmission.frame.id];
    switch (response.responseMode)
    {
        case LinFrameResponseMode::Unused:
        {
            return LinFrameStatus::LIN_RX_NO_RESPONSE;
        }
        case LinFrameResponseMode::Rx:
        {
            //Veriy Checksum and DataLength
            if (response.frame.dataLength != linTransmission.frame.dataLength
                || response.frame.checksumModel != linTransmission.frame.checksumModel)
            {
                return LinFrameStatus::LIN_RX_ERROR;
            }
            break;
        }
        case LinFrameResponseMode::TxUnconditional:
        {
            // Transmissions are always sent with LinFrameStatus::RX_xxx so we have to
            // convert the status to a TX_xxx if we sent this frame.
            return ToTxFrameStatus(linTransmission.status);
        }
    }
    return linTransmission.status;
}

void SimBehaviorTrivial::GoToSleep()
{
    LinTransmission gotosleepTx;
    gotosleepTx.frame = GoToSleepFrame();
    gotosleepTx.status = LinFrameStatus::LIN_RX_OK;
    gotosleepTx.timestamp = _timeProvider->Now();

    SendMsgImpl(gotosleepTx);

    // For trivial simulations we go directly to Sleep state.
    _parentController->GoToSleepInternal();
}

void SimBehaviorTrivial::Wakeup()
{
    // Send to others with direction=RX
    LinWakeupPulse pulse{_timeProvider->Now(), TransmitDirection::RX};
    SendMsgImpl(pulse);

    // No self delivery: directly call handlers with direction=TX
    ReceiveMsg(LinWakeupPulse{pulse.timestamp, TransmitDirection::TX});
    _parentController->WakeupInternal();
}


} // namespace Lin
} // namespace Services
} // namespace SilKit
