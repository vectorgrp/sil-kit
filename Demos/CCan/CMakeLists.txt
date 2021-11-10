# Copyright (c) Vector Informatik GmbH. All rights reserved.
make_ib_demo(IbDemoCCan CanDemo.c)
set_property(TARGET IbDemoCCan PROPERTY C_STANDARD 11)
target_sources(IbDemoCCan
    PRIVATE VirtualCanCable1_VAsio.json
)
