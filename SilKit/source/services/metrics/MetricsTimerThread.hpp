// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IMetricsTimerThread.hpp"

#include <atomic>
#include <functional>
#include <future>
#include <thread>

namespace VSilKit {

class MetricsTimerThread : public IMetricsTimerThread
{
    std::atomic<bool> _started{false};
    std::promise<void> _go;
    std::promise<void> _done;
    std::function<void()> _callback;

    std::thread _thread;

public:
    explicit MetricsTimerThread(std::chrono::seconds interval, std::function<void()> callback);

    ~MetricsTimerThread() override;

    void Start() override;

private:
    auto MakeThread(std::chrono::seconds interval) -> std::thread;
};

} // namespace VSilKit