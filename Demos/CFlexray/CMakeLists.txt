make_silkit_demo(SilKitDemoCFlexray FlexrayDemo.c)

target_sources(SilKitDemoCFlexray
    PRIVATE
        SilKitConfig_DemoFlexray.json
        NetworkSimulatorConfig.json
)
