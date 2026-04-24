# SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

add_library(O_SilKit_Core_RequestReply_ParticipantReplies OBJECT
    ParticipantReplies.cpp
    ParticipantReplies.hpp
)

target_link_libraries(O_SilKit_Core_RequestReply_ParticipantReplies
    PRIVATE I_SilKit
)
