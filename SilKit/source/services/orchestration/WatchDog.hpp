// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>
#include <future>
#include <chrono>
#include <memory>

#include "ParticipantConfiguration.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

class WatchDog
{
public:
    struct IClock
    {
        virtual ~IClock() = default;

        /// Returns the current time in nanoseconds since the start of the current epoch.
        virtual auto Now() const -> std::chrono::nanoseconds = 0;
    };

public:
    // ----------------------------------------
    // Constructors, Destructor, and Assignment
    WatchDog(const Config::HealthCheck& healthCheckConfig, IClock* clock = nullptr);
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
    /// Clock used for watchdog timing. Can be injected via the constructor.
    IClock* _clock;
    // we use a duration instead of a timepoint to avoid a bug in clang6 (up to v9.0)
    std::atomic<std::chrono::nanoseconds> _startTime{std::chrono::nanoseconds::min()};

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
