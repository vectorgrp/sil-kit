# Copyright (c) Vector Informatik GmbH. All rights reserved.
make_silkit_demo(SilKitDemoCLin LinDemo.c)
set_property(TARGET SilKitDemoCLin PROPERTY C_STANDARD 11)
target_sources(SilKitDemoCLin
    PRIVATE 
        SilKitConfig_DemoLin.json
        NetworkSimulatorConfig.json
)
