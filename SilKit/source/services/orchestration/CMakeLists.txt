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

add_library(SilKitMwSync
    ILifecycleStates.hpp
    LifecycleManagement.hpp
    LifecycleManagement.cpp
    LifecycleStates.hpp
    LifecycleStates.cpp
    LifecycleService.hpp
    LifecycleService.cpp
    
    SyncDatatypeUtils.hpp
    SyncDatatypeUtils.cpp
    SystemController.hpp
    SystemController.cpp
    SystemMonitor.hpp
    SystemMonitor.cpp
    WatchDog.hpp
    WatchDog.cpp
    TimeSyncService.hpp
    TimeSyncService.cpp
    
    SyncSerdes.hpp
    SyncSerdes.cpp

    TimeProvider.hpp
    TimeProvider.cpp

    TimeConfiguration.hpp
    TimeConfiguration.cpp
)

target_include_directories(SilKitMwSync
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(SilKitMwSync
    PUBLIC 
        SilKitInterface
        SilKitUtil
        SilKitSetThreadName
        SilKitInternal
    PRIVATE 
        SilKitInternal
        SilKitMwLogging
        SilKitConfiguration
    INTERFACE
        SilKitInternal
        SilKitConfiguration
)

add_silkit_test(TestMwLifecycleService
    SOURCES LifecycleServiceTest.cpp 
    LIBS SilKitMwSync SilKitMwMockParticipant
)
add_silkit_test(TestMwSystemController 
    SOURCES SystemControllerTest.cpp 
    LIBS SilKitMwSync SilKitMwMockParticipant
)
add_silkit_test(TestMwSystemMonitor 
    SOURCES SystemMonitorTest.cpp 
    LIBS SilKitMwSync SilKitMwMockParticipant
)
add_silkit_test(TestMwWatchDog
    SOURCES WatchDogTest.cpp
    LIBS SilKitMwSync
)
add_silkit_test(TestMwSync_Serdes SOURCES SyncSerdesTest.cpp LIBS SilKitMwSync SilKitInternal)
add_silkit_test(Test_TimeProvider SOURCES TimeProviderTest.cpp LIBS SilKitMwSync SilKitMwMockParticipant)
add_silkit_test(Test_TimeSyncService SOURCES TimeSyncServiceTest.cpp LIBS SilKitMwSync SilKitMwMockParticipant)
