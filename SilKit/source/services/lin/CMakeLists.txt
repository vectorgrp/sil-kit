# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(I_SilKit_Services_Lin INTERFACE)

target_include_directories(I_SilKit_Services_Lin
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(I_SilKit_Services_Lin
    INTERFACE SilKitInterface

    INTERFACE I_SilKit_Wire_Lin
    INTERFACE I_SilKit_Tracing
)


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
    PUBLIC I_SilKit_Services_Lin

    PRIVATE I_SilKit_Core_Internal
    PRIVATE I_SilKit_Services_Logging
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES
        LinTestUtils.hpp
        Test_LinControllerDetailedSim.cpp
        Test_LinControllerTrivialSim.cpp
        Test_LinControllerConfig.cpp
    LIBS
        S_SilKitImpl
        I_SilKit_Core_Mock_Participant
        O_SilKit_Core_Mock_NullConnection
)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_LinSerdes.cpp LIBS S_SilKitImpl I_SilKit_Core_Internal)
