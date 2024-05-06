// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/experimental/netsim/all.hpp"
#include "MySimulatedNetwork.hpp"

class MySimulatedCanController : public SilKit::Experimental::NetworkSimulation::Can::ISimulatedCanController
{
    
public:
    MySimulatedCanController(MySimulatedNetwork* mySimulatedNetwork,
                             SilKit::Experimental::NetworkSimulation::ControllerDescriptor controllerDescriptor);

    // ISimulatedCanController

    void OnSetBaudrate(const SilKit::Experimental::NetworkSimulation::Can::CanConfigureBaudrate& msg) override;
    void OnFrameRequest(const SilKit::Experimental::NetworkSimulation::Can::CanFrameRequest& msg) override;
    void OnSetControllerMode(const SilKit::Experimental::NetworkSimulation::Can::CanControllerMode& msg) override;

private:

    MySimulatedNetwork* _mySimulatedNetwork;
    Scheduler* _scheduler;

    SilKit::Experimental::NetworkSimulation::ControllerDescriptor _controllerDescriptor;

    double _baudRate;
    SilKit::Services::Can::CanControllerState _controllerMode;

    SilKit::Services::Logging::ILogger* _logger;

};
