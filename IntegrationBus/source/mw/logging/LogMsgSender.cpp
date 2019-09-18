// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LogMsgSender.hpp"

namespace ib {
namespace mw {
namespace logging {

LogMsgSender::LogMsgSender(IComAdapter* comAdapter)
    : _comAdapter{comAdapter}
{
}

void LogMsgSender::SendLogMsg(const LogMsg& msg)
{
    _comAdapter->SendIbMessage(_endpointAddress, msg);
}

void LogMsgSender::SendLogMsg(LogMsg&& msg)
{
    _comAdapter->SendIbMessage(_endpointAddress, std::move(msg));
}

void LogMsgSender::SetEndpointAddress(const ib::mw::EndpointAddress &address)
{
    _endpointAddress = address;
}
auto LogMsgSender::EndpointAddress(void) const -> const ib::mw::EndpointAddress&
{
    return _endpointAddress;
}

} // namespace logging
} // namespace mw
} // namespace ib
