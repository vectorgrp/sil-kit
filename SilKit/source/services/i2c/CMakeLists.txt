# SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(I_SilKit_Services_I2c INTERFACE)

target_include_directories(I_SilKit_Services_I2c
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(I_SilKit_Services_I2c
    INTERFACE SilKitInterface

    INTERFACE I_SilKit_Wire_I2c
    INTERFACE I_SilKit_Core_Internal
    INTERFACE I_SilKit_Config
    INTERFACE I_SilKit_Tracing
    INTERFACE I_SilKit_Services_Logging
)


add_library(O_SilKit_Services_I2c OBJECT
    I2cController.cpp
    I2cController.hpp
    ISimBehavior.hpp
    SimBehavior.cpp
    SimBehavior.hpp
    SimBehaviorDetailed.cpp
    SimBehaviorDetailed.hpp
    SimBehaviorTrivial.cpp
    SimBehaviorTrivial.hpp

    I2cSerdes.hpp
    I2cSerdes.cpp
)

target_link_libraries(O_SilKit_Services_I2c
    PUBLIC I_SilKit_Services_I2c
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES
        Test_I2cControllerTrivialSim.cpp
    LIBS
        S_SilKitImpl
        I_SilKit_Core_Mock_Participant
        O_SilKit_Core_Mock_NullConnection
)
