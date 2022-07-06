# Copyright (c) Vector Informatik GmbH. All rights reserved.

make_silkit_demo(SilKitDemoRpc RpcDemo.cpp)
target_sources(SilKitDemoRpc
    PRIVATE SilKitConfig_DemoRpc.json
)
