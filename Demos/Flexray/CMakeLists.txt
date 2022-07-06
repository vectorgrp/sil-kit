# Copyright (c) Vector Informatik GmbH. All rights reserved.
make_silkit_demo(SilKitDemoFlexray FlexrayDemo.cpp)

target_sources(SilKitDemoFlexray
    PRIVATE SilKitConfig_DemoFlexray.json
    PRIVATE NetworkSimulatorConfig.json
)
