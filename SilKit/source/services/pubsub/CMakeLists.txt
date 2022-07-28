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

add_library(SilKitSimDataMessage STATIC
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

target_include_directories(SilKitSimDataMessage
    PUBLIC .
)

target_link_libraries(SilKitSimDataMessage
    PUBLIC  SilKitWireData
    PRIVATE SilKitInterface
            SilKitSetThreadName
            SilKitInternal
            SilKitConfiguration
            SilKitUtil
)

add_silkit_test(Test_SimDataPublisher 
    SOURCES Test_DataPublisher.cpp 
    LIBS SilKitSimDataMessage SilKitMwMockParticipant SilKitInternal 
)
add_silkit_test(Test_SimDataSubscriber
    SOURCES Test_DataSubscriber.cpp
    LIBS SilKitSimDataMessage SilKitMwMockParticipant SilKitInternal 
)
add_silkit_test(Test_SimDataSubscriberInternal
    SOURCES Test_DataSubscriberInternal.cpp
    LIBS SilKitSimDataMessage SilKitMwMockParticipant SilKitInternal
)
add_silkit_test(Test_PubSubMatching
    SOURCES Test_PubSubMatching.cpp 
    LIBS SilKitSimDataMessage SilKitUtil
)
add_silkit_test(Test_SimData_Serdes SOURCES Test_DataSerdes.cpp LIBS SilKitSimDataMessage SilKitInternal)
