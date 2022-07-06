# Copyright (c) Vector Informatik GmbH. All rights reserved.
make_silkit_demo(SilKitDemoCCan CanDemo.c)
set_property(TARGET SilKitDemoCCan PROPERTY C_STANDARD 11)
target_sources(SilKitDemoCCan
    PRIVATE 
        SilKitConfig_DemoCan.json
        NetworkSimulatorConfig.json
)

make_silkit_demo(SilKitDemoCCanSync CanDemoSync.c)
set_property(TARGET SilKitDemoCCanSync PROPERTY C_STANDARD 11)
target_sources(SilKitDemoCCanSync
    PRIVATE 
        SilKitConfig_DemoCan.json
        NetworkSimulatorConfig.json
)
