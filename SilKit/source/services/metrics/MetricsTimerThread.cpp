// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "MetricsTimerThread.hpp"

namespace VSilKit {

MetricsTimerThread::MetricsTimerThread(std::function<void()> callback)
    : _callback{std::move(callback)}
    , _thread{MakeThread()}
{
}

MetricsTimerThread::~MetricsTimerThread()
{
    if (!_started.exchange(true))
    {
        try
        {
            _go.set_value();
        }
        catch (const std::future_error &)
        {
        }
    }

    try
    {
        _done.set_value();
    }
    catch (const std::future_error &)
    {
    }

    if (_thread.joinable())
    {
        _thread.join();
    }
}

void MetricsTimerThread::Start()
{
    if (!_started.exchange(true))
    {
        try
        {
            _go.set_value();
        }
        catch (const std::future_error &)
        {
        }
    }
}

auto MetricsTimerThread::MakeThread() -> std::thread
{
    auto go = _go.get_future();
    auto done = _done.get_future();
    return std::thread{[go = std::move(go), done = std::move(done), callback = &_callback]() mutable {
        go.get();

        while (true)
        {
            if (done.wait_for(std::chrono::seconds{1}) != std::future_status::timeout)
            {
                break;
            }

            if (*callback)
            {
                (*callback)();
            }
        }
    }};
}

} // namespace VSilKit
