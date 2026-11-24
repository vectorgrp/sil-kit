# SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(O_SilKit_Management OBJECT
    IRegistryManagementParticipant.hpp
    RegistryManagementParticipant.hpp
    RegistryManagementParticipant.cpp
)

target_link_libraries(O_SilKit_Management
    PRIVATE I_SilKit
)


add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_RegistryManagementParticipant.cpp
    LIBS S_SilKitImpl
)
