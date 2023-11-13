// Copyright (c) 2022 Vector Informatik GmbH
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
