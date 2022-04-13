// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LogMsgReceiver.hpp"

namespace ib {
namespace mw {
namespace logging {

LogMsgReceiver::LogMsgReceiver(IParticipantInternal* participant, Logger* logger)
    : _participant{participant}
    , _logger{logger}
{
    (void)_participant;
}

void LogMsgReceiver::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const LogMsg& msg)
{
    _logger->LogReceivedMsg(msg);
}

} // namespace logging
} // namespace mw
} // namespace ib
