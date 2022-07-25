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

#include <atomic>
#include <future>
#include <chrono>

#include "ParticipantConfiguration.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

class WatchDog
{
public:
    // ----------------------------------------
    // Constructors, Destructor, and Assignment
    WatchDog(const Config::HealthCheck& healthCheckConfig);
    ~WatchDog();

public:
    // ----------------------------------------
    // Public Methods
    void Start();
    void Reset();

    void SetWarnHandler(std::function<void(std::chrono::milliseconds)> handler);
    void SetErrorHandler(std::function<void(std::chrono::milliseconds)> handler);

    // For testing purposes only
    std::chrono::milliseconds GetWarnTimeout();
    std::chrono::milliseconds GetErrorTimeout();

private:
    // ----------------------------------------
    // private methods
    void Run();

public:
    const std::chrono::milliseconds _defaultTimeout = std::chrono::milliseconds::max();


private:
    // ----------------------------------------
    // private members
    std::promise<void> _stopPromise;
    // we use a duration instead of a timepoint to avoid a bug in clang6 (up to v9.0)
    std::atomic<std::chrono::steady_clock::duration> _startTime{std::chrono::steady_clock::duration::min()};

    std::chrono::milliseconds _resolution = std::chrono::milliseconds{2};
    std::chrono::milliseconds _warnTimeout = _defaultTimeout;
    std::chrono::milliseconds _errorTimeout = _defaultTimeout;

    std::function<void(std::chrono::milliseconds)> _warnHandler;
    std::function<void(std::chrono::milliseconds)> _errorHandler;
    
    std::thread _watchThread;
};
    
} // namespace Orchestration
} // namespace Services
} // namespace SilKit
