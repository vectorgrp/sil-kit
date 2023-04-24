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

# The Participant's connection interfaces are collected in this folder
# for easy consumption from external projects
add_library(I_SilKit_Core_Internal INTERFACE)
target_include_directories(I_SilKit_Core_Internal
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    # for internal type definitions
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../service # ServiceDiscovery is internal only special
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../requests # RequestReply is internal 
)
target_link_libraries(I_SilKit_Core_Internal
    INTERFACE SilKitInterface
    INTERFACE I_SilKit_Wire_Util
    INTERFACE I_SilKit_Wire_Can
    INTERFACE I_SilKit_Wire_Data
    INTERFACE I_SilKit_Wire_Ethernet
    INTERFACE I_SilKit_Wire_Flexray
    INTERFACE I_SilKit_Wire_Lin
    INTERFACE I_SilKit_Wire_Rpc
    INTERFACE I_SilKit_Config
    #for network simulator exports
    INTERFACE I_SilKit_Services_Can
    INTERFACE I_SilKit_Services_Lin
    INTERFACE I_SilKit_Services_Flexray
    INTERFACE I_SilKit_Services_Ethernet
)

add_silkit_test(Test_MwVAsio_MessageBuffer SOURCES Test_MessageBuffer.cpp LIBS I_SilKit_Core_Internal)
add_silkit_test(Test_MwInternal_Serdes  SOURCES Test_InternalSerdes.cpp LIBS I_SilKit_Core_Internal)
