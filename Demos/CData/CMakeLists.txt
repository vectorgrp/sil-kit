make_ib_demo(IbDemoCData DataDemo.c)
set_property(TARGET IbDemoCData PROPERTY C_STANDARD 11)
target_sources(IbDemoCData
    PRIVATE IbConfig_DemoDataMessage.json
)
