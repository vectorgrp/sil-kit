// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LogMsgReceiver.hpp"

namespace ib {
namespace mw {
namespace logging {

LogMsgReceiver::LogMsgReceiver(IComAdapter* comAdapter)
    : _comAdapter{comAdapter}
{
}

void LogMsgReceiver::SetLogger(logging::ILogger* logger)
{
    _logger = logger;
}

void LogMsgReceiver::ReceiveIbMessage(mw::EndpointAddress from, const LogMsg& msg)
{
    if (from == _endpointAddress)
        return;

    _logger->LogReceivedMsg(msg);
}

void LogMsgReceiver::SetEndpointAddress(const ib::mw::EndpointAddress &address)
{
    _endpointAddress = address;
}
auto LogMsgReceiver::EndpointAddress(void) const -> const ib::mw::EndpointAddress&
{
    return _endpointAddress;
}

} // namespace logging
} // namespace mw
} // namespace ib
