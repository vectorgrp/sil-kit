# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

# The Participant's connection interfaces are collected in this folder
# for easy consumption from external projects
add_library(I_SilKit_Core_Internal INTERFACE)
target_include_directories(I_SilKit_Core_Internal
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    # for internal type definitions
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../service # ServiceDiscovery is internal only special
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../requests # RequestReply is internal 
)
target_link_libraries(I_SilKit_Core_Internal
    INTERFACE SilKitInterface
    INTERFACE I_SilKit_Wire_Util
    INTERFACE I_SilKit_Wire_Can
    INTERFACE I_SilKit_Wire_Data
    INTERFACE I_SilKit_Wire_Ethernet
    INTERFACE I_SilKit_Wire_Flexray
    INTERFACE I_SilKit_Wire_Lin
    INTERFACE I_SilKit_Wire_Rpc
    INTERFACE I_SilKit_Config
    #for network simulator exports
    INTERFACE I_SilKit_Services_Can
    INTERFACE I_SilKit_Services_Lin
    INTERFACE I_SilKit_Services_Flexray
    INTERFACE I_SilKit_Services_Ethernet
    INTERFACE I_SilKit_Experimental_NetworkSimulatorInternals
    INTERFACE I_SilKit_Services_Metrics
)


add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_MessageBuffer.cpp LIBS I_SilKit_Core_Internal)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_InternalSerdes.cpp LIBS I_SilKit_Core_Internal)

