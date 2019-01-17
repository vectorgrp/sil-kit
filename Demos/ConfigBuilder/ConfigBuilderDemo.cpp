// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "ib/cfg/Config.hpp"
#include "ib/cfg/ConfigBuilder.hpp"

int main(int, char**)
{
    using namespace ib::cfg;

    ConfigBuilder myConfig("ExampleConfig");
    auto&& simulationSetup = myConfig.SimulationSetup();

    simulationSetup
        .AddParticipant("Alpha")
            .WithParticipantId(2)
            ->AddCan("CAN1")
                .WithLink("CAN1")
                .WithEndpointId(17)
            ->AddCan("CAN2")
            ->AddCan("CAN3").WithLink("CAN-2000")
            ->AddLin("LIN1")
            ->AddEthernet("ETH0").WithLink("LAN1")
            ->AddAnalogOut("AIO").WithInitValue(3.7).WithUnit("V")
            ->AddDigitalOut("DIO").WithInitValue(false)
            ->AddPwmOut("PWM").WithInitValue({2.5, 0.4}).WithUnit("kHz")
            ->AddPatternOut("PATTERN").WithInitValue({'H', 'e', 'l', 'l', 'o'}).WithLink("PTRIO");


    simulationSetup
        .AddParticipant("Beta")
            ->AddCan("CAN1")
            ->AddCan("CAN3").WithLink("CAN-2000")
            ->AddLin("LIN1")
            ->AddEthernet("ETH0")
                .WithLink("LAN1")
                .WithMacAddress("F6:04:68:71:AA:C1")
            ->AddAnalogOut("AIO")
            ->AddDigitalOut("DIO")
            ->AddPwmOut("PWM")
            ->AddPatternOut("PATTERN").WithLink("PTRIO");

    simulationSetup
        .AddParticipant("SystemController").AsSyncMaster();

    simulationSetup
        .AddParticipant("BusSimulator")
            ->AddNetworkSimulator("BusSimulator")
                .WithLinks({"CAN1", "CAN2"})
                .WithSwitches({"FrontSwitch"});

    simulationSetup
        .AddSwitch("FrontSwitch")
            ->AddPort("Port1").WithVlanIds({1,2})
            ->AddPort("Port2").WithVlanIds({2,3})
            ->AddPort("Port3").WithVlanIds({3,4});

    simulationSetup
        .ConfigureTimeSync();

    auto config = myConfig.Build();

    auto jsonString = config.ToJsonString();
    std::cout << jsonString << std::endl;

    return 0;
}
