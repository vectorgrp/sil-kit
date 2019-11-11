// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <atomic>
#include <future>
#include <chrono>

namespace ib {
namespace mw {
namespace sync {

class WatchDog
{
public:
    // ----------------------------------------
    // Constructors, Destructor, and Assignment
    WatchDog(std::chrono::milliseconds warnTimeout, std::chrono::milliseconds errorTimeout);
    ~WatchDog();

public:
    // ----------------------------------------
    // Public Methods
    void Start();
    void Reset();

    void SetWarnHandler(std::function<void(std::chrono::milliseconds)> handler);
    void SetErrorHandler(std::function<void(std::chrono::milliseconds)> handler);

private:
    // ----------------------------------------
    // private methods
    void Run();

private:
    // ----------------------------------------
    // private members
    std::promise<void> _stopPromise;
    // we use a duration instead of a timepoint to avoid a bug in clang6 (up to v9.0)
    std::atomic<std::chrono::steady_clock::duration> _startTime{std::chrono::steady_clock::duration::min()};

    std::chrono::milliseconds _resolution = std::chrono::milliseconds{2};
    std::chrono::milliseconds _warnTimeout = std::chrono::milliseconds::max();
    std::chrono::milliseconds _errorTimeout = std::chrono::milliseconds::max();

    std::function<void(std::chrono::milliseconds)> _warnHandler;
    std::function<void(std::chrono::milliseconds)> _errorHandler;
    
    std::thread _watchThread;
};

    
} // namespace sync
} // namespace mw
} // namespace ib
