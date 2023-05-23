/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "LinController.hpp"
#include "SimBehaviorTrivial.hpp"
#include "Assert.hpp"

namespace SilKit {
namespace Services {
namespace Lin {

namespace {
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
    _parentController->SendFrameHeader(msg.frame.id);
}

void SimBehaviorTrivial::SendMsg(LinTransmission&& msg)
{
    SendMsgImpl(msg);
}

void SimBehaviorTrivial::SendMsg(WireLinControllerConfig&& msg)
{
    // Only slaves need to distribute updates
    if (_parentController->Mode() == LinControllerMode::Slave)
    {
        SendMsgImpl(msg);
    }
}

void SimBehaviorTrivial::SendMsg(LinFrameResponseUpdate&& msg)
{
    // Only slaves need to distribute updates
    if (_parentController->Mode() == LinControllerMode::Slave)
    {
        SendMsgImpl(msg);
    }
}

void SimBehaviorTrivial::SendMsg(LinSendFrameHeaderRequest&& msg)
{
    LinFrame frame;
    auto numResponses = 0;
    std::tie(numResponses, frame) = _parentController->GetResponse(msg.id);

    if (numResponses == 1 || (numResponses == 0 && _parentController->HasDynamicNode()))
    {
        // Send the header, the Tx-Node will generate the LinTransmission (possibly the master itself)
        SendMsgImpl(msg);
    }
    else
    {
        // Error case: Send LinTransmission with error status
        SendErrorTransmissionOnHeaderRequest(numResponses, frame);
    }
}

void SimBehaviorTrivial::SendErrorTransmissionOnHeaderRequest(int numResponses, LinFrame frame)
{
    LinTransmission transmission{_timeProvider->Now(), frame, LinFrameStatus::NOT_OK};

    // Check for status change due to numResponses
    if (numResponses == 0)
    {
        transmission.status = LinFrameStatus::LIN_RX_NO_RESPONSE;
    }
    else if (numResponses > 1)
    {
        transmission.status = LinFrameStatus::LIN_RX_ERROR;
    }

    // Dispatch the LIN transmission to all connected nodes
    SendMsgImpl(transmission);

    // Dispatch the LIN transmission to our own callbacks
    LinFrameResponseMode masterResponseMode = _parentController->GetThisLinNode().responses[frame.id].responseMode;
    LinFrameStatus masterFrameStatus = transmission.status;
    if (masterResponseMode == LinFrameResponseMode::TxUnconditional)
    {
        // This can only happen if the master + another slave have TxUnconditional on this LinId
        masterFrameStatus = LinFrameStatus::LIN_TX_ERROR;
    }

    _parentController->GetTracer()->Trace(ToTracingDir(masterFrameStatus), transmission.timestamp, transmission.frame);

    // Evoke the callbacks locally
    _parentController->CallLinFrameStatusEventHandler(
        LinFrameStatusEvent{transmission.timestamp, transmission.frame, masterFrameStatus});
}

void SimBehaviorTrivial::SendMsg(LinControllerStatusUpdate&& msg)
{
    SendMsgImpl(msg);
}

void SimBehaviorTrivial::ProcessFrameHeaderRequest(const LinSendFrameHeaderRequest& header)
{
    LinFrameResponse response = _parentController->GetThisLinNode().responses[header.id];
    if (response.responseMode != LinFrameResponseMode::TxUnconditional)
    {
        // Ignore headers if not configured to answer
        return;
    }
    if (response.frame.dataLength == LinDataLengthUnknown)
    {
        _parentController->ThrowOnSendAttemptWithUndefinedDataLength(response.frame);
    }

    // Dispatch the LIN transmission to all connected nodes
    LinTransmission transmission{_timeProvider->Now(), response.frame, LinFrameStatus::LIN_RX_OK};
    SendMsgImpl(transmission);

    auto direction = ToTracingDir(LinFrameStatus::LIN_RX_OK);
    if (_parentController->GetThisLinNode().controllerMode == LinControllerMode::Master)
    {
        direction = ToTracingDir(LinFrameStatus::LIN_TX_OK);
    }
    _parentController->GetTracer()->Trace(direction, transmission.timestamp, transmission.frame);

    // Evoke the callbacks with LIN_TX_OK locally
    _parentController->CallLinFrameStatusEventHandler(
        LinFrameStatusEvent{transmission.timestamp, transmission.frame, LinFrameStatus::LIN_TX_OK});
}

void SimBehaviorTrivial::UpdateTxBuffer(const LinFrame& /*frame*/)
{
    // NOP
}

auto SimBehaviorTrivial::CalcFrameStatus(const LinTransmission& linTransmission, bool /*isGoToSleepFrame*/)
    -> LinFrameStatus
{
    // dynamic controllers report every transmission as it was received
    if (_parentController->GetThisLinNode().simulationMode == WireLinControllerConfig::SimulationMode::Dynamic)
    {
        return linTransmission.status;
    }

    // Evaluate locally known response
    auto& thisLinNode = _parentController->GetThisLinNode();
    const auto response = thisLinNode.responses[linTransmission.frame.id];
    switch (response.responseMode)
    {
        case LinFrameResponseMode::Unused:
        {
            // Return NOT_OK to not trigger the reception callback
            return LinFrameStatus::NOT_OK;
        }
        case LinFrameResponseMode::Rx:
        {
            // Skip check if receiving with unknown DataLength
            const bool checkDataLength = (response.frame.dataLength != LinDataLengthUnknown);
            if (checkDataLength && (response.frame.dataLength != linTransmission.frame.dataLength))
            {
                _parentController->WarnOnWrongDataLength(linTransmission.frame, response.frame);
                return LinFrameStatus::LIN_RX_ERROR;
            }

            // Skip check if sending or receiving with unknown CSM
            const bool checkChecksumModel = (linTransmission.frame.checksumModel != LinChecksumModel::Unknown)
                                         && (response.frame.checksumModel != LinChecksumModel::Unknown);
            if (checkChecksumModel && (response.frame.checksumModel != linTransmission.frame.checksumModel))
            {
                _parentController->WarnOnWrongChecksum(linTransmission.frame, response.frame);
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

    _parentController->GetTracer()->Trace(ToTracingDir(gotosleepTx.status), gotosleepTx.timestamp, gotosleepTx.frame);
    SendMsgImpl(gotosleepTx);

    // Evoke the callbacks with LIN_TX_OK locally
    _parentController->CallLinFrameStatusEventHandler(
        LinFrameStatusEvent{gotosleepTx.timestamp, gotosleepTx.frame, LinFrameStatus::LIN_TX_OK});

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
