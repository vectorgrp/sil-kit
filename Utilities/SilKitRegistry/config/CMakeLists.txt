
add_library(I_SilKitRegistry_Config INTERFACE)

target_include_directories(I_SilKitRegistry_Config
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(I_SilKitRegistry_Config
    INTERFACE SilKitInterface

    INTERFACE I_SilKit_Config

    INTERFACE YamlInterface
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


add_silkit_test(Test_RegistryConfiguration
    SOURCES Test_RegistryConfiguration.cpp
    LIBS S_SilKitImpl O_SilKitRegistry_Config
    CONFIGS
        Test_RegistryConfiguration_Full.json
        Test_RegistryConfiguration_Full.yaml
)
