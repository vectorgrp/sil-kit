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

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES
    ITest_CatchExceptionsInCallbacks.cpp

    LIBS
    SilKit
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_HopOnHopOff.cpp
    LIBS S_SilKitImpl
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_Abort.cpp
    LIBS S_SilKitImpl
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_Internals_ParticipantModes.cpp
    LIBS S_SilKitImpl
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES
    ITest_CommunicationGuarantees.cpp

    LIBS
    SilKit
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES
    ITest_Internals_DataPubSub.cpp

    LIBS
    S_SilKitImpl
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_Internals_TargetedMessaging.cpp
    LIBS S_ITests_STH_Internals
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES
    ITest_StateMachineVAsio.cpp

    LIBS
    SilKit
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES
    ITest_DeterministicSimVAsio.cpp

    LIBS
    SilKit
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES
    ITest_DifferentPeriods.cpp

    LIBS
    SilKit
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES
    ITest_SingleParticipant.cpp

    LIBS
    S_ITests_STH
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES
    ITest_SameParticipants.cpp

    LIBS
    S_ITests_STH
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_Internals_ServiceDiscovery.cpp
    LIBS S_ITests_STH_Internals
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_Internals_RequestReply.cpp
    LIBS S_SilKitImpl
)

add_silkit_test_to_executable(SilKitFunctionalTests
    SOURCES FTest_ServiceDiscoveryPerf.cpp
    LIBS S_ITests_STH_Internals
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_SystemMonitor.cpp
    LIBS S_ITests_STH_Internals
)

add_silkit_test_to_executable(SilKitFunctionalTests
    SOURCES FTest_PubSubPerf.cpp
    LIBS S_ITests_STH
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES
    ITest_AsyncSimTask.cpp

    LIBS
    S_ITests_STH
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES
    ITest_SimTask.cpp

    LIBS
    SilKit
)

# Windows specific tests
if(MSVC)
    add_silkit_test_to_executable(SilKitIntegrationTests
        SOURCES
        ITest_CompatibilityWithWindowsHeaders.cpp

        LIBS
        S_SilKitImpl
    )
endif()

# ============================================================
#  Integration Tests for Single Bus System
# ============================================================

add_silkit_test_to_executable(SilKitFunctionalTests
    SOURCES
    FTest_VendorVectorRegistry.cpp

    LIBS
    SilKit
)


# CAN

add_silkit_test_to_executable(SilKitFunctionalTests
    SOURCES
    FTest_CanWithoutSync.cpp

    LIBS
    SilKit
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES
    ITest_ThreeCanController.cpp

    LIBS
    S_ITests_STH
)

add_silkit_test_to_executable(SilKitFunctionalTests
    SOURCES FTest_CanControllerThreadSafety.cpp
    LIBS SilKit
)

# Ethernet

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES
    ITest_ThreeEthController.cpp

    LIBS
    S_ITests_STH
)

add_silkit_test_to_executable(SilKitFunctionalTests
    SOURCES
    FTest_EthWithoutSync.cpp

    LIBS
    SilKit
)

# LIN

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES
    ITest_Lin.cpp

    LIBS
    S_ITests_STH
)
add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES
    ITest_LinDynamicResponse.cpp

    LIBS
    S_ITests_STH
)

# RPC

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES
    ITest_Internals_Rpc.cpp
    ITest_Internals_Rpc.hpp

    LIBS
    S_SilKitImpl
    I_SilKit_Core_Internal
    I_SilKit_Config_TestUtils
)

# ============================================================
#  Integration Tests for NetworkSimulaton
# ============================================================

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES
    ITest_NetSimCan.cpp
    ITest_NetSim.hpp

    LIBS
    S_ITests_STH
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES
    ITest_NetSimEthernet.cpp
    ITest_NetSim.hpp

    LIBS
    S_ITests_STH
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES
    ITest_NetSimLin.cpp
    ITest_NetSim.hpp

    LIBS
    S_ITests_STH
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES
    ITest_NetSimFlexRay.cpp
    ITest_NetSim.hpp

    LIBS
    S_ITests_STH
)


# ============================================================
#  Integration Tests based on Demo Applications
# ============================================================

# CAN

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES
    ITest_CanDemo.cpp

    LIBS
    S_ITests_STH
)

# Ethernet

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES
    ITest_EthernetDemo.cpp

    LIBS
    S_ITests_STH
)

# LIN

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES
    ITest_LinDemo.cpp

    LIBS
    S_ITests_STH
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES
    ITest_Orchestration.cpp

    LIBS
    S_ITests_STH
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES
    ITest_RequestRemoteParticipantConnect.cpp

    LIBS
    S_ITests_STH
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES
    ITest_MultiSim.cpp

    LIBS
    S_ITests_STH
)

if(SILKIT_BUILD_DASHBOARD)
    add_silkit_test_to_executable(SilKitIntegrationTests
        SOURCES
        ITest_Dashboard.cpp

        LIBS
        S_ITests_STH_Internals
        O_SilKit_Dashboard
        I_SilKit_Core_Internal
        I_SilKit_Config_TestUtils
        I_SilKit_Services_Ethernet
    )

    add_silkit_test_to_executable(SilKitIntegrationTests
        SOURCES
        ITest_DashboardServerUnavailable.cpp

        LIBS
        S_ITests_STH_Internals
        O_SilKit_Dashboard
    )

    add_silkit_test_to_executable(SilKitIntegrationTests
        SOURCES
        ITest_DashboardServerTimeout.cpp

        LIBS
        S_ITests_STH_Internals
        O_SilKit_Dashboard
    )
endif()
