# Copyright (c) Vector Informatik GmbH. All rights reserved.

make_ib_demo(IbDemoLifecycle LifecycleDemo.cpp)

target_sources(IbDemoLifecycle
    PRIVATE IbConfig_DemoLifecycle.json
)
