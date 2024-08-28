// Copyright (c) 2022 Vector Informatik GmbH
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once


#include "ILoggerInternal.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


namespace SilKit {
namespace Services {
namespace Logging {


class MockLogger : public ::SilKit::Services::Logging::ILoggerInternal
{
    using Level = ::SilKit::Services::Logging::Level;
    using LoggerMessage = ::SilKit::Services::Logging::LoggerMessage;
    using LogMsg = ::SilKit::Services::Logging::LogMsg;

public:
    MockLogger()
    {
        ON_CALL(*this, GetLogLevel).WillByDefault(testing::Return(Level::Trace));
    }

public:
    MOCK_METHOD(void, Log, (Level level, const std::string& msg), (override));
    MOCK_METHOD(void, ProcessLoggerMessage, (const LoggerMessage& msg), (override));
    MOCK_METHOD(void, LogReceivedMsg, (const LogMsg& msg), (override));

    void Trace(const std::string& msg) override
    {
        Log(Level::Trace, msg);
    }

    void Debug(const std::string& msg) override
    {
        Log(Level::Debug, msg);
    }

    void Info(const std::string& msg) override
    {
        Log(Level::Info, msg);
    }

    void Warn(const std::string& msg) override
    {
        Log(Level::Warn, msg);
    }

    void Error(const std::string& msg) override
    {
        Log(Level::Error, msg);
    }

    void Critical(const std::string& msg) override
    {
        Log(Level::Critical, msg);
    }

    MOCK_METHOD(SilKit::Services::Logging::Level, GetLogLevel, (), (const, override));
};


} // namespace Logging
} // namespace Services
} // namespace SilKit
