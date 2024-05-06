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

class MySimulatedNetwork : public SilKit::Experimental::NetworkSimulation::ISimulatedNetwork
{
public:

    MySimulatedNetwork(SilKit::IParticipant* participant, Scheduler* scheduler,
                       SilKit::Experimental::NetworkSimulation::SimulatedNetworkType networkType,
                       std::string networkName);

    // ISimulatedNetwork
    auto ProvideSimulatedController(SilKit::Experimental::NetworkSimulation::ControllerDescriptor controllerDescriptor)
        -> SilKit::Experimental::NetworkSimulation::ISimulatedController* override;

    void SetEventProducer(
        std::unique_ptr<SilKit::Experimental::NetworkSimulation::IEventProducer> eventProducer) override;

    void SimulatedControllerRemoved(
        SilKit::Experimental::NetworkSimulation::ControllerDescriptor controllerDescriptor) override;

    // Getter for children
    auto GetLogger() -> SilKit::Services::Logging::ILogger*
    {
        return _logger;
    }
    auto GetNetworkType() -> SilKit::Experimental::NetworkSimulation::SimulatedNetworkType
    {
        return _networkType;
    }
    auto GetNetworkName() -> std::string
    {
        return _networkName;
    }
    auto GetAllControllerDescriptors()
        -> SilKit::Util::Span<SilKit::Experimental::NetworkSimulation::ControllerDescriptor>
    {
        return SilKit::Util::ToSpan(_controllerDescriptors);
    }
    auto GetScheduler() -> Scheduler*
    {
        return _scheduler;
    }
    auto GetCanEventProducer() -> SilKit::Experimental::NetworkSimulation::Can::ICanEventProducer*
    {
        return static_cast<SilKit::Experimental::NetworkSimulation::Can::ICanEventProducer*>(_eventProducer.get());
    }

private:

    // On construction
    SilKit::IParticipant* _participant;
    SilKit::Services::Logging::ILogger* _logger;
    Scheduler* _scheduler;
    SilKit::Experimental::NetworkSimulation::SimulatedNetworkType _networkType;
    std::string _networkName;

    // Aggregated
    std::unique_ptr<SilKit::Experimental::NetworkSimulation::IEventProducer> _eventProducer;
    std::vector<std::unique_ptr<SilKit::Experimental::NetworkSimulation::ISimulatedController>> _mySimulatedControllers;
    std::vector<SilKit::Experimental::NetworkSimulation::ControllerDescriptor> _controllerDescriptors;
};