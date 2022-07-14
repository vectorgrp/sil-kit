# Copyright (c) Vector Informatik GmbH. All rights reserved.

add_library(SilKitWireFlexray INTERFACE)
target_include_directories(SilKitWireFlexray INTERFACE .)
target_link_libraries(SilKitWireFlexray INTERFACE SilKitWireUtil)
