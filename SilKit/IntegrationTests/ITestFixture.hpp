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
#include <future>
// for thread safe logging:
#include <mutex>
#include <iostream>
#include <sstream>
#include <string>
#include "GetTestPid.hpp"

#include "SimTestHarness.hpp"

#include "gtest/gtest.h"

namespace SilKit {
namespace Tests {

using namespace SilKit::Config;

using namespace std::chrono_literals;


inline std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
    out << seconds.count() << "s";
    return out;
}

struct ThreadSafeLogger
{
    // No copy
    ThreadSafeLogger(const ThreadSafeLogger&) = delete;
    ThreadSafeLogger& operator=(const ThreadSafeLogger&) = delete;
    // Only movable
    ThreadSafeLogger() = default;
    ThreadSafeLogger(ThreadSafeLogger&&) = default;
    ThreadSafeLogger& operator=(ThreadSafeLogger&&) = default;

    template<typename T>
    auto operator<<(T&& arg) -> ThreadSafeLogger&
    {
        buf << std::forward<T>(arg);
        return *this;
    }

    ~ThreadSafeLogger()
    {
        std::unique_lock<std::mutex> lock(logMx);
        std::cout << buf.str() << std::endl;
    }
    std::stringstream buf;
    static std::mutex logMx; //!< global per test executable
};

// we only include this header once per test
std::mutex ThreadSafeLogger::logMx;

//!< thread safe stream logger for testing
inline auto Log() -> ThreadSafeLogger
{
    return ThreadSafeLogger();
}


class ITest_SimTestHarness : public testing::Test
{
protected: //CTor and operators
    ITest_SimTestHarness()
    : _registryUri{MakeTestRegistryUri()}
    {
    }

    auto TestHarness() -> SimTestHarness&
    {
        return *_simTestHarness;
    }

    void SetupFromParticipantList(std::vector<std::string> participantNames)
    {
        // create test harness with deferred participant creation.
        // Will only create the SIL Kit Registry and tell the SystemController the participantNames
        _simTestHarness = std::make_unique<SimTestHarness>(participantNames, _registryUri, true);

    }

protected:// members
    std::string _registryUri;
    std::unique_ptr<SimTestHarness> _simTestHarness;
};

} //namespace Tests
} //namespace SilKit
