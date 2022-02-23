make_ib_demo(IbDemoCFlexray FlexrayDemo.c)

target_sources(IbDemoCFlexray
    PRIVATE 
        IbConfig_DemoFlexRay.json
        NetworkSimulatorConfig.json
)
