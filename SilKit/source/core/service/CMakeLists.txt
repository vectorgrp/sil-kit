# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(O_SilKit_Core_Service OBJECT
    ServiceDatatypes.hpp
    ServiceDiscovery.hpp
    ServiceDiscovery.cpp
    SpecificDiscoveryStore.cpp
    SpecificDiscoveryStore.hpp

    ServiceSerdes.hpp
    ServiceSerdes.cpp
)

target_link_libraries(O_SilKit_Core_Service
    PRIVATE I_SilKit
)


add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_ServiceDiscovery.cpp 
    LIBS S_SilKitImpl I_SilKit
)

add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_ServiceSerdes.cpp LIBS S_SilKitImpl)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_SpecificDiscoveryStore.cpp 
    LIBS S_SilKitImpl I_SilKit
)
