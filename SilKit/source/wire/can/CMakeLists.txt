# Copyright (c) Vector Informatik GmbH. All rights reserved.

add_library(SilKitWireCan INTERFACE)
target_include_directories(SilKitWireCan INTERFACE .)
target_link_libraries(SilKitWireCan INTERFACE SilKitWireUtil)
