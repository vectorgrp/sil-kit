function(include_spdlog)
    # function() adds an additional scope
    # Temporarily patch some CMake variables to trick spdlog's install commands into using our install-folder layout
    set(CMAKE_INSTALL_LIBDIR "${INSTALL_LIB_DIR}")
    set(CMAKE_INSTALL_INCLUDEDIR  "${INSTALL_INCLUDE_DIR}")
    message(STATUS "Add subdirectory ThirdParty/spdlog")
    add_subdirectory(
        ${THIRDPARTY_DIR}/spdlog
        ${CMAKE_BINARY_DIR}/ThirdParty/spdlog
        EXCLUDE_FROM_ALL
    )
endfunction()

include_spdlog()
