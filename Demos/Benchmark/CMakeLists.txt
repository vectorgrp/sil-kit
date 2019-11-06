cmake_minimum_required(VERSION 3.5)
project("IbDemoBenchmark" LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 14)

add_executable(IbDemoBenchmark
    BenchmarkDemo.cpp
)

add_definitions(-DEXPORT_IntegrationBusAPI)

set_target_properties(IbDemoBenchmark PROPERTIES
    FOLDER "Demos"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Demos/$<CONFIG>/bin"
    )

target_link_libraries(IbDemoBenchmark
    PRIVATE IbMwComAdapter
)

if(ib_can_install)
	install(
		TARGETS IbDemoBenchmark
		RUNTIME DESTINATION ${INSTALL_BINDIR}
		COMPONENT IntegrationBus-Demos
	)
endif()

