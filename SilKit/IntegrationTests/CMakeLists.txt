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


target_link_libraries(SilKitIntegrationTests
    PRIVATE SilKit
    PRIVATE S_ITests_STH
)

target_link_libraries(SilKitInternalIntegrationTests
    PRIVATE S_SilKitImpl
    PRIVATE S_ITests_STH_Internals
)

target_link_libraries(SilKitFunctionalTests
    PRIVATE SilKit
    PRIVATE S_ITests_STH
)

target_link_libraries(SilKitInternalFunctionalTests
    PRIVATE S_SilKitImpl
    PRIVATE S_ITests_STH_Internals
)


add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_CatchExceptionsInCallbacks.cpp
)

add_silkit_test_to_executable(SilKitInternalIntegrationTests
    SOURCES ITest_HopOnHopOff.cpp
)

add_silkit_test_to_executable(SilKitInternalIntegrationTests
    SOURCES ITest_Abort.cpp
)

add_silkit_test_to_executable(SilKitInternalIntegrationTests
    SOURCES ITest_Internals_ParticipantModes.cpp
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_CommunicationGuarantees.cpp
)

add_silkit_test_to_executable(SilKitInternalIntegrationTests
    SOURCES ITest_Internals_DataPubSub.cpp
)

add_silkit_test_to_executable(SilKitInternalIntegrationTests
    SOURCES ITest_Internals_TargetedMessaging.cpp
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_StateMachineVAsio.cpp
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_DeterministicSimVAsio.cpp
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_DifferentPeriods.cpp
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_SingleParticipant.cpp
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_SameParticipants.cpp
)

add_silkit_test_to_executable(SilKitInternalIntegrationTests
    SOURCES ITest_Internals_ServiceDiscovery.cpp
)

add_silkit_test_to_executable(SilKitInternalIntegrationTests
    SOURCES ITest_Internals_RequestReply.cpp
)

add_silkit_test_to_executable(SilKitInternalFunctionalTests
    SOURCES FTest_ServiceDiscoveryPerf.cpp
)

add_silkit_test_to_executable(SilKitInternalIntegrationTests
    SOURCES ITest_SystemMonitor.cpp
)

add_silkit_test_to_executable(SilKitFunctionalTests
    SOURCES FTest_PubSubPerf.cpp
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_AsyncSimTask.cpp
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_SimTask.cpp
)

add_silkit_test_to_executable(SilKitFunctionalTests
    SOURCES FTest_WallClockCoupling.cpp
)

# Windows specific tests
if(MSVC)
    add_silkit_test_to_executable(SilKitInternalIntegrationTests
        SOURCES ITest_CompatibilityWithWindowsHeaders.cpp
    )
endif()

# ============================================================
#  Integration Tests for Single Bus System
# ============================================================

add_silkit_test_to_executable(SilKitFunctionalTests
    SOURCES FTest_VendorVectorRegistry.cpp
)


# CAN

add_silkit_test_to_executable(SilKitFunctionalTests
    SOURCES FTest_CanWithoutSync.cpp
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_ThreeCanController.cpp
)

add_silkit_test_to_executable(SilKitFunctionalTests
    SOURCES FTest_CanControllerThreadSafety.cpp
)

# Ethernet

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_ThreeEthController.cpp
)

add_silkit_test_to_executable(SilKitFunctionalTests
    SOURCES FTest_EthWithoutSync.cpp
)

# LIN

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_Lin.cpp
)
add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_LinDynamicResponse.cpp
)

# RPC

add_silkit_test_to_executable(SilKitInternalIntegrationTests
    SOURCES ITest_Internals_Rpc.cpp ITest_Internals_Rpc.hpp
)

# ============================================================
#  Integration Tests for Network Simulator API
# ============================================================

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_NetSimCan.cpp
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_NetSimEthernet.cpp
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_NetSimLin.cpp
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_NetSimFlexRay.cpp
)


# ============================================================
#  Integration Tests based on Demo Applications
# ============================================================

# CAN

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_CanDemo.cpp
)

# Ethernet

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_EthernetDemo.cpp
)

# LIN

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_LinDemo.cpp
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_Orchestration.cpp
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_MessageAggregation.cpp
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_RequestRemoteParticipantConnect.cpp
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_MultiSim.cpp
)

if(SILKIT_BUILD_DASHBOARD)
    add_silkit_test_to_executable(SilKitInternalIntegrationTests
        SOURCES ITest_Dashboard.cpp
        LIBS O_SilKit_Dashboard
    )

    add_silkit_test_to_executable(SilKitInternalIntegrationTests
        SOURCES ITest_DashboardServerUnavailable.cpp
        LIBS O_SilKit_Dashboard
    )

    add_silkit_test_to_executable(SilKitInternalIntegrationTests
        SOURCES ITest_DashboardServerTimeout.cpp
        LIBS O_SilKit_Dashboard
    )
endif()
