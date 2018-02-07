make_ib_demo(IbDemoCan CanDemo.cpp)

target_sources(IbDemoCan
    PRIVATE IbConfig_DemoCan.json
    PRIVATE IbConfig_DemoCan_NetSim.json
)
