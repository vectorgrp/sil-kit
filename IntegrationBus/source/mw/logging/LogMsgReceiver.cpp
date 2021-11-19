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

void LogMsgReceiver::ReceiveIbMessage(const IServiceId* from, const LogMsg& msg)
{
    _logger->LogReceivedMsg(msg);
}

void LogMsgReceiver::SetEndpointAddress(const ib::mw::EndpointAddress &address)
{
    _serviceId.legacyEpa = address;
}
auto LogMsgReceiver::EndpointAddress(void) const -> const ib::mw::EndpointAddress&
{
    return _serviceId.legacyEpa;
}

} // namespace logging
} // namespace mw
} // namespace ib
