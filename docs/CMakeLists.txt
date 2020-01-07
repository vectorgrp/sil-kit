if(${IB_BUILD_DOCS})
    Project(IBDocs NONE)
    include(IntegrationBusVersion)
    configure_ib_version(${PROJECT_NAME})
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

    set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
    set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/sphinx)
    set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

    # Only regenerate Sphinx when:
    # - Doxygen has rerun
    # - Our doc files have been updated
    # - The Sphinx config has been updated
    file(GLOB_RECURSE SPHINX_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.rst)

    add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
                       COMMENT "Generating docs"
                       MAIN_DEPENDENCY
                           ${DOXYFILE_OUT}
                           ${DOXYFILE_IN}
                           ${SPHINX_SOURCE}/conf.py
                       DEPENDS
                           ${IB_API_HEADERS}
                           ${SPHINX_SOURCE_FILES}
                       COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
                       COMMAND ${SPHINX_EXECUTABLE} -b html
                           # Tell Breathe where to find the Doxygen output
                           -Dbreathe_projects.IntegrationBus=${DOXYGEN_OUTPUT_DIR}/xml
                           -d${CMAKE_CURRENT_BINARY_DIR}/sphinx-doctree
                           ${SPHINX_SOURCE} ${SPHINX_BUILD}
                       WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                       VERBATIM
                       )

    add_custom_target(Doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})
    install(DIRECTORY ${SPHINX_BUILD}/
            DESTINATION ${INSTALL_DOC_DIR})
    install(
        FILES
            CHANGELOG.rst
        DESTINATION .
        COMPONENT IntegrationBus-Developer
    )
endif()
