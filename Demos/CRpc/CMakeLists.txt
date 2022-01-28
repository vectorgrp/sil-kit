make_ib_demo(IbDemoCRpc RpcDemo.c)
set_property(TARGET IbDemoCRpc PROPERTY C_STANDARD 11)
target_sources(IbDemoCRpc
    PRIVATE IbConfig_DemoRpc_Single.json
)
