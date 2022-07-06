// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LogMsgReceiver.hpp"

namespace SilKit {
namespace Core {
namespace Logging {

LogMsgReceiver::LogMsgReceiver(IParticipantInternal* participant, Logger* logger)
    : _participant{participant}
    , _logger{logger}
{
    (void)_participant;
}

void LogMsgReceiver::ReceiveSilKitMessage(const IServiceEndpoint* /*from*/, const LogMsg& msg)
{
    _logger->LogReceivedMsg(msg);
}

} // namespace Logging
} // namespace Core
} // namespace SilKit
