// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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
