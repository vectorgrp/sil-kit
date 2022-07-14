# Copyright (c) Vector Informatik GmbH. All rights reserved.

add_library(SilKitWireData INTERFACE)
target_include_directories(SilKitWireData INTERFACE .)
target_link_libraries(SilKitWireData INTERFACE SilKitWireUtil)
