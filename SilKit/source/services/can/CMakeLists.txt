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

add_library(SilKitSimCan STATIC
    CanDatatypesUtils.cpp
    CanDatatypesUtils.hpp
    CanController.cpp
    CanController.hpp
    ISimBehavior.hpp
    SimBehavior.cpp
    SimBehavior.hpp
    SimBehaviorDetailed.cpp
    SimBehaviorDetailed.hpp
    SimBehaviorTrivial.cpp
    SimBehaviorTrivial.hpp

    CanSerdes.hpp
    CanSerdes.cpp
)

target_include_directories(SilKitSimCan
    PUBLIC .
)

target_link_libraries(SilKitSimCan
    PUBLIC  SilKitWireCan
    PRIVATE SilKitInterface
            SilKitInternal
            SilKitTracing
            SilKitConfiguration
    INTERFACE SilKitInternal
            SilKitConfiguration
)

add_silkit_test(TestSimCanController
    SOURCES 
        CanControllerConfigTest.cpp
        CanControllerDetailedSimTest.cpp 
        CanControllerTrivialSimTest.cpp
    LIBS 
        SilKitSimCan
        SilKitMwMockParticipant
        SilKitTracing
        SilKitMwMockNullConnection
)

add_silkit_test(TestSimCan_Serdes  SOURCES CanSerdesTest.cpp LIBS SilKitInternal SilKitSimCan)

add_silkit_test(TestSimCan_StringUtils SOURCES CanStringUtilsTest.cpp LIBS SilKitInternal SilKitSimCan)
