make_ib_demo(IbDemoEthernet  EthernetDemo.cpp)

target_sources(IbDemoEthernet
    PRIVATE IbConfig_DemoEthernet.json
    PRIVATE IbConfig_DemoEthernet_NetSim.json
)
