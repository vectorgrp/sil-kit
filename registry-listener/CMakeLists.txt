cmake_minimum_required(VERSION 3.16)
project(RegistryListener LANGUAGES C CXX)

set(SILKIT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/.." CACHE PATH "SIL Kit source checkout")
get_filename_component(SILKIT_SOURCE_DIR "${SILKIT_SOURCE_DIR}" ABSOLUTE)
if(NOT EXISTS "${SILKIT_SOURCE_DIR}/SilKit/source/core/vasio/IRegistryEventListener.hpp")
    message(FATAL_ERROR "SILKIT_SOURCE_DIR must contain the internal IRegistryEventListener interface")
endif()

# Source dependency only: no installed SDK, dashboard client, or utility executable.
set(SILKIT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SILKIT_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(SILKIT_BUILD_UTILITIES OFF CACHE BOOL "" FORCE)
set(SILKIT_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(SILKIT_BUILD_DASHBOARD OFF CACHE BOOL "" FORCE)
add_subdirectory("${SILKIT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/sil-kit" EXCLUDE_FROM_ALL)

add_executable(registry-listener
    src/main.cpp
    src/RegistryEventListener.cpp
)
target_compile_features(registry-listener PRIVATE cxx_std_17)
set_target_properties(registry-listener PROPERTIES CXX_EXTENSIONS OFF)

# I_SilKit supplies private headers/dependencies; S_SilKitImpl supplies the implementation.
# Match the upstream registry utility's static implementation linkage, not the shared C API.
target_link_libraries(registry-listener PRIVATE I_SilKit S_SilKitImpl)
target_compile_definitions(registry-listener PRIVATE EXPORT_SilKitAPI)

option(REGISTRY_LISTENER_BUILD_TESTS "Build the participant fixture and enable the smoke test" OFF)
if(REGISTRY_LISTENER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
