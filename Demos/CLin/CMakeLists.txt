# Copyright (c) Vector Informatik GmbH. All rights reserved.
make_ib_demo(IbDemoCLin LinDemo.c)
set_property(TARGET IbDemoCLin PROPERTY C_STANDARD 11)
target_sources(IbDemoCLin
    PRIVATE 
        IbConfig_DemoLin.json
        NetworkSimulatorConfig.json
)
