find_package(Threads REQUIRED)

make_ib_demo(IbDemoGenericMessageITest GenericMessageITest.cpp)

target_link_libraries(IbDemoGenericMessageITest PUBLIC Threads::Threads)

target_sources(IbDemoGenericMessageITest
    PRIVATE IbConfig_DemoGenericMessageITest.json
)