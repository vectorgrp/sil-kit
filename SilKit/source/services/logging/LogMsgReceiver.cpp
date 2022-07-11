// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LogMsgReceiver.hpp"

namespace SilKit {
namespace Services {
namespace Logging {

LogMsgReceiver::LogMsgReceiver(Core::IParticipantInternal* participant, Logger* logger)
    : _participant{participant}
    , _logger{logger}
{
    (void)_participant;
}

void LogMsgReceiver::ReceiveMsg(const Core::IServiceEndpoint* /*from*/, const LogMsg& msg)
{
    _logger->LogReceivedMsg(msg);
}

} // namespace Logging
} // namespace Services
} // namespace SilKit