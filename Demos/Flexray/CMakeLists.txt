# Copyright (c) Vector Informatik GmbH. All rights reserved.
make_ib_demo(IbDemoFlexray FlexrayDemo.cpp)

target_sources(IbDemoFlexray
    PRIVATE IbConfig_DemoFlexray.json
    PRIVATE NetworkSimulatorConfig.json
)
