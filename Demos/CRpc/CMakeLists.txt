make_silkit_demo(SilKitDemoCRpc RpcDemo.c)
set_property(TARGET SilKitDemoCRpc PROPERTY C_STANDARD 11)
target_sources(SilKitDemoCRpc
    PRIVATE SilKitConfig_DemoRpc.json
)
