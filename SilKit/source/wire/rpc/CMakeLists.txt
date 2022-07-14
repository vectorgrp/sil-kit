# Copyright (c) Vector Informatik GmbH. All rights reserved.

add_library(SilKitWireRpc INTERFACE)
target_include_directories(SilKitWireRpc INTERFACE .)
target_link_libraries(SilKitWireRpc INTERFACE SilKitWireUtil)
