# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(I_SilKit_Services_Flexray INTERFACE)

target_include_directories(I_SilKit_Services_Flexray
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(I_SilKit_Services_Flexray
    INTERFACE SilKitInterface
)


add_library(O_SilKit_Services_Flexray OBJECT
    FlexrayController.cpp
    FlexrayController.hpp
    FlexrayDatatypeUtils.cpp
    FlexrayDatatypeUtils.hpp
    Validation.cpp
    Validation.hpp

    FlexraySerdes.hpp
    FlexraySerdes.cpp
)

target_link_libraries(O_SilKit_Services_Flexray
    PUBLIC I_SilKit_Services_Flexray

    PRIVATE I_SilKit_Core_Internal
    PRIVATE I_SilKit_Services_Logging
    PRIVATE I_SilKit_Tracing
    PRIVATE I_SilKit_Config
)

add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_Validation.cpp LIBS S_SilKitImpl)
add_silkit_test_to_executable(SilKitUnitTests
    SOURCES
        Test_FlexrayController.cpp
        Test_FlexrayControllerConfig.cpp
    LIBS
        S_SilKitImpl
        I_SilKit_Core_Mock_Participant
        O_SilKit_Core_Mock_NullConnection
)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_FlexraySerdes.cpp LIBS S_SilKitImpl I_SilKit_Core_Internal)
