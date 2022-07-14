# Copyright (c) Vector Informatik GmbH. All rights reserved.

add_library(SilKitWireEthernet INTERFACE)
target_include_directories(SilKitWireEthernet INTERFACE .)
target_link_libraries(SilKitWireEthernet INTERFACE SilKitWireUtil)
