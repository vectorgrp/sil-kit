# Copyright (c) Vector Informatik GmbH. All rights reserved.

make_ib_demo(IbDemoRpc RpcDemo.cpp)
target_sources(IbDemoRpc
    PRIVATE IbConfig_DemoRpc.json
)
