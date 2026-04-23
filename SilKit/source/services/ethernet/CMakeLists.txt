# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

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
    PRIVATE I_SilKit
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES 
        Test_EthControllerTrivialSim.cpp
        Test_EthControllerDetailedSim.cpp
        Test_EthControllerConfig.cpp
    LIBS
        S_SilKitImpl
        I_SilKit
        O_SilKit_Core_Mock_NullConnection
)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_EthernetSerdes.cpp LIBS S_SilKitImpl I_SilKit)
