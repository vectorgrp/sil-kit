# Copyright (c) Vector Informatik GmbH. All rights reserved.
make_ib_demo(IbDemoCEthernet EthernetDemo.c)
set_property(TARGET IbDemoCEthernet PROPERTY C_STANDARD 11)
target_sources(IbDemoCEthernet
    PRIVATE IbConfig_DemoEthernet_SingleParticipant.json
)
