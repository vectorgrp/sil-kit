// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "Logger.hpp"

#include "spdlog/sinks/null_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace ib {
namespace mw {
namespace logging {

namespace {
class IbRemoteSink
    : public spdlog::sinks::base_sink<spdlog::details::null_mutex>
{
public:
    IbRemoteSink() = delete;
    IbRemoteSink(Logger* logger)
        : _logger{logger}
    {
    }

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        _logger->Distribute(std::move(from_spdlog(msg)));
    }

    void flush_() override {}

private:
    Logger* _logger{nullptr};
};
} // anonymous namespace


Logger::Logger(const std::string& participantName)
    : _ibRemoteSink{std::make_shared<IbRemoteSink>(this)}
{
    // NB: do not create the _logger in the initializer list. If participantName is empty,
    //  this will cause a fairly unintuitive exception in spdlog.
    _logger = spdlog::create<spdlog::sinks::null_sink_st>(participantName);

    // NB: logger gets dropped from registry immediately after creating so that two comAdapter with the same
    // participantName won't lead to a spdlog exception because a logger with this name does already exist.
    spdlog::drop(participantName);
    // set_default_logger should not be used here, as there can only be one default logger and if another comAdapter
    // gets created, the first default logger will be dropped from the registry as well.

    _logger->sinks().push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    _logger->sinks().push_back(_ibRemoteSink);
}

void Logger::Log(Level level, const std::string& msg)
{
    _logger->log(to_spdlog(level), msg);
}

void Logger::Trace(const std::string& msg)
{
    Log(Level::trace, msg);
}

void Logger::Debug(const std::string& msg)
{
    Log(Level::debug, msg);
}

void Logger::Info(const std::string& msg)
{
    Log(Level::info, msg);
}

void Logger::Warn(const std::string& msg)
{
    Log(Level::warn, msg);
}

void Logger::Error(const std::string& msg)
{
    Log(Level::error, msg);
}

void Logger::Critical(const std::string& msg)
{
    Log(Level::critical, msg);
}

void Logger::RegisterLogMsgHandler(LogMsgHandlerT handler)
{
    _logMsgHandler = std::move(handler);
}

void Logger::LogReceivedMsg(const LogMsg& msg)
{
    if (to_spdlog(msg.level) < _logger->level())
        return;

    auto spdlog_msg = to_spdlog(msg);

    for (auto&& sink : _logger->sinks())
    {
        if (sink.get() == _ibRemoteSink.get())
            continue;

        sink->log(spdlog_msg);
    }
}

void Logger::Distribute(const LogMsg& msg)
{
    _logMsgHandler(msg);
}

void Logger::Distribute(LogMsg&& msg)
{
    _logMsgHandler(std::move(msg));
}

} // namespace logging
} // namespace mw
} // namespace ib
