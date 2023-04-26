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

add_library(SilKitInterface INTERFACE)

# Use generator expression to have a different set of include paths when the target is built versus when the target is installed somewhere
target_include_directories(SilKitInterface
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
        $<INSTALL_INTERFACE:${INSTALL_INCLUDE_DIR}>
)

# Add a custom project SilKitApi for quick access to SIL Kit Headers in Visual Studio
file(GLOB_RECURSE SILKIT_INTERFACE_SOURCES ${CMAKE_CURRENT_LIST_DIR}/*.hpp ${CMAKE_CURRENT_LIST_DIR}/*.ipp)
add_custom_target(SilKitApi SOURCES ${SILKIT_INTERFACE_SOURCES})
if(CMAKE_VERSION VERSION_GREATER 3.8.0)
    # Easy access to SIL Kit includes for Visual Studio
    source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/silkit PREFIX "silkit" FILES ${SILKIT_INTERFACE_SOURCES})
endif()

################################################################################
# add to install targets of the SIL Kit
################################################################################
# Mark the SIL Kit headers for export by adding it to the target container
# SilKitTargets
install(
    TARGETS SilKitInterface
    EXPORT SilKitTargets
    COMPONENT bin
    INCLUDES DESTINATION ${INSTALL_INCLUDE_DIR} # Destination for header files
)
# Copy all headers from the source directory to the proper destination
install(
    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/
    DESTINATION ${INSTALL_INCLUDE_DIR}
    COMPONENT bin
    FILES_MATCHING
        PATTERN *.hpp
        PATTERN *.ipp
)
# for C API
install(
    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/
    DESTINATION ${INSTALL_INCLUDE_DIR}
    COMPONENT bin
    FILES_MATCHING PATTERN *.h
)
