# The Participant's connection interfaces are collected in this folder
# for easy consumption from external VIBE projects
add_library(IbInternal INTERFACE)
target_include_directories(IbInternal 
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    # for internal type definitions
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../service # ServiceDiscovery is internal only special
)

add_vib_test(TestMwVAsio_MessageBuffer SOURCES MessageBufferTest.cpp LIBS IbInternal)
