// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "MySimulatedCanController.hpp"
#include "silkit/services/can/CanDatatypes.hpp"

using namespace SilKit;
using namespace SilKit::Experimental::NetworkSimulation;
using namespace SilKit::Experimental::NetworkSimulation::Can;
using namespace std::chrono_literals;

MySimulatedCanController::MySimulatedCanController(MySimulatedNetwork* mySimulatedNetwork,
                                                   ControllerDescriptor controllerDescriptor)
    : _mySimulatedNetwork{mySimulatedNetwork}
    , _controllerDescriptor{controllerDescriptor}
    , _baudRate{0}
    , _controllerMode{}
{
    std::string controllerDescriptorStr = std::to_string(static_cast<uint64_t>(controllerDescriptor));

    _logger = _mySimulatedNetwork->GetLogger();
    std::stringstream logMsg;
    logMsg << "Registered SimulatedCanController #" << controllerDescriptorStr  << " on network '"
           << _mySimulatedNetwork->GetNetworkName() << "' of type '"
        << _mySimulatedNetwork->GetNetworkType() << "'";
    _logger->Info(logMsg.str());

    _scheduler = _mySimulatedNetwork->GetScheduler();
}

// ISimulatedCanController

void MySimulatedCanController::OnSetControllerMode(const CanControllerMode& controllerMode)
{
    std::stringstream logMsg;
    logMsg << "Received 'CanControllerMode' on network '" << _mySimulatedNetwork->GetNetworkName() << "' of type '"
        << _mySimulatedNetwork->GetNetworkType() << "'";
    _logger->Info(logMsg.str());

    _controllerMode = controllerMode.state;
}

void MySimulatedCanController::OnSetBaudrate(const CanConfigureBaudrate& configureBaudrate)
{
    std::stringstream logMsg;
    logMsg << "Received 'CanConfigureBaudrate' on network '" << _mySimulatedNetwork->GetNetworkName() << "' of type '"
           << _mySimulatedNetwork->GetNetworkType() << "'";
    _logger->Info(logMsg.str());

    _baudRate = configureBaudrate.baudRate;
}

void MySimulatedCanController::OnFrameRequest(const CanFrameRequest& frameRequest)
{
    std::stringstream logMsg;
    logMsg << "Received 'CanFrameRequest' on network '" << _mySimulatedNetwork->GetNetworkName() << "' of type '"
        << _mySimulatedNetwork->GetNetworkType() << "'";
    _logger->Info(logMsg.str());

    // Send acknowledge back to the sending CAN controller
    Services::Can::CanFrameTransmitEvent ack;
    ack.canId = frameRequest.frame.canId;
    ack.status = Services::Can::CanTransmitStatus::Transmitted;
    ack.timestamp = _scheduler->Now();
    ack.userContext = frameRequest.userContext;

    std::array<ControllerDescriptor, 1> receiverArray{_controllerDescriptor};
    auto receivers = SilKit::Util::MakeSpan(receiverArray);
    _mySimulatedNetwork->GetCanEventProducer()->Produce(std::move(ack), receivers);

    // Distribute the frame to all controllers in the network with delay
    Services::Can::CanFrameEvent frameEvent;
    frameEvent.direction = Services::TransmitDirection::RX;
    frameEvent.frame = frameRequest.frame;
    frameEvent.userContext = frameRequest.userContext;
    std::vector<uint8_t> payloadBytes{frameRequest.frame.dataField.begin(), frameRequest.frame.dataField.end()};

    _scheduler->ScheduleEvent(2ms, [this, frameEvent, payloadBytes = std::move(payloadBytes)]() mutable {
        frameEvent.frame.dataField = SilKit::Util::ToSpan(payloadBytes);
        frameEvent.timestamp = _scheduler->Now();
        _mySimulatedNetwork->GetCanEventProducer()->Produce(
            frameEvent, _mySimulatedNetwork->GetAllControllerDescriptors());
    });
}
