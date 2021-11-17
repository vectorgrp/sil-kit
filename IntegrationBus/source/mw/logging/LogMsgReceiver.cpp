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

void LogMsgReceiver::ReceiveIbMessage(mw::EndpointAddress from, const LogMsg& msg)
{
    if (from.participant == _endpointAddress.participant)
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
