# Copyright (c) Vector Informatik GmbH. All rights reserved.

make_silkit_demo(SilKitDemoLifecycle LifecycleDemo.cpp)

target_sources(SilKitDemoLifecycle
    PRIVATE SilKitConfig_DemoLifecycle.json
)
