// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include "IConnectKnownParticipantsListener.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


namespace SilKit {
namespace Core {


struct MockConnectKnownParticipantsListener : IConnectKnownParticipantsListener
{
    MOCK_METHOD(void, OnConnectKnownParticipantsFailure, (ConnectKnownParticipants&), (override));
    MOCK_METHOD(void, OnConnectKnownParticipantsWaitingForAllReplies, (ConnectKnownParticipants&), (override));
    MOCK_METHOD(void, OnConnectKnownParticipantsAllRepliesReceived, (ConnectKnownParticipants&), (override));
};


} // namespace Core
} // namespace SilKit
