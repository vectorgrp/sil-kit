# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

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
    PRIVATE I_SilKit
)

if (MSVC)
    target_compile_options(O_SilKit_Core_Participant PRIVATE "/bigobj")
endif (MSVC)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_Participant.cpp
    LIBS S_SilKitImpl O_SilKit_Core_Mock_NullConnection I_SilKit
)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_ValidateAndSanitizeConfig.cpp
    LIBS S_SilKitImpl
)
