# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(I_SilKit_Core_RequestReply_ParticipantReplies INTERFACE)

target_include_directories(I_SilKit_Core_RequestReply_ParticipantReplies
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(I_SilKit_Core_RequestReply_ParticipantReplies
    INTERFACE SilKitInterface
    INTERFACE I_SilKit_Services_Logging
    INTERFACE I_SilKit_Core_RequestReply
    INTERFACE I_SilKit_Core_Internal
    INTERFACE I_SilKit_Util_Uuid
)

add_library(O_SilKit_Core_RequestReply_ParticipantReplies OBJECT
    ParticipantReplies.cpp
    ParticipantReplies.hpp
)

target_link_libraries(O_SilKit_Core_RequestReply_ParticipantReplies
    PUBLIC I_SilKit_Core_RequestReply_ParticipantReplies
)
