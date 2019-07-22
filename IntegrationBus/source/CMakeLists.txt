add_subdirectory(cfg)
add_subdirectory(mw)
add_subdirectory(sim)
add_subdirectory(util)

set(GIT_DIR "${PROJECT_SOURCE_DIR}/../.git")
set(GIT_HEAD_FILE "${GIT_DIR}/HEAD")
if(EXISTS "${GIT_HEAD_FILE}")
    configure_file(
        "MakeVersionMacros.cmake.in"
        ${CMAKE_CURRENT_BINARY_DIR}/MakeVersionMacros.cmake
        @ONLY)
    include(${CMAKE_CURRENT_BINARY_DIR}/MakeVersionMacros.cmake)
else()
    message(WARNING "Cannot determine hash of current git head!")
    set(GIT_HEAD_HASH "UNKNOWN")
    configure_file(
        version_macros.hpp.in
        ${CMAKE_CURRENT_BINARY_DIR}/version_macros.hpp
        @ONLY)
endif()

add_library(IntegrationBus SHARED
    IntegrationBus.hpp
    IntegrationBus.cpp
    IbForceIncludes.cpp
    version.cpp
    version.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/version_macros.hpp
)

if (MSVC)
    target_sources(IntegrationBus PRIVATE IntegrationBus.rc)
endif (MSVC)


include_directories(${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(IntegrationBus
    PUBLIC  IbInterface
    PRIVATE IbConfig
    PRIVATE IbMwComAdapter
)
set_target_properties(IntegrationBus
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/
)

#######################################################################################################################
# Add to install targets of the IntegrationBus
#######################################################################################################################

# Mark the IntegrationBus sources for export by adding it to the target container IntegrationBusTargets
install(
    TARGETS IntegrationBus
    EXPORT IntegrationBusTargets
    DESTINATION ${INSTALL_LIBDIR}
    #LIBRARY DESTINATION ${INSTALL_LIBDIR} # Destination for Linux shared object libraries (.so)
    #ARCHIVE DESTINATION ${INSTALL_LIBDIR} # Destination for static libraries (.lib/.a)
    #RUNTIME DESTINATION ${INSTALL_LIBDIR} # Destination for executables and dynamic libraries (.dll)
    PUBLIC_HEADER DESTINATION ${INSTALL_INCLUDEDIR} # Destination for header files
    COMPONENT IntegrationBus
)
# Note: When we use multiple destinations in a single command, all are installed but only the first is packaged.
#       See https://cmake.org/pipermail/cmake/2009-November/033309.html, this being the proposed fix/workaround.
if(MSVC)
    # Install the Windows import library (.lib) and the dynamic library itself (.dll)
    install(
        TARGETS IntegrationBus
        DESTINATION ${INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${INSTALL_LIBDIR} # Destination for static libraries (.lib/.a)
        COMPONENT IntegrationBus
    )
    install(
        TARGETS IntegrationBus
        DESTINATION ${INSTALL_LIBDIR}
        RUNTIME DESTINATION ${INSTALL_LIBDIR} # Destination for executables and dynamic libraries (.dll)
        COMPONENT IntegrationBus
    )
    if(IB_INSTALL_PDB_FILES)
        install(
            FILES $<TARGET_PDB_FILE:IntegrationBus>
            CONFIGURATIONS Debug RelWithDebInfo
            DESTINATION ${INSTALL_LIBDIR}
            COMPONENT IntegrationBus-Developer
        )
    endif()
else()
    # Install the Linux shared object library (.so)
    install(
        TARGETS IntegrationBus
        DESTINATION ${INSTALL_LIBDIR}
        LIBRARY DESTINATION ${INSTALL_LIBDIR} # Destination for Linux shared object libraries (.so)
        COMPONENT IntegrationBus
    )
endif()
