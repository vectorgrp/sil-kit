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

# local headers as interface lib
add_subdirectory(SimTestHarness)
add_subdirectory(Hourglass)

add_library(ITest_Utils INTERFACE)
target_include_directories(ITest_Utils
    INTERFACE ${CMAKE_CURRENT_LIST_DIR})

add_silkit_test(ITest_CatchExceptionsInCallbacks
    SOURCES
    ITest_CatchExceptionsInCallbacks.cpp

    LIBS
    SilKit
)

add_silkit_test(ITest_CatchExceptionsInCallbacks_Hourglass
    SOURCES
    ITest_CatchExceptionsInCallbacks.cpp

    LIBS
    SilKit_Hourglass
)

add_silkit_test(ITest_Internals_HopOnHopOff
    SOURCES
    ITest_Internals_HopOnHopOff.cpp
    LIBS
        SilKit
        I_SilKit_Config_TestUtils
        I_SilKit_Core_Internal
)

add_silkit_test(ITest_CommunicationReady
    SOURCES
    ITest_CommunicationReady.cpp

    LIBS
    SilKit
)

add_silkit_test(ITest_CommunicationReady_Hourglass
    SOURCES
    ITest_CommunicationReady.cpp

    LIBS
    SilKit_Hourglass
)

add_silkit_test(ITest_Internals_DataPubSub
    SOURCES
    ITest_Internals_DataPubSub.cpp

    LIBS
    SilKit
    I_SilKit_Core_Internal
    I_SilKit_Config_TestUtils
)

add_silkit_test(ITest_Internals_TargetedMessaging
    SOURCES
    ITest_Internals_TargetedMessaging.cpp

    LIBS
    S_SilKit_Tracing      # XXX
    S_SilKit_Services_Can # XXX
    I_SilKit_Core_Internal
    S_ITests_STH
)

add_silkit_test(ITest_StateMachineVAsio
    SOURCES
    ITest_StateMachineVAsio.cpp

    LIBS
    SilKit
)

add_silkit_test(ITest_DeterministicSimVAsio
    SOURCES
    ITest_DeterministicSimVAsio.cpp

    LIBS
    SilKit
)

# XXX Non-Internals Candidate
add_silkit_test(ITest_DifferentPeriods
    SOURCES
    ITest_DifferentPeriods.cpp

    LIBS
    SilKit
)

add_silkit_test(ITest_SingleParticipant
    SOURCES
    ITest_SingleParticipant.cpp

    LIBS
    S_ITests_STH
)

add_silkit_test(ITest_Internals_ServiceDiscovery
    SOURCES
      ITest_Internals_ServiceDiscovery.cpp
    LIBS
      I_SilKit_Core_Internal
      I_SilKit_Core_Participant
      I_SilKit_Core_Service
      I_SilKit_Core_VAsio
      I_SilKit_Services_Can
      I_SilKit_Services_Ethernet
      I_SilKit_Services_Flexray
      I_SilKit_Services_Lin
      I_SilKit_Services_Logging
      I_SilKit_Services_Orchestration
      I_SilKit_Services_PubSub
      I_SilKit_Services_Rpc
      S_ITests_STH
)

add_silkit_test(FTest_Internals_ServiceDiscoveryPerf
    SOURCES
      FTest_Internals_ServiceDiscoveryPerf.cpp
    LIBS
      I_SilKit_Core_Participant
      I_SilKit_Core_VAsio
      S_ITests_STH
)

add_silkit_test(ITest_Internals_SystemMonitor
    SOURCES
      ITest_Internals_SystemMonitor.cpp
    LIBS
      I_SilKit_Core_Internal
      I_SilKit_Core_Participant
      I_SilKit_Core_VAsio
      I_SilKit_Services_Can
      I_SilKit_Services_Ethernet
      I_SilKit_Services_Flexray
      I_SilKit_Services_Lin
      I_SilKit_Services_Logging
      I_SilKit_Services_Orchestration
      I_SilKit_Services_PubSub
      I_SilKit_Services_Rpc
      S_ITests_STH

      S_SilKit_Services_Orchestration
)

add_silkit_test(FTest_Internals_PubSubPerf
    SOURCES
      FTest_Internals_PubSubPerf.cpp
    LIBS
      I_SilKit_Core_Participant
      I_SilKit_Core_VAsio
      S_ITests_STH
)

add_silkit_test(ITest_AsyncSimTask
    SOURCES
      ITest_AsyncSimTask.cpp
    LIBS
      S_ITests_STH
)

add_silkit_test(ITest_AsyncSimTask_Hourglass
    SOURCES
    ITest_AsyncSimTask.cpp

    LIBS
    S_ITests_STH_Hourglass
)

# Windows specific tests
if(MSVC)
    add_silkit_test(ITest_CompatibilityWithWindowsHeaders
        SOURCES
        ITest_CompatibilityWithWindowsHeaders.cpp

        LIBS
        S_SilKit_Core_Participant
    )
endif()

# ============================================================
#  Integration Tests for Single Bus System
# ============================================================

# CAN

add_silkit_test(
    FTest_CanWithoutSync

    SOURCES FTest_CanWithoutSync.cpp

    LIBS
    SilKit
)

add_silkit_test(
    FTest_CanWithoutSync_Hourglass

    SOURCES FTest_CanWithoutSync.cpp

    LIBS
    SilKit_Hourglass
)

add_silkit_test(ITest_ThreeCanController
    SOURCES
    ITest_ThreeCanController.cpp

    LIBS
    S_ITests_STH
)

add_silkit_test(ITest_ThreeCanController_Hourglass
    SOURCES
    ITest_ThreeCanController.cpp

    LIBS
    S_ITests_STH_Hourglass
)

add_silkit_test(FTest_CanControllerThreadSafety
    SOURCES
    FTest_CanControllerThreadSafety.cpp

    LIBS
    SilKit
)

add_silkit_test(FTest_CanControllerThreadSafety_Hourglass
    SOURCES
    FTest_CanControllerThreadSafety.cpp

    LIBS
    SilKit_Hourglass
)

# Ethernet

add_silkit_test(ITest_ThreeEthController
    SOURCES
    ITest_ThreeEthController.cpp

    LIBS
    S_ITests_STH
)

add_silkit_test(ITest_ThreeEthController_Hourglass
    SOURCES
    ITest_ThreeEthController.cpp

    LIBS
    S_ITests_STH_Hourglass
)

add_silkit_test(FTest_EthWithoutSync
    SOURCES
    FTest_EthWithoutSync.cpp

    LIBS
    SilKit
)

add_silkit_test(FTest_EthWithoutSync_Hourglass
    SOURCES
    FTest_EthWithoutSync.cpp

    LIBS
    SilKit_Hourglass
)

# LIN

add_silkit_test(ITest_Lin
    SOURCES
    ITest_Lin.cpp

    LIBS
    S_ITests_STH
)

add_silkit_test(ITest_Lin_Hourglass
    SOURCES
    ITest_Lin.cpp

    LIBS
    S_ITests_STH_Hourglass
)

# RPC

add_silkit_test(ITest_Internals_Rpc
    SOURCES
    ITest_Internals_Rpc.cpp
    ITest_Internals_Rpc.hpp

    LIBS
    SilKit
    I_SilKit_Core_Internal
    I_SilKit_Config_TestUtils
)

# Hourglass:
# Currently impossible due to explicit use of the ServiceDiscovery, as it is
# retrieved via through a dynamic_cast to IParticipantInternal.

# ============================================================
#  Integration Tests based on Demo Applications
# ============================================================

# CAN

add_silkit_test(ITest_CanDemo
    SOURCES
    ITest_CanDemo.cpp

    LIBS
    S_ITests_STH
)

add_silkit_test(ITest_CanDemo_Hourglass
    SOURCES
    ITest_CanDemo.cpp

    LIBS
    S_ITests_STH_Hourglass
)

# Ethernet

add_silkit_test(ITest_EthernetDemo
    SOURCES
    ITest_EthernetDemo.cpp

    LIBS
    S_ITests_STH
)

add_silkit_test(ITest_EthernetDemo_Hourglass
    SOURCES
    ITest_EthernetDemo.cpp

    LIBS
    S_ITests_STH_Hourglass
)

# LIN

add_silkit_test(ITest_LinDemo
    SOURCES
    ITest_LinDemo.cpp

    LIBS
    S_ITests_STH
)

add_silkit_test(ITest_LinDemo_Hourglass
    SOURCES
    ITest_LinDemo.cpp

    LIBS
    S_ITests_STH_Hourglass
)
