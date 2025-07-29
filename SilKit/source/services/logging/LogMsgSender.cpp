// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "LogMsgSender.hpp"

namespace SilKit {
namespace Services {
namespace Logging {

LogMsgSender::LogMsgSender(Core::IParticipantInternal* participant)
    : _participant{participant}
{
}

void LogMsgSender::SendLogMsg(const LogMsg& msg)
{
    _participant->SendMsg(this, msg);
}

void LogMsgSender::SendLogMsg(LogMsg&& msg)
{
    _participant->SendMsg(this, std::move(msg));
}

} // namespace Logging
} // namespace Services
} // namespace SilKit
