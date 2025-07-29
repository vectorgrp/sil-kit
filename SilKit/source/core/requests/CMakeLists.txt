# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(I_SilKit_Core_RequestReply INTERFACE)

target_include_directories(I_SilKit_Core_RequestReply
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(I_SilKit_Core_RequestReply
    INTERFACE SilKitInterface

    INTERFACE I_SilKit_Core_Internal
    INTERFACE I_SilKit_Util_Uuid
)

add_library(O_SilKit_Core_RequestReply OBJECT
    RequestReplyDatatypes.hpp
    
    RequestReplyService.hpp
    RequestReplyService.cpp

    RequestReplySerdes.hpp
    RequestReplySerdes.cpp
)

target_link_libraries(O_SilKit_Core_RequestReply
    PUBLIC I_SilKit_Core_RequestReply
)

add_subdirectory(procs)

add_silkit_test_to_executable(SilKitUnitTests
    SOURCES Test_RequestReplyService.cpp 
    LIBS S_SilKitImpl I_SilKit_Core_Mock_Participant)

