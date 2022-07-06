# Copyright (c) Vector Informatik GmbH. All rights reserved.
make_silkit_demo(SilKitDemoCan CanDemo.cpp)

target_sources(SilKitDemoCan
    PRIVATE SilKitConfig_DemoCan.json
    PRIVATE NetworkSimulatorConfig.json
)
