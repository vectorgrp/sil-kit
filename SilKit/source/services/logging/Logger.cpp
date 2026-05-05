// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <chrono>
#include <iomanip>
#include <sstream>
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
    const std::vector<std::pair<std::string, std::string>>& kv;
    Topic topic;
};

struct JsonLogMessage
{
    const std::string& msg;
    const std::vector<std::pair<std::string, std::string>>& kv;
    Topic topic;
};

struct JsonString
{
    const std::string& m;
    Topic topic;
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


std::string KeyValuesToSimpleString(const std::vector<std::pair<std::string, std::string>>& input)
{
    std::string result;
    result.reserve(input.size() * 2);

    std::vector<std::pair<std::string, std::string>>::const_iterator it = input.begin();

    while (it != input.end())
    {
        if (it != input.begin())
        {
            result.append(", ");
        }
        result.append(it->first + ": " + it->second);
        ++it;
    }
    return result;
}

std::string KeyValuesToJsonString(const std::vector<std::pair<std::string, std::string>>& input)
{
    std::string result;
    result.reserve(input.size() * 2);

    std::vector<std::pair<std::string, std::string>>::const_iterator it = input.begin();
    result.append("{");
    while (it != input.end())
    {
        if (it != input.begin())
        {
            result.append(",");
        }


        if (it->first == SilKit::Services::Logging::Keys::raw)
        {
            result.append("\"" + SilKit::Util::EscapeString(it->first) + "\"" + ":" + it->second);
        }
        else
        {
            result.append("\"" + SilKit::Util::EscapeString(it->first) + "\"" + ":" + "\""
                          + SilKit::Util::EscapeString(it->second) + "\"");
        }
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
    auto format(const SilKit::Services::Logging::SimpleLogMessage& msg, FormatContext& ctx) const
    {
        bool hasTopic = !(msg.topic == SilKit::Services::Logging::Topic::None);
        bool hasKv = !msg.kv.empty();

        if (hasTopic && hasKv)
        {
            return fmt::format_to(ctx.out(), "[{}] {}, {}", to_string(msg.topic), msg.msg,
                                  KeyValuesToSimpleString(msg.kv));
        }
        else if (hasTopic)
        {
            return fmt::format_to(ctx.out(), "[{}] {}", to_string(msg.topic), msg.msg);
        }
        else if (hasKv)
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
    auto format(const SilKit::Services::Logging::JsonLogMessage& msg, FormatContext& ctx) const
    {
        bool hasTopic = !(msg.topic == SilKit::Services::Logging::Topic::None);
        bool hasKv = !msg.kv.empty();

        if (hasTopic && hasKv)
        {
            return fmt::format_to(ctx.out(), "\"topic\": \"{}\", \"msg\": \"{}\", \"kv\": {}",
                                  SilKit::Util::EscapeString(to_string(msg.topic)),
                                  SilKit::Util::EscapeString(msg.msg),
                                  KeyValuesToJsonString(msg.kv));
        }
        else if (hasTopic)
        {
            return fmt::format_to(ctx.out(), "\"topic\": \"{}\", \"msg\": \"{}\"",
                                  SilKit::Util::EscapeString(to_string(msg.topic)),
                                  SilKit::Util::EscapeString(msg.msg));
        }
        else if (hasKv)
        {
            return fmt::format_to(ctx.out(), "\"msg\": \"{}\", \"kv\": {}",
                                  SilKit::Util::EscapeString(msg.msg),
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
    auto format(const SilKit::Services::Logging::JsonString& msg, FormatContext& ctx) const
    {

        return fmt::format_to(ctx.out(), "\"topic\": \"{}\", \"msg\": \"{}\"",
                                SilKit::Util::EscapeString(to_string(msg.topic)),
                                SilKit::Util::EscapeString(msg.m));
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
            _remoteLogger.emplace(std::make_shared<RemoteLogger>(sink.level, participantName), sink);
        }
        else
        {
            // NB: logger gets dropped from registry immediately after creating so that two participant with the same
            // participantName won't lead to a spdlog exception because a logger with this name does already exist.
            if (sink.format == Config::Sink::Format::Json)
            {
                _spdlogLogger.emplace(spdlog::create<spdlog::sinks::null_sink_st>(participantName), sink);
                spdlog::drop(participantName);
            }
            if (sink.format == Config::Sink::Format::Simple)
            {
                _spdlogLogger.emplace(spdlog::create<spdlog::sinks::null_sink_st>(participantName), sink);
                spdlog::drop(participantName);
            }
        }
    }

    // set_default_logger should not be used here, as there can only be one default logger and if another participant
    // gets created, the first default logger will be dropped from the registry as well.

    // Generate a tm object for the timestamp once, so that all file loggers will have the very same timestamp.
    const auto logFileTimestamp = SilKit::Util::CurrentTimestampString();

    // Defined JSON pattern for the logger output
    const std::string jsonpattern{R"({"ts":"%E","log":"%n","lvl":"%l", %v })"};

    for (const auto& pair : _spdlogLogger)
    {
        auto log_level = to_spdlog(pair.second.level);
        pair.first->set_level(log_level);

        if (pair.second.type == Config::Sink::Type::Stdout)
        {
#if _WIN32
            auto stdoutSink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
#else
            auto stdoutSink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
#endif
            if (pair.second.format == Config::Sink::Format::Json)
            {
                using spdlog::details::make_unique; // for pre c++14
                auto formatter = make_unique<spdlog::pattern_formatter>();
                formatter->add_flag<epoch_formatter_flag>('E').set_pattern("[%E] [%^%l%$] %v");
                formatter->set_pattern(jsonpattern);
                stdoutSink->set_formatter(std::move(formatter));
                stdoutSink->set_level(log_level);
                pair.first->sinks().emplace_back(std::move(stdoutSink));
            }
            else // Simple format 
            {
                stdoutSink->set_level(log_level);
                pair.first->sinks().emplace_back(std::move(stdoutSink));
            }
        }
        else if (pair.second.type == Config::Sink::Type::File)
        {
            if (pair.second.format == Config::Sink::Format::Json)
            {
                auto filename = fmt::format("{}_{}_{}.jsonl", pair.second.logName,
                                            SilKit::Util::PrintableString(participantName), logFileTimestamp);
                auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename);
                using spdlog::details::make_unique; // for pre c++14
                auto formatter = make_unique<spdlog::pattern_formatter>();
                formatter->add_flag<epoch_formatter_flag>('E').set_pattern("[%E] [%^%l%$] %v");
                formatter->set_pattern(jsonpattern);
                fileSink->set_formatter(std::move(formatter));
                fileSink->set_level(log_level);
                pair.first->sinks().push_back(fileSink);
            }
            else
            {
                auto filename = fmt::format("{}_{}_{}.txt", pair.second.logName,
                                            SilKit::Util::PrintableString(participantName), logFileTimestamp);
                auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename);
                fileSink->set_level(log_level);
                pair.first->sinks().push_back(fileSink);
            }
        }
    }

    for (const auto& pair : _spdlogLogger)
    {
        pair.first->flush_on(to_spdlog(_config.flushLevel));
    }
}

void Logger::DispatchToSinks(log_clock::time_point now, Level level, Topic topic,
                             const std::function<std::string(Config::Sink::Format)>& formatter,
                             const std::function<void(const std::shared_ptr<RemoteLogger>&, log_clock::time_point)>& remoteDispatcher)
{
    for (const auto& pair : _spdlogLogger)
    {
        if (!IsLevelMatching(pair.second.level, level))
        {
            continue;
        }

        if (IsTopicDisabled(pair.second.enabledTopics, pair.second.disabledTopics, topic))
        {
            continue;
        }

        auto formatted = formatter(pair.second.format);
        pair.first->log(now, spdlog::source_loc{}, to_spdlog(level), formatted);
    }

    for (const auto& pair : _remoteLogger)
    {
        if (!IsLevelMatching(pair.second.level, level))
        {
            continue;
        }

        if (IsTopicDisabled(pair.second.enabledTopics, pair.second.disabledTopics, topic))
        {
            continue;
        }

        if (remoteDispatcher)
        {
            remoteDispatcher(pair.first, now);
        }
        else
        {
            pair.first->Log(now, level, topic, formatter(Config::Sink::Format::Simple));
        }
    }
}

void Logger::ProcessLoggerMessage(const LoggerMessage& msg)
{
    const auto now = log_clock::now();

    DispatchToSinks(now, msg.GetLevel(), msg.GetTopic(),
        [&msg](Config::Sink::Format format) -> std::string {
            if (format == Config::Sink::Format::Json)
            {
                JsonLogMessage myJsonMsg{msg.GetMsgString(), msg.GetKeyValues(), msg.GetTopic()};
                return fmt::format("{}", myJsonMsg);
            }
            else
            {
                SimpleLogMessage myMsg{msg.GetMsgString(), msg.GetKeyValues(), msg.GetTopic()};
                return fmt::format("{}", myMsg);
            }
        },
        [&msg](const std::shared_ptr<RemoteLogger>& remote, log_clock::time_point tp) {
            remote->Log(tp, msg);
        });
}

void Logger::LogReceivedMsg(const LogMsg& msg)
{
    for (const auto& pair : _spdlogLogger)
    {
        if (pair.second.format == Config::Sink::Format::Json)
        {
            JsonLogMessage jsonMsg{msg.payload, msg.keyValues, msg.topic};

            auto fmt{fmt::format("{}", jsonMsg)};
            auto spdlog_msg = to_spdlog(msg, fmt);

            for (auto&& sink : pair.first->sinks())
            {
                if (to_spdlog(msg.level) < sink->level())
                    continue;

                sink->log(spdlog_msg);

                if (_config.flushLevel <= msg.level)
                    sink->flush();
            }
        }
        else
        {
            SimpleLogMessage simpleMsg{msg.payload, msg.keyValues, msg.topic};

            auto fmt{fmt::format("{}", simpleMsg)};
            auto spdlog_msg = to_spdlog(msg, fmt);

            for (auto&& sink : pair.first->sinks())
            {
                if (to_spdlog(msg.level) < sink->level())
                    continue;

                sink->log(spdlog_msg);

                if (_config.flushLevel <= msg.level)
                    sink->flush();
            }
        }
    }
}

void Logger::Log(Level level, Topic topic, const std::string& msg)
{
    const auto now = log_clock::now();

    DispatchToSinks(now, level, topic, [&msg, topic](Config::Sink::Format format) -> std::string {
        if (format == Config::Sink::Format::Json)
        {
            JsonString jsonString{msg, topic};
            return fmt::format("{}", jsonString);
        }
        else
        {
            return fmt::format("[{}] {}", to_string(topic), msg);
        }
    });
}


void Logger::RegisterRemoteLogging(const LogMsgHandler& handler)
{
    for (const auto& pair : _remoteLogger)
    {
        pair.first->RegisterRemoteLogging(handler);
    }
}

void Logger::DisableRemoteLogging()
{
    for (const auto& pair : _remoteLogger)
    {
        pair.first->DisableRemoteLogging();
    }
}

Level Logger::GetLogLevel() const
{
    auto lvl = to_spdlog(Level::Critical);

    for (const auto& pair : _spdlogLogger)
    {
        lvl = lvl < pair.first->level() ? lvl : pair.first->level();
    }

    for (const auto& pair : _remoteLogger)
    {
        lvl = lvl < to_spdlog(pair.first->level()) ? lvl : to_spdlog(pair.first->level());
    }
    return from_spdlog(lvl);
}

} // namespace Logging
} // namespace Services
} // namespace SilKit
