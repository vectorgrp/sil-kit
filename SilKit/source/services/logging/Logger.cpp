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
#include <unordered_map>
#include <string>

#include "Logger.hpp"

#include "StringHelpers.hpp"

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
#include "spdlog/pattern_formatter.h"


namespace SilKit {
namespace Services {
namespace Logging {

class LoggerMessage;

struct SimpleLogMessage
{
    const std::string& msg;
    const std::unordered_map<std::string, std::string>& kv;
};

struct JsonLogMessage
{
    const std::string& msg;
    const std::unordered_map<std::string, std::string>& kv;
};

struct JsonString
{
    const std::string& m;
};

} // namespace Logging
} // namespace Services
} // namespace SilKit


// Custom flag handler to print the UNIX epoch timestamp
class epoch_formatter_flag : public spdlog::custom_flag_formatter
{
public:
    void format(const spdlog::details::log_msg& msg, const std::tm&, spdlog::memory_buf_t& dest) override
    {
        using namespace std::chrono;
        auto epoch_time = duration_cast<microseconds>(msg.time.time_since_epoch()).count();
        fmt::format_to(std::back_inserter(dest), "{}", epoch_time);
    }

    std::unique_ptr<spdlog::custom_flag_formatter> clone() const override
    {
        return spdlog::details::make_unique<epoch_formatter_flag>();
    }
};


std::string KeyValuesToSimpleString(const std::unordered_map<std::string, std::string>& input)
{
    std::string result;
    result.reserve(input.size() * 2);

    std::unordered_map<std::string, std::string>::const_iterator it = input.begin();

    while (it != input.end())
    {
        if (it != input.begin())
        {
            result.append(", ");
        }
        result.append(SilKit::Util::EscapeString(it->first) + ": " + SilKit::Util::EscapeString(it->second));
        ++it;
    }
    return result;
}

std::string KeyValuesToJsonString(const std::unordered_map<std::string, std::string>& input)
{
    std::string result;
    result.reserve(input.size() * 2);

    std::unordered_map<std::string, std::string>::const_iterator it = input.begin();
    result.append("{");
    while (it != input.end())
    {
        if (it != input.begin())
        {
            result.append(",");
        }
        result.append("\"" + SilKit::Util::EscapeString(it->first) + "\"" + ":" + "\""
                      + SilKit::Util::EscapeString(it->second) + "\"");
        ++it;
    }
    result.append("}");

    return result;
}


template <>
struct fmt::formatter<SilKit::Services::Logging::SimpleLogMessage>
{
    constexpr auto parse(fmt::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const SilKit::Services::Logging::SimpleLogMessage& msg, FormatContext& ctx)
    {
        if (!msg.kv.empty())
        {
            return fmt::format_to(ctx.out(), "{}, {}", msg.msg, KeyValuesToSimpleString(msg.kv));
        }
        else
        {
            return fmt::format_to(ctx.out(), "{}", msg.msg);
        }
    }
};


template <>
struct fmt::formatter<SilKit::Services::Logging::JsonLogMessage>
{
    constexpr auto parse(fmt::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const SilKit::Services::Logging::JsonLogMessage& msg, FormatContext& ctx)
    {
        if (!msg.kv.empty())
        {
            return fmt::format_to(ctx.out(), "\"msg\": \"{}\", \"kv\": {}", SilKit::Util::EscapeString(msg.msg),
                                  KeyValuesToJsonString(msg.kv));
        }
        else
        {
            return fmt::format_to(ctx.out(), "\"msg\": \"{}\"", SilKit::Util::EscapeString(msg.msg));
        }
    }
};

template <>
struct fmt::formatter<SilKit::Services::Logging::JsonString>
{
    constexpr auto parse(fmt::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const SilKit::Services::Logging::JsonString& msg, FormatContext& ctx)
    {
        // format the message output string
        // "msg": "This is the log message", "kv":{ "key1": "value1", key2: "value2"}
        // the message, key and value strings needed to be escaped
        return fmt::format_to(ctx.out(), "\"msg\": \"{}\"", SilKit::Util::EscapeString(msg.m));
    }
};


namespace SilKit {
namespace Services {
namespace Logging {


namespace {
class SilKitRemoteSink : public spdlog::sinks::base_sink<spdlog::details::null_mutex>
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
        if (_is_disabled)
            return;

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
    for (auto sink : _config.sinks)
    {
        if (sink.type == Config::Sink::Type::Remote)
        {
            _loggerRemote = std::make_shared<RemoteLogger>(sink.level, participantName);
        }
        else
        {
            // NB: logger gets dropped from registry immediately after creating so that two participant with the same
            // participantName won't lead to a spdlog exception because a logger with this name does already exist.
            if (sink.format == Config::Sink::Format::Json && nullptr == _loggerJson)
            {
                _loggerJson = spdlog::create<spdlog::sinks::null_sink_st>(participantName);
                spdlog::drop(participantName);
            }
            if (sink.format == Config::Sink::Format::Simple && nullptr == _loggerSimple)
            {
                _loggerSimple = spdlog::create<spdlog::sinks::null_sink_st>(participantName);
                spdlog::drop(participantName);
            }
        }
    }

    // set_default_logger should not be used here, as there can only be one default logger and if another participant
    // gets created, the first default logger will be dropped from the registry as well.

    // Generate a tm object for the timestamp once, so that all file loggers will have the very same timestamp.
    const auto logFileTimestamp = SilKit::Util::CurrentTimestampString();

    // Defined JSON pattern for the logger output
    std::string jsonpattern{R"({"ts":"%E","log":"%n","lvl":"%l", %v })"};

    for (auto sink : _config.sinks)
    {
        auto log_level = to_spdlog(sink.level);
        if (sink.format == Config::Sink::Format::Json && sink.type != Config::Sink::Type::Remote)
        {
            if (log_level < _loggerJson->level())
            {
                _loggerJson->set_level(log_level);
            }
        }
        if (sink.format == Config::Sink::Format::Simple && sink.type != Config::Sink::Type::Remote)
        {
            if (log_level < _loggerSimple->level())
            {
                _loggerSimple->set_level(log_level);
            }
        }

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

            if (sink.format == Config::Sink::Format::Json && sink.type != Config::Sink::Type::Remote)
            {
                using spdlog::details::make_unique; // for pre c++14
                auto formatter = make_unique<spdlog::pattern_formatter>();
                formatter->add_flag<epoch_formatter_flag>('E').set_pattern("[%E] [%^%l%$] %v");
                formatter->set_pattern(jsonpattern);
                stdoutSink->set_formatter(std::move(formatter));
                stdoutSink->set_level(log_level);
                _loggerJson->sinks().emplace_back(std::move(stdoutSink));
            }
            else if (sink.type != Config::Sink::Type::Remote)
            {
                stdoutSink->set_level(log_level);
                _loggerSimple->sinks().emplace_back(std::move(stdoutSink));
            }
            break;
        }
        case Config::Sink::Type::File:
        {
            if (sink.format == Config::Sink::Format::Json)
            {
                auto filename = fmt::format("{}_{}.jsonl", sink.logName, logFileTimestamp);
                auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename);
                using spdlog::details::make_unique; // for pre c++14
                auto formatter = make_unique<spdlog::pattern_formatter>();
                formatter->add_flag<epoch_formatter_flag>('E').set_pattern("[%E] [%^%l%$] %v");
                formatter->set_pattern(jsonpattern);
                fileSink->set_formatter(std::move(formatter));
                fileSink->set_level(log_level);
                _loggerJson->sinks().push_back(fileSink);
            }
            else
            {
                auto filename = fmt::format("{}_{}.txt", sink.logName, logFileTimestamp);
                auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename);
                fileSink->set_level(log_level);
                _loggerSimple->sinks().push_back(fileSink);
            }
        }
        }
    }
    if (nullptr != _loggerSimple)
    {
        _loggerSimple->flush_on(to_spdlog(_config.flushLevel));
    }
    if (nullptr != _loggerJson)
    {
        _loggerJson->flush_on(to_spdlog(_config.flushLevel));
    }
}

void Logger::ProcessLoggerMessage(const LoggerMessage& msg)
{
    const auto now = log_clock::now();
    if (nullptr != _loggerJson)
    {
        JsonLogMessage myJsonMsg{msg.GetMsgString(), msg.GetKeyValues()};

        _loggerJson->log(now, spdlog::source_loc{}, to_spdlog(msg.GetLevel()), fmt::format("{}", myJsonMsg));
    }

    if (nullptr != _loggerSimple)
    {
        SimpleLogMessage myMsg{msg.GetMsgString(), msg.GetKeyValues()};
        _loggerSimple->log(now, spdlog::source_loc{}, to_spdlog(msg.GetLevel()), fmt::format("{}", myMsg));
    }
    if (nullptr != _loggerRemote)
    {
        _loggerRemote->Log(now, msg);
    }
}

void Logger::LogReceivedMsg(const LogMsg& msg)
{
    if (nullptr != _loggerJson)
    {
        JsonLogMessage jsonMsg{msg.payload, msg.keyValues};

        auto fmt{fmt::format("{}", jsonMsg)};
        auto spdlog_msg = to_spdlog(msg, fmt);


        for (auto&& sink : _loggerJson->sinks())
        {
            if (to_spdlog(msg.level) < sink->level())
                continue;

            sink->log(spdlog_msg);

            if (_config.flushLevel <= msg.level)
                sink->flush();
        }
    }

    if (nullptr != _loggerSimple)
    {
        SimpleLogMessage simpleMsg{msg.payload, msg.keyValues};

        auto fmt{fmt::format("{}", simpleMsg)};
        auto spdlog_msg = to_spdlog(msg, fmt);

        for (auto&& sink : _loggerSimple->sinks())
        {
            if (to_spdlog(msg.level) < sink->level())
                continue;

            sink->log(spdlog_msg);

            if (_config.flushLevel <= msg.level)
                sink->flush();
        }
    }
}


void Logger::Log(Level level, const std::string& msg)
{
    const auto now = log_clock::now();
    if (nullptr != _loggerJson)
    {
        JsonString jsonString{msg};
        _loggerJson->log(now, spdlog::source_loc{}, to_spdlog(level), fmt::format("{}", jsonString));
    }
    if (nullptr != _loggerSimple)
    {
        _loggerSimple->log(now, spdlog::source_loc{}, to_spdlog(level), msg);
    }
    if (nullptr != _loggerRemote)
    {
        _loggerRemote->Log(now, level, msg);
    }
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
    if (nullptr != _loggerRemote)
    {
        _loggerRemote->RegisterRemoteLogging(handler);
    }
}

void Logger::DisableRemoteLogging()
{
    if (nullptr != _loggerRemote)
    {
        _loggerRemote->DisableRemoteLogging();
    }
}

Level Logger::GetLogLevel() const
{
    auto lvl = to_spdlog(Level::Critical);

    if (nullptr != _loggerSimple)
    {
        lvl = lvl < _loggerSimple->level() ? lvl : _loggerSimple->level();
    }
    if (nullptr != _loggerJson)
    {
        lvl = lvl < _loggerJson->level() ? lvl : _loggerJson->level();
    }
    if (nullptr != _loggerRemote)
    {
        lvl = lvl < to_spdlog(_loggerRemote->level()) ? lvl : to_spdlog(_loggerRemote->level());
    }

    return from_spdlog(lvl);
}

} // namespace Logging
} // namespace Services
} // namespace SilKit
