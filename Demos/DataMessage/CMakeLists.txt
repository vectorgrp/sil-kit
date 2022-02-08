# Copyright (c) Vector Informatik GmbH. All rights reserved.

make_ib_demo(IbDemoDataMessage DataMessageDemo.cpp)
target_sources(IbDemoDataMessage
    PRIVATE IbConfig_DemoDataMessage.json
)

make_ib_demo(IbDemoDataMessageExt DataMessageDemoExt.cpp)
target_sources(IbDemoDataMessageExt
    PRIVATE IbConfig_DemoDataMessageExt.json
)
