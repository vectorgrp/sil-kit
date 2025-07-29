// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include "ITimer.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


namespace VSilKit {


struct MockTimer : ITimer
{
    MOCK_METHOD(void, SetListener, (ITimerListener&), (override));
    MOCK_METHOD(void, AsyncWaitFor, (std::chrono::nanoseconds), (override));
    MOCK_METHOD(void, Shutdown, (), (override));
};


struct MockTimerListener : ITimerListener
{
    MOCK_METHOD(void, OnTimerExpired, (ITimer & timer), (override));
};


class MockTimerThatExpiresImmediately : public ITimer
{
    IIoContext* _ioContext{nullptr};
    ITimerListener* _listener{nullptr};
    bool _pending{false};

public:
    MockTimerThatExpiresImmediately(IIoContext& ioContext)
        : _ioContext{&ioContext}
    {
    }

    void SetListener(ITimerListener& listener) override
    {
        DoSetListener(listener);

        ASSERT_EQ(_listener, nullptr);
        _listener = &listener;
    }

    void AsyncWaitFor(std::chrono::nanoseconds duration) override
    {
        DoAsyncWaitFor(duration);

        _pending = true;

        _ioContext->Post([this] {
            if (!_pending)
            {
                return;
            }

            _pending = false;
            _listener->OnTimerExpired(*this);
        });
    }

    void Shutdown() override
    {
        DoShutdown();

        _pending = false;
    }

public:
    MOCK_METHOD(void, DoSetListener, (ITimerListener&));
    MOCK_METHOD(void, DoAsyncWaitFor, (std::chrono::nanoseconds));
    MOCK_METHOD(void, DoShutdown, ());
};


} // namespace VSilKit
