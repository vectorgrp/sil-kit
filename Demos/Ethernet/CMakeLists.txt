# Copyright (c) Vector Informatik GmbH. All rights reserved.
make_ib_demo(IbDemoEthernet  EthernetDemo.cpp)

target_sources(IbDemoEthernet
    PRIVATE IbConfig_DemoEthernet.json
    PRIVATE NetworkSimulatorConfig.json
)
