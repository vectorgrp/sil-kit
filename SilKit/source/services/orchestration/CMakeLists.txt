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


add_library(I_SilKit_Services_Orchestration INTERFACE)

target_include_directories(I_SilKit_Services_Orchestration
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(I_SilKit_Services_Orchestration
    INTERFACE SilKitInterface

    INTERFACE I_SilKit_Util
    INTERFACE I_SilKit_Util_SetThreadName
    INTERFACE I_SilKit_Core_Internal
    INTERFACE I_SilKit_Config
)


add_library(O_SilKit_Services_Orchestration OBJECT
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

    SystemStateTracker.cpp
)

target_link_libraries(O_SilKit_Services_Orchestration
    PUBLIC I_SilKit_Services_Orchestration

    PRIVATE I_SilKit_Services_Logging
    PRIVATE I_SilKit_Experimental
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_LifecycleService.cpp 
    LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant
)
add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_SystemController.cpp 
    LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant
)
add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_SystemMonitor.cpp 
    LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant
)
add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_WatchDog.cpp
    LIBS S_SilKitImpl
)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_SyncSerdes.cpp LIBS S_SilKitImpl I_SilKit_Core_Internal)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_TimeProvider.cpp LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant)
add_silkit_test_to_executable(SilKitUnitTests SOURCES Test_TimeSyncService.cpp LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant)
