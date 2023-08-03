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


add_library(I_SilKit_Services_PubSub INTERFACE)

target_include_directories(I_SilKit_Services_PubSub
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(I_SilKit_Services_PubSub
    INTERFACE SilKitInterface

    INTERFACE I_SilKit_Core_Internal
    INTERFACE I_SilKit_Core_Participant
    INTERFACE I_SilKit_Tracing
    INTERFACE I_SilKit_Wire_Data
)


add_library(O_SilKit_Services_PubSub OBJECT
    DataMessageDatatypeUtils.hpp
    DataMessageDatatypeUtils.cpp
    DataPublisher.hpp
    DataPublisher.cpp
    DataSubscriber.hpp
    DataSubscriber.cpp

    DataSubscriberInternal.hpp
    DataSubscriberInternal.cpp

    DataSerdes.hpp
    DataSerdes.cpp
)

target_link_libraries(O_SilKit_Services_PubSub
    PUBLIC I_SilKit_Services_PubSub

    PRIVATE SilKitInterface
    PRIVATE I_SilKit_Core_Internal
    PRIVATE I_SilKit_Config
    PRIVATE I_SilKit_Util
    PRIVATE I_SilKit_Util_LabelMatching
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_DataPublisher.cpp 
    LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant
)
add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_DataSubscriber.cpp
    LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant
)
add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_DataSubscriberInternal.cpp
    LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant
)
add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_PubSubMatching.cpp 
    LIBS S_SilKitImpl
)
add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_DataSerdes.cpp
    LIBS S_SilKitImpl
)

