// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/experimental/netsim/all.hpp"
#include "MySimulatedNetwork.hpp"

using namespace SilKit;
using namespace SilKit::Experimental::NetworkSimulation;
using namespace SilKit::Experimental::NetworkSimulation::Can;

class MySimulatedCanController : public ISimulatedCanController
{
    
public:
    MySimulatedCanController(MySimulatedNetwork* mySimulatedNetwork, ControllerDescriptor controllerDescriptor);

    // ISimulatedCanController

    void OnSetBaudrate(const CanConfigureBaudrate& msg) override;
    void OnFrameRequest(const CanFrameRequest& msg) override;
    void OnSetControllerMode(const CanControllerMode& msg) override;

private:

    MySimulatedNetwork* _mySimulatedNetwork;
    Scheduler* _scheduler;

    ControllerDescriptor _controllerDescriptor;

    double _baudRate;
    Services::Can::CanControllerState _controllerMode;

    SilKit::Services::Logging::ILogger* _logger;

};
