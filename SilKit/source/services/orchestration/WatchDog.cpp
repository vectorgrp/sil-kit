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

#include <iostream>

#include "WatchDog.hpp"
#include "SetThreadName.hpp"

using namespace std::chrono_literals;

namespace {

auto GetDefaultClock() -> SilKit::Services::Orchestration::WatchDog::IClock*;

} // namespace

namespace SilKit {
namespace Services {
namespace Orchestration {

WatchDog::WatchDog(const Config::HealthCheck& healthCheckConfig, IClock* clock)
    : _clock{clock ? clock : GetDefaultClock()}
    , _warnHandler{[](std::chrono::milliseconds) {}}
    , _errorHandler{[](std::chrono::milliseconds) {}}
{
    if (healthCheckConfig.softResponseTimeout.has_value())
    {
        _warnTimeout = healthCheckConfig.softResponseTimeout.value();
        if (_warnTimeout <= 0ms)
            throw SilKitError{"WatchDog requires warnTimeout > 0ms"};
    }

    if (healthCheckConfig.hardResponseTimeout.has_value())
    {
        _errorTimeout = healthCheckConfig.hardResponseTimeout.value();
        if (_errorTimeout <= 0ms)
            throw SilKitError{"WatchDog requires errorTimeout > 0ms"};
    }
    _watchThread = std::thread{&WatchDog::Run, this};
}

WatchDog::~WatchDog()
{
    _stopPromise.set_value();
    _watchThread.join();
}

void WatchDog::Start()
{
    _startTime.store(_clock->Now());
}

void WatchDog::Reset()
{
    _startTime.store(std::chrono::nanoseconds::min());
}

void WatchDog::SetWarnHandler(std::function<void(std::chrono::milliseconds)> handler)
{
    _warnHandler = std::move(handler);
}

void WatchDog::SetErrorHandler(std::function<void(std::chrono::milliseconds)> handler)
{
    _errorHandler = std::move(handler);
}

void WatchDog::Run()
{
    enum class WatchDogState
    {
        Healthy,
        Warn,
        Error
    };

    SilKit::Util::SetThreadName("SilKit-Watchdog");
    WatchDogState state = WatchDogState::Healthy;
    auto stopFuture = _stopPromise.get_future();

    while (true)
    {
        auto futureStatus = stopFuture.wait_for(_resolution);

        if (futureStatus == std::future_status::ready)
        {
            // stop was signaled; stopping thread;
            return;
        }

        const auto startTime = _startTime.load();

        // We only communicate with the "main thread" via the atomic _startTime.
        // If _startTime is duration::min(), Start() has not yet been called.
        // Otherwise, _startTime is the duration since epoch when the Start() was called.
        if (startTime == std::chrono::nanoseconds::min())
        {
            // no job is currently running. Reset state and continue.
            state = WatchDogState::Healthy;
            continue;
        }

        // These declarations are after the startTime check to prevent integer overflow
        // by deferring arithmetic on duration::min() until Start() was called.
        const auto now = _clock->Now();
        const auto currentRunDuration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);

        if (currentRunDuration > _warnTimeout && currentRunDuration <= _errorTimeout)
        {
            if (state == WatchDogState::Healthy)
            {
                _warnHandler(currentRunDuration);
                state = WatchDogState::Warn;
            }
            continue;
        }

        if (currentRunDuration > _errorTimeout)
        {
            if (state != WatchDogState::Error)
            {
                _errorHandler(currentRunDuration);
                state = WatchDogState::Error;
            }
            continue;
        }

        // If neither warning, nor error timeouts were hit, the state is healthy.
        state = WatchDogState::Healthy;
    }
}

// For testing purposes only
std::chrono::milliseconds WatchDog::GetWarnTimeout()
{
    return _warnTimeout;
}

std::chrono::milliseconds WatchDog::GetErrorTimeout()
{
    return _errorTimeout;
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit

namespace {

struct SteadyClock : public SilKit::Services::Orchestration::WatchDog::IClock
{
    auto Now() const -> std::chrono::nanoseconds override
    {
        return std::chrono::steady_clock::now().time_since_epoch();
    }
};

auto GetDefaultClock() -> SilKit::Services::Orchestration::WatchDog::IClock*
{
    static SteadyClock steadyClock{};
    return &steadyClock;
}

} // namespace
