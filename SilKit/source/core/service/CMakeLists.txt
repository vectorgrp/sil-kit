# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(I_SilKit_Core_Service INTERFACE)

target_include_directories(I_SilKit_Core_Service
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(I_SilKit_Core_Service
    INTERFACE SilKitInterface

    INTERFACE I_SilKit_Config
    INTERFACE I_SilKit_Core_Internal
    INTERFACE I_SilKit_Util
)


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
    PUBLIC I_SilKit_Core_Service

    PRIVATE I_SilKit_Services_Logging
    PRIVATE rapidyaml
)


add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_ServiceDiscovery.cpp 
    LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant I_SilKit_Util_Uuid)


add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_ServiceSerdes.cpp LIBS S_SilKitImpl)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_SpecificDiscoveryStore.cpp 
    LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant I_SilKit_Util_Uuid)

