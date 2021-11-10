# Copyright (c) Vector Informatik GmbH. All rights reserved.
make_ib_demo(IbDemoLin LinDemo.cpp)

target_sources(IbDemoLin 
	PRIVATE IbConfig_DemoLin.json
	PRIVATE IbConfig_DemoLin_NetSim.json
)
