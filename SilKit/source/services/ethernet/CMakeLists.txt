# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(I_SilKit_Services_Ethernet INTERFACE)

target_include_directories(I_SilKit_Services_Ethernet
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(I_SilKit_Services_Ethernet
    INTERFACE I_SilKit_Core_Internal
    INTERFACE I_SilKit_Tracing
    INTERFACE I_SilKit_Wire_Ethernet
    INTERFACE I_SilKit_Services_Logging
)


add_library(O_SilKit_Services_Ethernet OBJECT
    EthController.cpp
    EthController.hpp

    ISimBehavior.hpp
    SimBehavior.cpp
    SimBehavior.hpp
    SimBehaviorDetailed.cpp
    SimBehaviorDetailed.hpp
    SimBehaviorTrivial.cpp
    SimBehaviorTrivial.hpp

    EthernetSerdes.hpp
    EthernetSerdes.cpp
)

target_link_libraries(O_SilKit_Services_Ethernet
    PUBLIC I_SilKit_Services_Ethernet

    PRIVATE I_SilKit_Core_Internal
    PRIVATE SilKitInterface
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES 
        Test_EthControllerTrivialSim.cpp
        Test_EthControllerDetailedSim.cpp
        Test_EthControllerConfig.cpp
    LIBS
        S_SilKitImpl
        I_SilKit_Core_Mock_Participant
        O_SilKit_Core_Mock_NullConnection
)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_EthernetSerdes.cpp LIBS S_SilKitImpl I_SilKit_Core_Internal)
