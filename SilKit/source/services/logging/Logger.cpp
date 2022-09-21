/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <chrono>
#include <iomanip>
#include <sstream>

#include "Logger.hpp"

#include "fmt/chrono.h"
#include "fmt/format.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/null_sink.h"
// NB: we do not use the windows color sink, as that will open "CONOUT$" and
//     we won't be able to trivially capture its output in SilKitLauncher.
#include "spdlog/sinks/ansicolor_sink.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

#include "SpdlogTypeConversion.hpp"


namespace SilKit {
namespace Services {
namespace Logging {

namespace {
class SilKitRemoteSink
    : public spdlog::sinks::base_sink<spdlog::details::null_mutex>
{
public:
    SilKitRemoteSink() = delete;
    SilKitRemoteSink(const Logger::LogMsgHandler& handler)
        : _logMsgHandler{handler}
    {
    }

    void Disable()
    {
        _is_disabled = true;
    }

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        // ignore recursive calls to the remote logger or when explicitly disabled
        if (_is_disabled) return;

        _is_disabled = true;
        _logMsgHandler(from_spdlog(msg));
        _is_disabled = false;
    }

    void flush_() override {}

private:
    Logger::LogMsgHandler _logMsgHandler;
    bool _is_disabled{false};
};
} // anonymous namespace


Logger::Logger(const std::string& participantName, Config::Logging config)
    : _config{std::move(config)}
{
    // NB: do not create the _logger in the initializer list. If participantName is empty,
    //  this will cause a fairly unintuitive exception in spdlog.
    _logger = spdlog::create<spdlog::sinks::null_sink_st>(participantName);

    // NB: logger gets dropped from registry immediately after creating so that two participant with the same
    // participantName won't lead to a spdlog exception because a logger with this name does already exist.
    spdlog::drop(participantName);
    
    // set_default_logger should not be used here, as there can only be one default logger and if another participant
    // gets created, the first default logger will be dropped from the registry as well.

    // Generate a tm object for the timestamp once, so that all file loggers will have the very same timestamp.
    auto timeNow = std::time(nullptr);
    std::tm tmBuffer{};
#if defined(_WIN32)
    localtime_s(&tmBuffer, &timeNow);
#else
    localtime_r(&timeNow, &tmBuffer);
#endif

    for (auto sink : _config.sinks)
    {
        auto log_level = to_spdlog(sink.level);
        if (log_level < _logger->level())
            _logger->set_level(log_level);

        switch (sink.type)
        {
        case Config::Sink::Type::Remote:
            // The remote sink is instantiated and added later together with setting up
            // all necessary connection logic to avoid segmentation errors if sth. goes wrong
            break;

        case Config::Sink::Type::Stdout:
        {
#if _WIN32
            auto stdoutSink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
#else
            auto stdoutSink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
#endif
            stdoutSink->set_level(log_level);
            _logger->sinks().emplace_back(std::move(stdoutSink));
            break;
        }
        case Config::Sink::Type::File:
        {
            //
            auto filename = fmt::format("{}_{:%FT%H-%M-%S}.txt", sink.logName, tmBuffer);
            auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename);
            fileSink->set_level(log_level);
            _logger->sinks().push_back(fileSink);
        }
        }
    }

    _logger->flush_on(to_spdlog(_config.flushLevel));
}

void Logger::Log(Level level, const std::string& msg)
{
    _logger->log(to_spdlog(level), msg);
}

void Logger::Trace(const std::string& msg)
{
    Log(Level::Trace, msg);
}

void Logger::Debug(const std::string& msg)
{
    Log(Level::Debug, msg);
}

void Logger::Info(const std::string& msg)
{
    Log(Level::Info, msg);
}

void Logger::Warn(const std::string& msg)
{
    Log(Level::Warn, msg);
}

void Logger::Error(const std::string& msg)
{
    Log(Level::Error, msg);
}

void Logger::Critical(const std::string& msg)
{
    Log(Level::Critical, msg);
}

void Logger::RegisterRemoteLogging(const LogMsgHandler& handler)
{
    auto remoteSinkRef = std::find_if(_config.sinks.begin(), _config.sinks.end(),
        [](const Config::Sink& sink) 
        { 
            return sink.type == Config::Sink::Type::Remote;
        });

    if (remoteSinkRef != _config.sinks.end())
    {
        _remoteSink = std::make_shared<SilKitRemoteSink>(handler);
        _remoteSink->set_level(to_spdlog(remoteSinkRef->level));
        _logger->sinks().push_back(_remoteSink);
    }
}

void Logger::DisableRemoteLogging()
{
    for (auto sink : _logger->sinks())
    {
        auto* remoteSink = dynamic_cast<SilKitRemoteSink*>(sink.get());
        if (remoteSink)
        {
            remoteSink->Disable();
        }
    }
}

void Logger::LogReceivedMsg(const LogMsg& msg)
{
    auto spdlog_msg = to_spdlog(msg);

    for (auto&& sink : _logger->sinks())
    {
        if (to_spdlog(msg.level) < sink->level())
            continue;

        if (sink.get() == _remoteSink.get())
            continue;

        sink->log(spdlog_msg);

        if (_config.flushLevel <= msg.level)
            sink->flush();
    }
}

Level Logger::GetLogLevel() const
{
    return from_spdlog(_logger->level());
}

} // namespace Logging
} // namespace Services
} // namespace SilKit
