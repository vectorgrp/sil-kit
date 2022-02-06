# Copyright (c) Vector Informatik GmbH. All rights reserved.

make_ib_demo(IbDemoDynamicConfig CanDemoDynamic.cpp)
target_sources(IbDemoDynamicConfig
    PRIVATE IbConfig_DemoCanDynamic.json
)
