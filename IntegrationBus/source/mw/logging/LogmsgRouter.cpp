// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LogmsgRouter.hpp"

#include "spdlog/logger.h"
#include "spdlog/details/log_msg.h"
#include "spdlog/sinks/base_sink.h"

namespace ib {
namespace mw {
namespace logging {

auto from_spdlog(const spdlog::source_loc& spdLoc) -> SourceLoc
{
    SourceLoc loc;
    loc.filename = spdLoc.filename;
    loc.line = spdLoc.line;
    loc.funcname = spdLoc.funcname;

    return loc;
}
auto from_spdlog(const spdlog::details::log_msg& spdMsg) -> LogMsg
{
    LogMsg msg;
    if (spdMsg.logger_name)
        msg.logger_name = *spdMsg.logger_name;
    msg.level = spdMsg.level;
    msg.time = spdMsg.time;
    msg.source = from_spdlog(spdMsg.source);
    msg.payload = std::string{spdMsg.payload.begin(), spdMsg.payload.end()};

    return msg;
}

auto to_spdlog(const SourceLoc& loc) -> spdlog::source_loc
{
    return spdlog::source_loc{loc.filename.c_str(), static_cast<int>(loc.line), loc.funcname.c_str()};
}
auto to_spdlog(const LogMsg& msg) -> spdlog::details::log_msg
{
    return spdlog::details::log_msg{
        to_spdlog(msg.source),
        &msg.logger_name,
        msg.level,
        msg.payload
    };
}

namespace {
class IbRemoteSink
    : public spdlog::sinks::base_sink<spdlog::details::null_mutex>
{
public:
    IbRemoteSink() = delete;
    IbRemoteSink(LogmsgRouter* logmsgRouter)
        : _logmsgRouter{logmsgRouter}
    {
    }

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        _logmsgRouter->Distribute(std::move(from_spdlog(msg)));
    }

    void flush_() override {}

private:
    LogmsgRouter* _logmsgRouter{nullptr};
};
} // anonymous namespace

LogmsgRouter::LogmsgRouter(IComAdapter* comAdapter)
    : _comAdapter{comAdapter}
    , _ibRemoteSink{std::make_shared<IbRemoteSink>(this)}
{
}

void LogmsgRouter::Distribute(const LogMsg& msg)
{
    _comAdapter->SendIbMessage(_endpointAddress, msg);
}

void LogmsgRouter::Distribute(LogMsg&& msg)
{
    _comAdapter->SendIbMessage(_endpointAddress, std::move(msg));
}

void LogmsgRouter::ReceiveIbMessage(mw::EndpointAddress from, const LogMsg& msg)
{
    if (from == _endpointAddress)
        return;

    if (msg.level < _logger->level())
        return;

    auto spdlog_msg = to_spdlog(msg);

    for (auto&& sink: _logger->sinks())
    {
        if (sink.get() == _ibRemoteSink.get())
            continue;

        sink->log(spdlog_msg);
    }
}

void LogmsgRouter::SetLogger(std::shared_ptr<spdlog::logger> logger)
{
    logger->sinks().push_back(_ibRemoteSink);
    _logger = logger;
}

template <class MsgT>
void LogmsgRouter::SendIbMessage(MsgT&& msg) const
{
    _comAdapter->SendIbMessage(_endpointAddress, std::forward<MsgT>(msg));
}

void LogmsgRouter::SetEndpointAddress(const ib::mw::EndpointAddress &address)
{
    _endpointAddress = address;
}
auto LogmsgRouter::EndpointAddress(void) const -> const ib::mw::EndpointAddress &
{
    return _endpointAddress;
}

} // namespace logging
} // namespace mw
} // namespace ib
