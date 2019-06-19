add_library(json11
    ${THIRDPARTY_DIR}/json11/json11.hpp
    ${THIRDPARTY_DIR}/json11/json11.cpp
    )

target_include_directories(json11
    PUBLIC ${THIRDPARTY_DIR}/json11
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(TARGET json11 PROPERTY FOLDER "ThirdParty")


add_library(IbConfig STATIC
    Config.cpp
    ConfigBuilder.cpp
    ControllerBuilder.cpp
    GenericPortBuilder.cpp
    LinkBuilder.cpp
    NetworkSimulatorBuilder.cpp
    ParticipantBuilder.cpp
    SimulationSetupBuilder.cpp
    SwitchBuilder.cpp
    SwitchPortBuilder.cpp
    TimeSyncBuilder.cpp
    FastRtpsConfigBuilder.cpp
    JsonConfig.hpp
    JsonConfig.cpp
    Validation.hpp
    Validation.cpp
)

target_link_libraries(IbConfig
    PRIVATE IbInterface
    PRIVATE IbUtil
    PRIVATE json11
)

target_include_directories(IbConfig
    PUBLIC .
)

add_mvsc_ib_include_filter()

add_vib_test(TestMwCfgConfigBuilder SOURCES ConfigBuilderTest.cpp LIBS IbConfig)
add_vib_test(TestMwCfgJsonConfig SOURCES JsonConfigTest.cpp LIBS IbConfig json11)
add_vib_test(TestMwCfgValidation SOURCES ValidationTest.cpp LIBS IbConfig)
