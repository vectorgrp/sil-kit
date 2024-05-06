// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "MySimulatedNetwork.hpp"
#include "Can/MySimulatedCanController.hpp"

using namespace SilKit::Experimental::NetworkSimulation;

MySimulatedNetwork::MySimulatedNetwork(SilKit::IParticipant* participant,
                                       Scheduler* scheduler,
                                       SimulatedNetworkType networkType, std::string networkName)
    : _participant{participant}
    , _logger{_participant->GetLogger()}
    , _scheduler{scheduler}
    , _networkType{networkType}
    , _networkName{networkName}
    , _mySimulatedControllers{}
{
    std::stringstream msg;
    msg << "Registered SimulatedNetwork '" << networkName << "' of type '" << _networkType << "'";
    _logger->Info(msg.str());

}

// ISimulatedNetwork

void MySimulatedNetwork::SetEventProducer(std::unique_ptr<IEventProducer> eventProducer) {
    _eventProducer = std::move(eventProducer);
}

auto MySimulatedNetwork::ProvideSimulatedController(ControllerDescriptor controllerDescriptor) -> ISimulatedController*
{

    _controllerDescriptors.push_back(controllerDescriptor);

    switch (_networkType)
    {
    case SimulatedNetworkType::CAN:
    {
        _mySimulatedControllers.emplace_back(std::make_unique<MySimulatedCanController>(this, controllerDescriptor));
        return _mySimulatedControllers.back().get();
    }
    default:
        break;
    }

    return {};
}

void MySimulatedNetwork::SimulatedControllerRemoved(ControllerDescriptor /*controllerDescriptor*/)
{
}
