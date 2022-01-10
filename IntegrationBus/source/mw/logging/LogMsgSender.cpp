// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LogMsgSender.hpp"

namespace ib {
namespace mw {
namespace logging {

LogMsgSender::LogMsgSender(IComAdapterInternal* comAdapter)
    : _comAdapter{comAdapter}
{
}

void LogMsgSender::SendLogMsg(const LogMsg& msg)
{
    _comAdapter->SendIbMessage(this, msg);
}

void LogMsgSender::SendLogMsg(LogMsg&& msg)
{
    _comAdapter->SendIbMessage(this, std::move(msg));
}

void LogMsgSender::SetEndpointAddress(const ib::mw::EndpointAddress &address)
{
    _serviceDescriptor.legacyEpa= address;
}
auto LogMsgSender::EndpointAddress(void) const -> const ib::mw::EndpointAddress&
{
    return _serviceDescriptor.legacyEpa;
}

} // namespace logging
} // namespace mw
} // namespace ib
