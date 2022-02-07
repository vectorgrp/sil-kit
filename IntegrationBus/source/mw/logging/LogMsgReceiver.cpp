// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LogMsgReceiver.hpp"

namespace ib {
namespace mw {
namespace logging {

LogMsgReceiver::LogMsgReceiver(IComAdapterInternal* comAdapter, Logger* logger)
    : _comAdapter{comAdapter}
    , _logger{logger}
{
}

void LogMsgReceiver::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const LogMsg& msg)
{
    _logger->LogReceivedMsg(msg);
}

} // namespace logging
} // namespace mw
} // namespace ib
