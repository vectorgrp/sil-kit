// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <iomanip>
#include <sstream>

#include "Logger.hpp"

#include "spdlog/sinks/null_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

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


Logger::Logger(const std::string& participantName, const std::vector<cfg::Logger>& config)
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

    for (auto logger : config)
    {
        if (logger.type == cfg::Logger::Type::Remote)
        {
            _ibRemoteSink->set_level(to_spdlog(logger.level));
            _logger->sinks().push_back(_ibRemoteSink);
        }

        if (logger.type == cfg::Logger::Type::Stdout)
        {
            auto stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            stdoutSink->set_level(to_spdlog(logger.level));
            _logger->sinks().push_back(stdoutSink);
        }

        if (logger.type == cfg::Logger::Type::File)
        {
            // Generate Timestamp for log file.
            auto now = std::chrono::system_clock::now();
            auto itt = std::chrono::system_clock::to_time_t(now);
            std::ostringstream string_stream;
            string_stream << logger.filename << "_" << std::put_time(localtime(&itt), "%FT%H-%M-%S") << ".txt";

            auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(string_stream.str());
            fileSink->set_level(to_spdlog(logger.level));
            _logger->sinks().push_back(fileSink);
        }
    }
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
    auto spdlog_msg = to_spdlog(msg);

    for (auto&& sink : _logger->sinks())
    {
        if (sink.get() == _ibRemoteSink.get())
            continue;

        if (to_spdlog(msg.level) < sink->level())
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

auto Logger::GetSinks() -> const std::vector<spdlog::sink_ptr>&
{
    return _logger->sinks();
}

} // namespace logging
} // namespace mw
} // namespace ib
