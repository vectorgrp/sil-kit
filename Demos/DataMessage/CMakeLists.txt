# Copyright (c) Vector Informatik GmbH. All rights reserved.

make_ib_demo(IbDemoDataMessage DataMessageDemo.cpp)
target_sources(IbDemoDataMessage
    PRIVATE IbConfig_DemoDataMessage.json
)
