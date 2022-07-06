# Copyright (c) Vector Informatik GmbH. All rights reserved.
make_silkit_demo(SilKitDemoCEthernet EthernetDemo.c)
set_property(TARGET SilKitDemoCEthernet PROPERTY C_STANDARD 11)
target_sources(SilKitDemoCEthernet
    PRIVATE 
        SilKitConfig_DemoEthernet.json
        NetworkSimulatorConfig.json
)
