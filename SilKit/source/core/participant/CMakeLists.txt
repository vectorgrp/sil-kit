# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(I_SilKit_Core_Participant INTERFACE)

target_include_directories(I_SilKit_Core_Participant
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(I_SilKit_Core_Participant
    INTERFACE SilKitInterface

    INTERFACE I_SilKit_Core_VAsio
    INTERFACE I_SilKit_Services_Can
    INTERFACE I_SilKit_Services_Ethernet
    INTERFACE I_SilKit_Services_Flexray
    INTERFACE I_SilKit_Services_Lin
    INTERFACE I_SilKit_Services_Logging
    INTERFACE I_SilKit_Services_Orchestration
    INTERFACE I_SilKit_Services_PubSub
    INTERFACE I_SilKit_Services_Rpc
    INTERFACE I_SilKit_Tracing
    INTERFACE I_SilKit_Experimental_NetworkSimulatorInternals

    # Due to Participant_impl.hpp being used directly in NullConnectionParticipant and
    # RpcTestUtilities, this interface library must also depend on the headers used by
    # the participant implementation.
    INTERFACE I_SilKit_VersionImpl
)


add_library(O_SilKit_Core_Participant OBJECT
    Participant.hpp
    Participant_impl.hpp
    Participant.cpp
    CreateParticipantInternal.hpp
    CreateParticipantInternal.cpp
    ValidateAndSanitizeConfig.hpp
    ValidateAndSanitizeConfig.cpp
    CreateParticipantT.hpp
    CreateParticipantT.cpp
)

target_link_libraries(O_SilKit_Core_Participant
    PUBLIC I_SilKit_Core_Participant

    PRIVATE I_SilKit_Config
    PRIVATE I_SilKit_Core_Internal
    PRIVATE I_SilKit_Core_Service
    PRIVATE I_SilKit_Core_RequestReply
    PRIVATE I_SilKit_Util
    PRIVATE I_SilKit_Util_SetThreadName
    PRIVATE I_SilKit_Util_Uuid

    PRIVATE I_SilKit_VersionImpl
)

target_link_libraries(O_SilKit_Core_Participant PUBLIC I_SilKit_Core_VAsio)

if (MSVC)
    target_compile_options(O_SilKit_Core_Participant PRIVATE "/bigobj")
endif (MSVC)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_Participant.cpp
    LIBS S_SilKitImpl O_SilKit_Core_Mock_NullConnection I_SilKit_Core_Mock_NullConnection I_SilKit_Config_TestUtils
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_ValidateAndSanitizeConfig.cpp
    LIBS S_SilKitImpl
)

