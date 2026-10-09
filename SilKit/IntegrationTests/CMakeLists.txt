# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

# Without an enclosing SIL Kit build, the tests using only the public API are built against a
# prebuilt SIL Kit package: cmake -S SilKit/IntegrationTests -B <dir> -DSILKIT_DIR=<dir of SilKitConfig.cmake>
if(NOT TARGET SilKit)
    cmake_minimum_required(VERSION 3.13)
    project("SilKit-IntegrationTests" LANGUAGES CXX C)

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_STANDARD_EXTENSIONS OFF)

    option(SILKIT_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

    find_package(SilKit REQUIRED CONFIG HINTS "${SILKIT_DIR}")
    message(STATUS "SIL Kit integration tests: using SIL Kit ${SilKit_VERSION} from ${SilKit_DIR}")

    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake")
    include(SilKitBuildSettings)
    include(SilKitTest)
    enable_testing()
    silkit_clean_default_compileflags()

    set(SILKIT_BUILD_TESTS ON)
    add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../../ThirdParty" "${CMAKE_CURRENT_BINARY_DIR}/ThirdParty")
    include_gtest()
    silkit_enable_warnings(${SILKIT_WARNINGS_AS_ERRORS})

    set(_silkit_itests_standalone TRUE)
else()
    set(_silkit_itests_standalone FALSE)
endif()

add_silkit_test_executable(SilKitIntegrationTests LABEL integration)
add_silkit_test_executable(SilKitFunctionalTests LABEL functional)

# local headers as interface lib
add_subdirectory(SimTestHarness)

# local headers as interface lib
add_subdirectory(Hourglass)

target_link_libraries(SilKitIntegrationTests
    PRIVATE SilKit::SilKit
    PRIVATE S_ITests_STH
)

target_link_libraries(SilKitFunctionalTests
    PRIVATE SilKit::SilKit
    PRIVATE S_ITests_STH
)

if(_silkit_itests_standalone AND WIN32)
    foreach(testExecutable SilKitIntegrationTests SilKitFunctionalTests)
        add_custom_command(TARGET ${testExecutable} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:SilKit::SilKit> $<TARGET_FILE_DIR:${testExecutable}>
        )
    endforeach()
endif()


add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_CatchExceptionsInCallbacks.cpp
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_HopOnHopOff.cpp
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_Abort.cpp
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_CommunicationGuarantees.cpp
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_LabelsMatching.cpp
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_Logging.cpp
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

add_silkit_test_to_executable(SilKitFunctionalTests
    SOURCES FTest_ServiceDiscoveryPerf.cpp
)

add_silkit_test_to_executable(SilKitIntegrationTests
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

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_DynStepSizes.cpp
)


add_silkit_test_to_executable(SilKitFunctionalTests
    SOURCES FTest_WallClockCoupling.cpp
)

# Windows specific tests
if(MSVC)
    add_silkit_test_to_executable(SilKitIntegrationTests
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

# Pub/Sub

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_PubHistory.cpp
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

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_Config.cpp
)

add_silkit_test_to_executable(SilKitIntegrationTests
    SOURCES ITest_RemoteLogging.cpp
)

# ============================================================
#  Integration Tests using SIL Kit internals (in-tree builds only)
# ============================================================

if(NOT _silkit_itests_standalone)
    add_silkit_test_executable(SilKitInternalIntegrationTests LABEL integration)

    target_link_libraries(SilKitInternalIntegrationTests
        PRIVATE I_SilKit
        PRIVATE S_SilKitImpl
        PRIVATE S_ITests_STH_Internals
    )

    # extension loading
    add_subdirectory(ExtensionTests)

    add_silkit_test_to_executable(SilKitInternalIntegrationTests
        SOURCES ITest_Internals_ParticipantModes.cpp
    )

    add_silkit_test_to_executable(SilKitInternalIntegrationTests
        SOURCES ITest_Internals_DataPubSub.cpp
    )

    add_silkit_test_to_executable(SilKitInternalIntegrationTests
        SOURCES ITest_Internals_TargetedMessaging.cpp
    )

    add_silkit_test_to_executable(SilKitInternalIntegrationTests
        SOURCES ITest_Internals_ServiceDiscovery.cpp
    )

    add_silkit_test_to_executable(SilKitInternalIntegrationTests
        SOURCES ITest_Internals_RequestReply.cpp
    )

    add_silkit_test_to_executable(SilKitInternalIntegrationTests
        SOURCES ITest_Internals_Rpc.cpp ITest_Internals_Rpc.hpp
    )
endif()
