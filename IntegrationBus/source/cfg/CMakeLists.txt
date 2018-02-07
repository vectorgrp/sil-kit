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
)

target_link_libraries(IbConfig
    PRIVATE IbInterface
    PRIVATE IbUtil
    PRIVATE json11
)

add_mvsc_ib_include_filter()

make_gtest(TestConfigBuilder ConfigBuilderTest.cpp IbConfig)
make_gtest(TestJsonConfig JsonConfigTest.cpp IbConfig json11)
