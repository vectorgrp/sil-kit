# Copyright (c) 2022 Vector Informatik GmbH
# 
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


Project(sil-kit-docs  NONE)
include(SilKitVersion)
configure_silkit_version(${PROJECT_NAME})
find_package(Doxygen REQUIRED)

# Find all the public headers
get_target_property(SILKIT_API_HEADERS SilKitApi SOURCES)

set(DOXYGEN_INPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/../SilKit/include/silkit)
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/xml/index.xml)
set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

if (${SILKIT_WARNINGS_AS_ERRORS})
  set(SILKIT_DOXYGEN_WARN_AS_ERROR YES)
else()
  set(SILKIT_DOXYGEN_WARN_AS_ERROR NO)
endif()

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
file(GLOB_RECURSE SVG_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.svg)

if (UNIX)
  if (${SILKIT_WARNINGS_AS_ERRORS})
    set(SILKIT_SPHINX_WARNINGS_AS_ERRORS_ARGUMENTS "-W;--keep-going")
  else()
    set(SILKIT_SPHINX_WARNINGS_AS_ERRORS_ARGUMENTS "")
  endif()
endif()

add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
                   COMMENT "Generating docs"
                   MAIN_DEPENDENCY
                       ${DOXYFILE_OUT}
                       ${DOXYFILE_IN}
                       ${SPHINX_SOURCE}/conf.py
                   DEPENDS
                       ${SILKIT_API_HEADERS}
                       ${SPHINX_SOURCE_FILES}
                       ${SVG_FILES}
                   COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
                   # generate docs
                   COMMAND sphinx-build -b html
                       # Tell Breathe where to find the Doxygen output
                       -Dbreathe_projects.SilKit=${DOXYGEN_OUTPUT_DIR}/xml
                       -Drelease=${PROJECT_VERSION}
                       -Dversion=${PROJECT_VERSION}
                       -d${CMAKE_CURRENT_BINARY_DIR}/sphinx-doctree
                       -T
                       -w ${CMAKE_CURRENT_BINARY_DIR}/sphinx-build-error.log
                       -E
                       ${SILKIT_SPHINX_WARNINGS_AS_ERRORS_ARGUMENTS}
                       ${SPHINX_SOURCE} ${SPHINX_BUILD}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
                   VERBATIM
                   )

add_custom_target(Doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})
install(DIRECTORY ${SPHINX_BUILD}/
        DESTINATION ${INSTALL_DOC_DIR}
        COMPONENT docs
        REGEX "\.venv$" EXCLUDE
        REGEX "\.buildinfo$" EXCLUDE
        )
install(
    FILES
        CHANGELOG.rst
    DESTINATION .
    COMPONENT docs
    )
#add sample code so it shows in the IDE, but don't build or install it
add_executable(sample_silkit_new_config EXCLUDE_FROM_ALL usage/sample_silkit/simple.cpp)
target_link_libraries(sample_silkit_new_config PRIVATE SilKit)
