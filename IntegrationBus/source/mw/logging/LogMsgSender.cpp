// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LogMsgSender.hpp"

namespace ib {
namespace mw {
namespace logging {

LogMsgSender::LogMsgSender(IParticipantInternal* participant)
    : _participant{participant}
{
}

void LogMsgSender::SendLogMsg(const LogMsg& msg)
{
    _participant->SendIbMessage(this, msg);
}

void LogMsgSender::SendLogMsg(LogMsg&& msg)
{
    _participant->SendIbMessage(this, std::move(msg));
}

} // namespace logging
} // namespace mw
} // namespace ib
