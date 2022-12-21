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

#include <future>
#include <thread>
#include <functional>
#include <chrono>

#include "SetThreadName.hpp"

namespace SilKit {
namespace Util {

class Timer
{
public:
    Timer() = default;

    Timer(Timer&& other) noexcept
        : _isRunning{other._isRunning.load()}
        , _m{std::move(other._m)}
    {
    }

    Timer& operator=(Timer&& other) noexcept
    {
        if (this != &other)
        {
            _isRunning = other._isRunning.load();
            _m = std::move(other._m);
        }

        return *this;
    }

    ~Timer()
    {
        Stop();
        if (_m.thread.joinable())
        {
            _m.thread.join();
        }
    }

public:
    void Stop()
    {
        if (_isRunning)
        {
            _isRunning = false;
            _m.promise.set_value();
        }
    }

    void WithPeriod(std::chrono::nanoseconds period,
        std::function<void(std::chrono::nanoseconds)> callback)
    {
        _m.period = period;
        if (_m.period <= std::chrono::nanoseconds{0})
        {
            return;
        }

        _m.callback = std::move(callback);
        if (!_m.callback)
        {
            return;
        }

        Stop();
        if (_m.thread.joinable())
        {
            _m.thread.join();
        }

        _isRunning = true;
        _m.promise = std::promise<void>{};
        _m.thread = std::thread{&Timer::ThreadMain, this, _m.promise.get_future()};
    }

    bool IsActive() const
    {
        return  _isRunning;
    }

private:
    // Methods
    void ThreadMain(std::future<void> future)
    {
        SilKit::Util::SetThreadName("SilKit-Timer");
        while (_isRunning)
        {
            if (future.wait_for(_m.period) == std::future_status::timeout)
            {
                const auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
                _m.callback(now);
            }
        }
    }

private:
    // Immovable Members
    std::atomic<bool> _isRunning{false};
    // Movable Members
    struct
    {
        std::chrono::nanoseconds period{0};
        std::thread thread;
        std::function<void(std::chrono::nanoseconds)> callback;
        std::promise<void> promise;
    } _m;
};

} // namespace Util
} // namespace SilKit
