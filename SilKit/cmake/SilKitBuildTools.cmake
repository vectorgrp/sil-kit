include(CMakeFindBinUtils)

macro(silkit_split_debugsymbols targetName)
    set(_targetFile "$<TARGET_FILE:${targetName}>")
    set(_debugFile "${SILKIT_SYMBOLS_DIR}/${targetName}${CMAKE_DEBUG_POSTFIX}.debug")
    add_custom_command(
        TARGET ${targetName}
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} --only-keep-debug "${_targetFile}" "${_debugFile}"
        COMMAND ${CMAKE_STRIP} "${_targetFile}"
        COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink="${_debugFile}" "${_targetFile}"
        COMMENT "SIL Kit: SILKIT_PACKAGE_SYMBOLS: splitting ELF debug symbols"
    )
endmacro()


macro(silkit_package_debugsymbols targetName)
    if(MSVC)
        add_custom_command(
            TARGET "${targetName}"
            POST_BUILD
            WORKING_DIRECTORY "${SILKIT_SYMBOLS_DIR_BASE}"
            COMMAND "${CMAKE_COMMAND}" -E tar cf "${SILKIT_SYMBOLS_DIR_NAME}.zip" --format=zip -- "${SILKIT_SYMBOLS_DIR_NAME}"
            COMMENT "SIL Kit: SILKIT_PACKAGE_SYMBOLS: creating PDB zip"
        )
    endif()
    if(APPLE)
        return()
    endif()
    if(UNIX AND CMAKE_BUILD_TYPE MATCHES "Debug")

        silkit_split_debugsymbols("${targetName}")

        file(MAKE_DIRECTORY "${SILKIT_SYMBOLS_DIR}")

        add_custom_command(
            TARGET "${targetName}"
            POST_BUILD
            WORKING_DIRECTORY "${SILKIT_SYMBOLS_DIR_BASE}"
            COMMAND "${CMAKE_COMMAND}" -E tar cf "${SILKIT_SYMBOLS_DIR_NAME}.zip" --format=zip -- "${SILKIT_SYMBOLS_DIR_NAME}"
            COMMENT "SIL Kit: SILKIT_PACKAGE_SYMBOLS: creating ELF debug symbols zip"
        )
    endif()
endmacro()
