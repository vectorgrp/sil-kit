# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(O_SilKit_Services_Lin OBJECT
    LinController.cpp
    LinController.hpp

    ISimBehavior.hpp
    ILinControllerExtensions.hpp
    SimBehavior.cpp
    SimBehavior.hpp
    SimBehaviorDetailed.cpp
    SimBehaviorDetailed.hpp
    SimBehaviorTrivial.cpp

    LinSerdes.hpp
    LinSerdes.cpp
)

target_link_libraries(O_SilKit_Services_Lin
    PRIVATE I_SilKit
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES
        LinTestUtils.hpp
        Test_LinControllerDetailedSim.cpp
        Test_LinControllerTrivialSim.cpp
        Test_LinControllerConfig.cpp
    LIBS
        I_SilKit
        S_SilKitImpl
        O_SilKit_Core_Mock_NullConnection
)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_LinSerdes.cpp LIBS S_SilKitImpl I_SilKit)
