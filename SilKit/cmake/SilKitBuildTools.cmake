# SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

include(CMakeFindBinUtils)

macro(silkit_split_debugsymbols targetName)
    set(_targetFile "$<TARGET_FILE:${targetName}>")
    set(_debugFile "${SILKIT_SYMBOLS_DIR}/${targetName}${CMAKE_DEBUG_POSTFIX}.debug")
    message(DEBUG "Split debugsymbols from ${_targetName} into ${_debugFile}")
    add_custom_command(
        TARGET ${targetName}
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} --only-keep-debug "${_targetFile}" "${_debugFile}"
        COMMAND ${CMAKE_STRIP} "${_targetFile}"
        COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink="${_debugFile}" "${_targetFile}"
        COMMENT "SIL Kit: SILKIT_PACKAGE_SYMBOLS: splitting ELF debug symbols"
    )
endmacro()
macro(silkit_strip targetName)
    set(_targetFile "$<TARGET_FILE:${targetName}>")
    add_custom_command(
        TARGET ${targetName}
        POST_BUILD
        COMMAND ${CMAKE_STRIP} "${_targetFile}"
        COMMENT "SIL Kit: stripping ${_targetName}"
    )
endmacro()

macro(silkit_install_debugsymbols targetName)
    if(MSVC)
	message(STATUS "Installing .pdb symbols file")
	install(
		FILES $<TARGET_PDB_FILE:SilKit>
		DESTINATION ${INSTALL_LIB_DIR}
		COMPONENT bin
	)
    endif()
    if(APPLE)
        return()
    endif()
    if(UNIX)
        get_target_property(targetType ${targetName} TYPE)
        if(targetType STREQUAL STATIC_LIBRARY)
            message(STATUS "SIL Kit: splitting debug symbols on static libraries is not supported")
            return()
        endif()

        silkit_split_debugsymbols("${targetName}")

        file(MAKE_DIRECTORY "${SILKIT_SYMBOLS_DIR}")

	message(STATUS "Installing ELF debug file")
	install(
		FILES "${SILKIT_SYMBOLS_DIR}/${targetName}${CMAKE_DEBUG_POSTFIX}.debug"
		DESTINATION ${INSTALL_LIB_DIR}
		COMPONENT bin
	)
    endif()
endmacro()

