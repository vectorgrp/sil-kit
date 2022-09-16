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

#pragma once

#include <memory>
#include <functional>

#include "silkit/services/logging/LoggingDatatypes.hpp"

#include "ILogger.hpp"
#include "Configuration.hpp"

namespace spdlog {
class logger;
namespace sinks {
class sink;
} // namespace sinks
} // namespace spdlog

namespace SilKit {
namespace Services {
namespace Logging {

struct LogMsg;

class Logger : public ILogger
{
public:
    using LogMsgHandler = std::function<void(LogMsg)>;

public:
    // ----------------------------------------
    // Constructors and Destructor
    Logger(const std::string& participantName, Config::Logging config);

    // ----------------------------------------
    // Public interface methods
    //
    // ILogger
    void Log(Level level, const std::string& msg) override;

    void Trace(const std::string& msg) override;

    void Debug(const std::string& msg) override;

    void Info(const std::string& msg) override;

    void Warn(const std::string& msg) override;

    void Error(const std::string& msg) override;

    void Critical(const std::string& msg) override;

    void RegisterRemoteLogging(const LogMsgHandler& handler);
    void DisableRemoteLogging();
    void LogReceivedMsg(const LogMsg& msg);

    Level GetLogLevel() const override;

private:
    // ----------------------------------------
    // Private members
    Config::Logging _config;

    std::shared_ptr<spdlog::logger> _logger;
    std::shared_ptr<spdlog::sinks::sink> _remoteSink;
};

} // namespace Logging
} // namespace Services
} // namespace SilKit
