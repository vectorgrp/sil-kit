make_silkit_demo(SilKitDemoCData DataDemo.c)
set_property(TARGET SilKitDemoCData PROPERTY C_STANDARD 11)
target_sources(SilKitDemoCData
    PRIVATE SilKitConfig_DemoDataMessage.json
)
