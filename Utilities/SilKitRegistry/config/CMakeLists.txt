# SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(I_SilKitRegistry_Config INTERFACE)

target_include_directories(I_SilKitRegistry_Config
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(I_SilKitRegistry_Config
    INTERFACE SilKitInterface

    INTERFACE I_SilKit_Config

    INTERFACE ${SILKIT_THIRD_PARTY_YAML_INTERFACE}
)


add_library(O_SilKitRegistry_Config OBJECT
    RegistryConfiguration.cpp
    RegistryYamlConversion.cpp
)

target_link_libraries(O_SilKitRegistry_Config
    PUBLIC I_SilKitRegistry_Config

    PUBLIC I_SilKit_Config

    PUBLIC fmt::fmt
    PUBLIC yaml-cpp
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
    S_SilKitImpl
    O_SilKitRegistry_Config

    CONFIGS
    Test_RegistryConfiguration_Full.json
    Test_RegistryConfiguration_Full.yaml
)
