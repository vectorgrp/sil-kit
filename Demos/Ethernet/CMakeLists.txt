# Copyright (c) Vector Informatik GmbH. All rights reserved.
make_silkit_demo(SilKitDemoEthernet  EthernetDemo.cpp)

target_sources(SilKitDemoEthernet
    PRIVATE SilKitConfig_DemoEthernet.json
    PRIVATE NetworkSimulatorConfig.json
)
