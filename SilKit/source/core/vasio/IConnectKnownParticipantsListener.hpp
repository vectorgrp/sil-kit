// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


namespace SilKit {
namespace Core {


class ConnectKnownParticipants;


struct IConnectKnownParticipantsListener
{
    virtual ~IConnectKnownParticipantsListener() = default;

    virtual void OnConnectKnownParticipantsFailure(ConnectKnownParticipants& connectKnownParticipants) = 0;
    virtual void OnConnectKnownParticipantsWaitingForAllReplies(ConnectKnownParticipants& connectKnownParticipants) = 0;
    virtual void OnConnectKnownParticipantsAllRepliesReceived(ConnectKnownParticipants& connectKnownParticipants) = 0;
};


} // namespace Core
} // namespace SilKit
