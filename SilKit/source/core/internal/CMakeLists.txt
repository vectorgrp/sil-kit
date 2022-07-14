# The Participant's connection interfaces are collected in this folder
# for easy consumption from external projects
add_library(SilKitInternal INTERFACE)
target_include_directories(SilKitInternal 
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    # for internal type definitions
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../service # ServiceDiscovery is internal only special
)
target_link_libraries(
    SilKitInternal
    INTERFACE SilKitWireUtil
    INTERFACE SilKitWireCan
    INTERFACE SilKitWireData
    INTERFACE SilKitWireEthernet
    INTERFACE SilKitWireFlexray
    INTERFACE SilKitWireLin
    INTERFACE SilKitWireRpc
)

add_silkit_test(TestMwVAsio_MessageBuffer SOURCES MessageBufferTest.cpp LIBS SilKitInternal)
add_silkit_test(TestMwInternal_Serdes  SOURCES InternalSerdesTest.cpp LIBS SilKitInternal)
