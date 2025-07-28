# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(I_SilKit_Services_Can INTERFACE)

target_include_directories(I_SilKit_Services_Can
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(I_SilKit_Services_Can
    INTERFACE SilKitInterface

    INTERFACE I_SilKit_Wire_Can
    INTERFACE I_SilKit_Core_Internal
    INTERFACE I_SilKit_Config
    INTERFACE I_SilKit_Tracing
    INTERFACE I_SilKit_Services_Logging
)


add_library(O_SilKit_Services_Can OBJECT
    CanDatatypesUtils.cpp
    CanDatatypesUtils.hpp
    CanController.cpp
    CanController.hpp
    ISimBehavior.hpp
    SimBehavior.cpp
    SimBehavior.hpp
    SimBehaviorDetailed.cpp
    SimBehaviorDetailed.hpp
    SimBehaviorTrivial.cpp
    SimBehaviorTrivial.hpp

    CanSerdes.hpp
    CanSerdes.cpp
)

target_link_libraries(O_SilKit_Services_Can
    PUBLIC I_SilKit_Services_Can
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES 
        Test_CanControllerConfig.cpp
        Test_CanControllerDetailedSim.cpp 
        Test_CanControllerTrivialSim.cpp
    LIBS 
        S_SilKitImpl
        I_SilKit_Core_Mock_Participant
        O_SilKit_Core_Mock_NullConnection
)

add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_CanSerdes.cpp LIBS S_SilKitImpl I_SilKit_Core_Internal)

add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_CanStringUtils.cpp LIBS S_SilKitImpl)
