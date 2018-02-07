make_ib_demo(IbDemoFlexray FlexrayDemo.cpp)

target_sources(IbDemoFlexray
    PRIVATE IbConfig_DemoFlexray.json
    PRIVATE IbConfig_DemoFlexray_NetSim.json
)
