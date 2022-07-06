# Copyright (c) Vector Informatik GmbH. All rights reserved.
make_silkit_demo(SilKitDemoLin LinDemo.cpp)

target_sources(SilKitDemoLin 
	PRIVATE SilKitConfig_DemoLin.json
    PRIVATE NetworkSimulatorConfig.json
)
