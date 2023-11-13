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


#include "IConnector.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


namespace VSilKit {


struct MockConnector : IConnector
{
    MOCK_METHOD(void, SetListener, (IConnectorListener&), (override));
    MOCK_METHOD(void, AsyncConnect, (std::chrono::milliseconds), (override));
    MOCK_METHOD(void, Shutdown, (), (override));
};


struct MockConnectorListener : IConnectorListener
{
    MOCK_METHOD(void, OnAsyncConnectSuccess, (IConnector&, std::unique_ptr<IRawByteStream>), (override));
    MOCK_METHOD(void, OnAsyncConnectFailure, (IConnector&), (override));
};


class MockConnectorThatFails : public IConnector
{
    IIoContext* _ioContext{nullptr};
    IConnectorListener* _listener{nullptr};
    bool _pending{false};

public:
    explicit MockConnectorThatFails(IIoContext& ioContext)
        : _ioContext{&ioContext}
    {
    }

public:
    void SetListener(IConnectorListener& listener) override
    {
        DoSetListener(listener);

        ASSERT_EQ(_listener, nullptr);
        _listener = &listener;
    }

    void AsyncConnect(std::chrono::milliseconds timeout) override
    {
        DoAsyncConnect(timeout);

        _pending = true;

        _ioContext->Post([this] {
            if (!_pending)
            {
                return;
            }

            _pending = false;
            _listener->OnAsyncConnectFailure(*this);
        });
    }

    void Shutdown() override
    {
        DoShutdown();

        _ioContext->Dispatch([this] {
            if (!_pending)
            {
                return;
            }

            _pending = false;
            _listener->OnAsyncConnectFailure(*this);
        });
    }

public:
    MOCK_METHOD(void, DoSetListener, (IConnectorListener&));
    MOCK_METHOD(void, DoAsyncConnect, (std::chrono::milliseconds));
    MOCK_METHOD(void, DoShutdown, ());
};


class MockConnectorThatSucceeds : public IConnector
{
    IIoContext* _ioContext{nullptr};
    IConnectorListener* _listener{nullptr};
    bool _pending{false};

public:
    explicit MockConnectorThatSucceeds(IIoContext& ioContext)
        : _ioContext{&ioContext}
    {
    }

public:
    void SetListener(IConnectorListener& listener) override
    {
        DoSetListener(listener);

        ASSERT_EQ(_listener, nullptr);
        _listener = &listener;
    }

    void AsyncConnect(std::chrono::milliseconds timeout) override
    {
        DoAsyncConnect(timeout);

        _pending = true;

        _ioContext->Post([this] {
            if (!_pending)
            {
                return;
            }

            _pending = false;
            _listener->OnAsyncConnectSuccess(*this, MakeRawByteStream());
        });
    }

    void Shutdown() override
    {
        DoShutdown();

        _ioContext->Dispatch([this] {
            if (!_pending)
            {
                return;
            }

            _pending = false;
            _listener->OnAsyncConnectFailure(*this);
        });
    }

public:
    MOCK_METHOD(void, DoSetListener, (IConnectorListener&));
    MOCK_METHOD(void, DoAsyncConnect, (std::chrono::milliseconds));
    MOCK_METHOD(void, DoShutdown, ());

    MOCK_METHOD(std::unique_ptr<IRawByteStream>, MakeRawByteStream, ());
};


} // namespace VSilKit
