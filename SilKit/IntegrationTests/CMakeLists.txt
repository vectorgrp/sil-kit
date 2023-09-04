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

# local headers as interface lib
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

add_silkit_test(ITest_Internals_HopOnHopOff
    SOURCES
    ITest_Internals_HopOnHopOff.cpp

    LIBS
    S_SilKitImpl
)

add_silkit_test(ITest_Internals_Abort
    SOURCES
    ITest_Internals_Abort.cpp

    LIBS
    S_SilKitImpl
)

add_silkit_test(ITest_Internals_ParticipantModes
    SOURCES
    ITest_Internals_ParticipantModes.cpp

    LIBS
    S_SilKitImpl
)

add_silkit_test(ITest_CommunicationGuarantees
    SOURCES
    ITest_CommunicationGuarantees.cpp

    LIBS
    SilKit
)

add_silkit_test(ITest_Internals_DataPubSub
    SOURCES
    ITest_Internals_DataPubSub.cpp

    LIBS
    S_SilKitImpl
)

add_silkit_test(ITest_Internals_TargetedMessaging
    SOURCES
    ITest_Internals_TargetedMessaging.cpp

    LIBS
    S_ITests_STH_Internals
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

add_silkit_test(ITest_SameParticipants
    SOURCES
    ITest_SameParticipants.cpp

    LIBS
    S_ITests_STH
)

add_silkit_test(ITest_Internals_ServiceDiscovery
    SOURCES
      ITest_Internals_ServiceDiscovery.cpp
    LIBS
      S_ITests_STH_Internals
)

add_silkit_test(ITest_Internals_RequestReply
    SOURCES
      ITest_Internals_RequestReply.cpp
    LIBS
      S_SilKitImpl
)

add_silkit_test(FTest_Internals_ServiceDiscoveryPerf
    SOURCES
      FTest_Internals_ServiceDiscoveryPerf.cpp
    LIBS
      S_ITests_STH_Internals
)

add_silkit_test(ITest_Internals_SystemMonitor
    SOURCES
      ITest_Internals_SystemMonitor.cpp
    LIBS
      S_ITests_STH_Internals
)

add_silkit_test(FTest_PubSubPerf
    SOURCES
      FTest_Internals_PubSubPerf.cpp
    LIBS
      S_ITests_STH
)

add_silkit_test(FTest_Internals_PubSubPerf
    SOURCES
      FTest_Internals_PubSubPerf.cpp
    LIBS
      I_SilKit_Core_Participant
      I_SilKit_Core_VAsio
      S_ITests_STH_Internals
)

add_silkit_test(ITest_AsyncSimTask
    SOURCES
      ITest_AsyncSimTask.cpp
    LIBS
      S_ITests_STH
)

add_silkit_test(ITest_SimTask
    SOURCES
    ITest_SimTask.cpp

    LIBS
      SilKit
)

# Windows specific tests
if(MSVC)
    add_silkit_test(ITest_CompatibilityWithWindowsHeaders
        SOURCES
        ITest_CompatibilityWithWindowsHeaders.cpp

        LIBS
        S_SilKitImpl
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

add_silkit_test(ITest_ThreeCanController
    SOURCES
    ITest_ThreeCanController.cpp

    LIBS
    S_ITests_STH
)

add_silkit_test(FTest_CanControllerThreadSafety
    SOURCES
    FTest_CanControllerThreadSafety.cpp

    LIBS
    SilKit
)

# Ethernet

add_silkit_test(ITest_ThreeEthController
    SOURCES
    ITest_ThreeEthController.cpp

    LIBS
    S_ITests_STH
)

add_silkit_test(FTest_EthWithoutSync
    SOURCES
    FTest_EthWithoutSync.cpp

    LIBS
    SilKit
)

# LIN

add_silkit_test(ITest_Lin
    SOURCES
    ITest_Lin.cpp

    LIBS
    S_ITests_STH
)
add_silkit_test(ITest_LinDynamicResponse
    SOURCES
    ITest_LinDynamicResponse.cpp

    LIBS
    S_ITests_STH
)

# RPC

add_silkit_test(ITest_Internals_Rpc
    SOURCES
    ITest_Internals_Rpc.cpp
    ITest_Internals_Rpc.hpp

    LIBS
    S_SilKitImpl
    I_SilKit_Core_Internal
    I_SilKit_Config_TestUtils
)

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

add_silkit_test(ITest_CanDemo_Internals
    SOURCES
    ITest_CanDemo.cpp

    LIBS
    S_ITests_STH_Internals
)

# Ethernet

add_silkit_test(ITest_EthernetDemo
    SOURCES
    ITest_EthernetDemo.cpp

    LIBS
    S_ITests_STH
)

add_silkit_test(ITest_EthernetDemo_Internals
    SOURCES
    ITest_EthernetDemo.cpp

    LIBS
    S_ITests_STH_Internals
)

# LIN

add_silkit_test(ITest_LinDemo
    SOURCES
    ITest_LinDemo.cpp

    LIBS
    S_ITests_STH
)

add_silkit_test(ITest_LinDemo_Internals
    SOURCES
    ITest_LinDemo.cpp

    LIBS
    S_ITests_STH_Internals
)

add_silkit_test(ITest_Orchestration
    SOURCES
    ITest_Orchestration.cpp

    LIBS
    S_ITests_STH
)

add_silkit_test(ITest_RequestRemoteParticipantConnect
    SOURCES
    ITest_RequestRemoteParticipantConnect.cpp

    LIBS
    S_ITests_STH
)

if(SILKIT_BUILD_DASHBOARD)
    add_silkit_test(
        ITest_Dashboard

        SOURCES ITest_Dashboard.cpp

        LIBS
        S_ITests_STH_Internals
        S_SilKit_Dashboard
        I_SilKit_Core_Internal
        I_SilKit_Config_TestUtils
        I_SilKit_Services_Ethernet
    )

    add_silkit_test(
        ITest_DashboardServerUnavailable

        SOURCES ITest_DashboardServerUnavailable.cpp

        LIBS
        S_ITests_STH_Internals
        S_SilKit_Dashboard
    )

    add_silkit_test(
        ITest_DashboardServerTimeout

        SOURCES ITest_DashboardServerTimeout.cpp

        LIBS
        S_ITests_STH_Internals
        S_SilKit_Dashboard
    )
endif()
