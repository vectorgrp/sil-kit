// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <unordered_set>
#include <memory>

#include "silkit/SilKit.hpp"
#include "silkit/experimental/netsim/string_utils.hpp"
#include "silkit/experimental/netsim/all.hpp"

#include "Scheduler.hpp"

using namespace SilKit::Experimental::NetworkSimulation;

class MySimulatedNetwork : public ISimulatedNetwork
{
public:

    MySimulatedNetwork(SilKit::IParticipant* participant, Scheduler* scheduler, SimulatedNetworkType networkType,
                       std::string networkName);

    // ISimulatedNetwork
    auto ProvideSimulatedController(ControllerDescriptor controllerDescriptor)
        -> ISimulatedController* override;

    void SetEventProducer(std::unique_ptr<IEventProducer> eventProducer) override;

    void SimulatedControllerRemoved(ControllerDescriptor controllerDescriptor) override;

    // Getter for children
    auto GetLogger()
    {
        return _logger;
    }
    auto GetNetworkType()
    {
        return _networkType;
    }
    auto GetNetworkName()
    {
        return _networkName;
    }
    auto GetAllControllerDescriptors()
    {
        return SilKit::Util::ToSpan(_controllerDescriptors);
    }
    auto GetScheduler()
    {
        return _scheduler;
    }
    auto GetCanEventProducer()
    {
        return static_cast<Can::ICanEventProducer*>(_eventProducer.get());
    }

private:

    // On construction
    SilKit::IParticipant* _participant;
    SilKit::Services::Logging::ILogger* _logger;
    Scheduler* _scheduler;
    SimulatedNetworkType _networkType;
    std::string _networkName;

    // Aggregated
    std::unique_ptr<IEventProducer> _eventProducer;
    std::vector<std::unique_ptr<ISimulatedController>> _mySimulatedControllers;
    std::vector<ControllerDescriptor> _controllerDescriptors;
};