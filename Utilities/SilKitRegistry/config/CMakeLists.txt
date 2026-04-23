# SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(O_SilKitRegistry_Config OBJECT
    RegistryConfiguration.cpp
)

target_include_directories(O_SilKitRegistry_Config
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(O_SilKitRegistry_Config
    PRIVATE I_SilKit
    PRIVATE fmt::fmt
    PRIVATE rapidyaml
)


add_silkit_test_executable(SilKitRegistryTests)

if(TARGET SilKitRegistryTests)
    target_compile_definitions(SilKitRegistryTests
        PRIVATE EXPORT_SilKitAPI
    )
endif()

add_silkit_test_to_executable(SilKitRegistryTests
    SOURCES
    Test_RegistryConfiguration.cpp

    LIBS
    I_SilKit
    S_SilKitImpl
    O_SilKitRegistry_Config
    O_SilKit_Config

    CONFIGS
    Test_RegistryConfiguration_Full.json
    Test_RegistryConfiguration_Full.yaml
)
