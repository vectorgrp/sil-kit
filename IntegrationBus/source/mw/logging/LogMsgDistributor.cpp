// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LogMsgDistributor.hpp"

namespace ib {
namespace mw {
namespace logging {

LogMsgDistributor::LogMsgDistributor(IComAdapter* comAdapter)
    : _comAdapter{comAdapter}
{
}

void LogMsgDistributor::SendLogMsg(const LogMsg& msg)
{
    _comAdapter->SendIbMessage(_endpointAddress, msg);
}

void LogMsgDistributor::SendLogMsg(LogMsg&& msg)
{
    _comAdapter->SendIbMessage(_endpointAddress, std::move(msg));
}

template <class MsgT>
void LogMsgDistributor::SendIbMessage(MsgT&& msg) const
{
    _comAdapter->SendIbMessage(_endpointAddress, std::forward<MsgT>(msg));
}

void LogMsgDistributor::SetEndpointAddress(const ib::mw::EndpointAddress &address)
{
    _endpointAddress = address;
}
auto LogMsgDistributor::EndpointAddress(void) const -> const ib::mw::EndpointAddress&
{
    return _endpointAddress;
}

} // namespace logging
} // namespace mw
} // namespace ib
