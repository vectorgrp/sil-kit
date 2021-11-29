// Copyright (c) Vector Informatik GmbH. All rights reserved.
#pragma once

#include <future>
#include <thread>
#include <functional>
#include <chrono>
#include "SetThreadName.hpp"

namespace ib {
namespace util {

class Timer
{
public:
    Timer() = default;

    ~Timer()
    {
        Stop();
        if (_thread.joinable())
        {
            _thread.join();
        }
    }

    void Stop()
    {
        if (_isRunning)
        {
            _isRunning = false;
            _promise.set_value();
        }
    }

    void WithPeriod(std::chrono::nanoseconds period,
        std::function<void(std::chrono::nanoseconds)> callback)
    {
        _period = period;
        if (_period <= std::chrono::nanoseconds{0})
        {
            return;
        }

        _callback = std::move(callback);
        if (!_callback)
        {
            return;
        }

        Stop();
        if (_thread.joinable())
        {
            _thread.join();
        }

        _isRunning = true;
        _promise = std::promise<void>{};
        _thread = std::move(std::thread{&Timer::ThreadMain, this,
            std::move(_promise.get_future())});

    }

    bool IsActive() const
    {
        return  _isRunning;
    }
private:
    // Methods
    void ThreadMain(std::future<void> future)
    {
        ib::util::SetThreadName("IB-Timer");
        while (_isRunning)
        {
            if (future.wait_for(_period) == std::future_status::timeout)
            {
                const auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
                _callback(now);
            }
        }
    }

private:
    // Members
    bool _isRunning{false};
    std::chrono::nanoseconds _period{0};
    std::thread _thread;
    std::function<void(std::chrono::nanoseconds)> _callback;
    std::promise<void> _promise;
};

} //end util
} //end ib
