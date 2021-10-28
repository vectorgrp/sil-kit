add_vib_test(TestCapiEthernet SOURCES CapiEthernetTest.cpp LIBS IbSimEthernet IntegrationBus)
#target_link_libraries(TestCapiEthernet INTERFACE IbSimEthernet)
set_target_properties(TestCapiEthernet PROPERTIES
RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>"
)