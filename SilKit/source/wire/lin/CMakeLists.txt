# Copyright (c) Vector Informatik GmbH. All rights reserved.

add_library(SilKitWireLin INTERFACE)
target_include_directories(SilKitWireLin INTERFACE .)
target_link_libraries(SilKitWireLin INTERFACE SilKitWireUtil)
