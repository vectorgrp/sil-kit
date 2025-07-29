// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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

    void WithPeriod(std::chrono::nanoseconds period, std::function<void(std::chrono::nanoseconds)> callback)
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
        return _isRunning;
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
