# Copyright (c) Vector Informatik GmbH. All rights reserved.
make_ib_demo(IbDemoFlexray FlexrayDemo.cpp)

target_sources(IbDemoFlexray
    PRIVATE IbConfig_DemoFlexRay.json
    PRIVATE NetworkSimulatorConfig.json
)
