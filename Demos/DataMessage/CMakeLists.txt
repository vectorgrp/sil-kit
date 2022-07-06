# Copyright (c) Vector Informatik GmbH. All rights reserved.

make_silkit_demo(SilKitDemoDataMessage DataMessageDemo.cpp)
target_sources(SilKitDemoDataMessage
    PRIVATE SilKitConfig_DemoDataMessage.json
)

make_silkit_demo(SilKitDemoDataMessageExt DataMessageDemoExt.cpp)
target_sources(SilKitDemoDataMessageExt
    PRIVATE SilKitConfig_DemoDataMessage.json
)
