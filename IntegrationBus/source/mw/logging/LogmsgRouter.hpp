// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ILogmsgRouter.hpp"
#include "IIbToLogmsgRouter.hpp"

#include "ib/mw/IComAdapter.hpp"

namespace spdlog {
namespace sinks {
    class sink;
} // namespace sinks
    class logger;
} // namespace spdlog

namespace ib {
namespace mw {
namespace logging {

auto from_spdlog(const spdlog::source_loc& spdMsg) -> SourceLoc;
auto from_spdlog(const spdlog::details::log_msg& spdMsg) -> LogMsg;

class LogmsgRouter
    : public ILogmsgRouter
    , public IIbToLogmsgRouter
{
public:
    LogmsgRouter(IComAdapter* comAdapter);

    void SetLogger(std::shared_ptr<spdlog::logger> logger);

    void Distribute(const LogMsg& msg);
    void Distribute(LogMsg&& msg);

    void ReceiveIbMessage(mw::EndpointAddress from, const LogMsg& msg) override;

    void SetEndpointAddress(const ib::mw::EndpointAddress &address) override;
    auto EndpointAddress(void) const -> const ib::mw::EndpointAddress & override;

private:
    // ----------------------------------------
    // private methods
    template <class MsgT>
    void SendIbMessage(MsgT&& msg) const;

private:
    // ----------------------------------------
    // private members
    IComAdapter* _comAdapter{nullptr};
    mw::EndpointAddress _endpointAddress{};
    
    std::shared_ptr<spdlog::logger> _logger;
    std::shared_ptr<spdlog::sinks::sink> _ibRemoteSink;
};

} // namespace logging
} // namespace mw
} // namespace ib
