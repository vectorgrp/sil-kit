if(${IB_BUILD_DOCS})
    find_package(Doxygen REQUIRED)
    find_package(Sphinx REQUIRED)

    # Find all the public headers
    get_target_property(IB_API_HEADERS IbApi SOURCES)

    set(DOXYGEN_INPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/../IntegrationBus/include/ib)
    set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
    set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/xml/index.xml)
    set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    #Replace variables inside @@ with the current values
    configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

    #Doxygen won't create this for us
    file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})

    add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
                       COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
                       DEPENDS ${IB_API_HEADERS}
                       MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
                       COMMENT "Generating docs"
                       VERBATIM)

    add_custom_target(Doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})

    set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
    set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/sphinx)
    set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

    # Only regenerate Sphinx when:
    # - Doxygen has rerun
    # - Our doc files have been updated
    # - The Sphinx config has been updated
    file(GLOB_RECURSE SPHINX_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.rst)
    add_custom_command(OUTPUT ${SPHINX_INDEX_FILE}
                       COMMAND
                           ${SPHINX_EXECUTABLE} -b html
                           # Tell Breathe where to find the Doxygen output
                           -Dbreathe_projects.IntegrationBus=${DOXYGEN_OUTPUT_DIR}/xml
                           ${SPHINX_SOURCE} ${SPHINX_BUILD}
                       WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                       DEPENDS
                       # Other docs files you want to track should go here (or in some variable)
                       ${SPHINX_SOURCE_FILES}
                       ${DOXYGEN_INDEX_FILE}
                       MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
                       COMMENT "Generating documentation with Sphinx")

    add_custom_target(Sphinx ALL DEPENDS ${SPHINX_INDEX_FILE})

    include(GNUInstallDirs)

    install(DIRECTORY ${SPHINX_BUILD}/
            DESTINATION ${CMAKE_INSTALL_DOCDIR})
    install(
        FILES
            CHANGELOG.rst
        DESTINATION ${CMAKE_INSTALL_PREFIX}
    )
endif()
