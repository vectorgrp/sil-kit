# Copyright (c) Vector Informatik GmbH. All rights reserved.
make_ib_demo(IbDemoGenericMessage GenericMessageDemo.cpp)
target_sources(IbDemoGenericMessage
    PRIVATE IbConfig_DemoGenericMessage.json
)

make_ib_demo(IbDemoGenericMessageSerDes GenericMessageDemoSerDes.cpp)
target_sources(IbDemoGenericMessageSerDes
    PRIVATE IbConfig_DemoGenericMessageSerDes.json
)
