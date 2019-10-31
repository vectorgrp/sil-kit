// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "WatchDog.hpp"

#include <iostream>

using namespace std::chrono_literals;

namespace ib {
namespace mw {
namespace sync {

WatchDog::WatchDog()
{
    _warnHandler = [](std::chrono::milliseconds) {};
    _killHandler = [](std::chrono::milliseconds) {};
    _watchThread = std::thread{&WatchDog::Run, this};
}

WatchDog::~WatchDog()
{
    _stopPromise.set_value();
    _watchThread.join();
}

void WatchDog::Start()
{
    _startTime.store(std::chrono::steady_clock::now());
}

void WatchDog::Reset()
{
    _startTime.store(std::chrono::steady_clock::time_point::min());
}

void WatchDog::SetWarnHandler(std::function<void(std::chrono::milliseconds)> handler)
{
    _warnHandler = std::move(handler);
}

void WatchDog::SetKillHandler(std::function<void(std::chrono::milliseconds)> handler)
{
    _killHandler = std::move(handler);
}

void WatchDog::Run()
{
    const auto Resolution = 2ms;
    const auto warnDuration = 1005ms;
    const auto killDuration = 1500ms;

    enum class WatchDogState
    {
        Healthy,
        Warned,
        Killed
    };
    
    WatchDogState state = WatchDogState::Healthy;
    auto stopFuture = _stopPromise.get_future();
    
    while (true)
    {
        auto futureStatus = stopFuture.wait_for(Resolution);

        if (futureStatus == std::future_status::ready)
        {
            // stop was signaled; stopping thread;
            return;
        }
        
        auto startTime = _startTime.load();
        auto now = std::chrono::steady_clock::now();
        auto currentRunDuration = now - startTime;

        // We only communicate with the "main thread" via the atomic _startTime.
        // If _startTime is time_point::min(), Start() has not yet been called.
        // Otherwise, _startTime is the time point when the Start() was called.
        if (startTime == std::chrono::steady_clock::time_point::min())
        {
            // no job is currently running. Reset state and continue.
            state = WatchDogState::Healthy;
            continue;
        }


        if (currentRunDuration <= warnDuration)
        {
            state = WatchDogState::Healthy;
            continue;
        }
        if (currentRunDuration > warnDuration && currentRunDuration <= killDuration)
        {
            if (state == WatchDogState::Healthy)
            {
                _warnHandler(std::chrono::duration_cast<std::chrono::milliseconds>(currentRunDuration));
                state = WatchDogState::Warned;
            }
            continue;
        }

        if (currentRunDuration > killDuration)
        {
            if (state != WatchDogState::Killed)
            {
                _killHandler(std::chrono::duration_cast<std::chrono::milliseconds>(currentRunDuration));
                state = WatchDogState::Killed;
            }
            continue;
        }
    }
}

    
} // namespace sync
} // namespace mw
} // namespace ib
