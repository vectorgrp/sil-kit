function(get_external_project name)
    set(oneValueArgs URL URL_HASH SVN_REPOSITORY SVN_REVISION SOURCE_DIR BINARY_DIR WORKING_DIR)
    set(multiValueArgs INSTALL_COMMAND)
    cmake_parse_arguments(ext "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    if(NOT ext_SOURCE_DIR)
        set(ext_SOURCE_DIR ${CMAKE_BINARY_DIR}/${name})
    endif()
    if(NOT ext_BINARY_DIR)
        set(ext_BINARY_DIR ${CMAKE_BINARY_DIR}/${name})
    endif()

    set(PRO_NAME "${name}")
    set(URL ${ext_URL})
    set(URL_HASH ${ext_URL_HASH})
    set(SOURCE_DIR ${ext_SOURCE_DIR})
    set(BINARY_DIR ${ext_BINARY_DIR})
    set(INSTALL_COMMAND ${ext_INSTALL_COMMAND})
    #pass through additional arguments
    set(EXT_ARGS ${ext_UNPARSED_ARGUMENTS})
    configure_file(${CMAKE_SOURCE_DIR}/cmake/GetExternalProject-CMakeLists.txt.in ${ext_WORKING_DIR}/CMakeLists.txt)
    
    execute_process(
        COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${ext_WORKING_DIR}
    )
    if(result)
        message(FATAL_ERROR "CMake step for ${ext_project}-download failed: returncode=${result}")
    endif()
  
    execute_process(
        COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${ext_WORKING_DIR}
    )
    if(result)
      message(FATAL_ERROR "Build step for ${ext_project}-download failed: returncode=${result}")
    endif()

endfunction()
