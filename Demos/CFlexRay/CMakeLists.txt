make_ib_demo(IbDemoCFlexray FlexrayDemo.c)

target_sources(IbDemoCFlexray
    PRIVATE IbConfig_DemoFlexray.json
    PRIVATE IbConfig_DemoFlexray_NetSim.json
)
