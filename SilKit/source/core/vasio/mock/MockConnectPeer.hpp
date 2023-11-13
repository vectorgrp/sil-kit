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


#include "IConnectPeer.hpp"

#include "Uri.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


namespace SilKit {
namespace Core {


struct MockConnectPeer : IConnectPeer
{
    MOCK_METHOD(void, SetListener, (IConnectPeerListener&), (override));
    MOCK_METHOD(void, AsyncConnect, (size_t, std::chrono::milliseconds), (override));
    MOCK_METHOD(void, Shutdown, (), (override));
};


struct MockConnectPeerListener : IConnectPeerListener
{
    MOCK_METHOD(void, OnConnectPeerSuccess, (IConnectPeer&, VAsioPeerInfo, std::unique_ptr<IRawByteStream>),
                (override));
    MOCK_METHOD(void, OnConnectPeerFailure, (IConnectPeer&, VAsioPeerInfo), (override));
};


/// IConnectPeer implementation that succeeds if any acceptor URIs are present in the passed peer info.
///
/// The first acceptor URI is passed to the mock-method MakeRawByteStream. The result of this call is returned in the
/// success handler.
///
/// If no acceptor URIs are present, the failure handler is invoked.
class MockConnectPeerThatSucceeds : public IConnectPeer
{
    IIoContext* _ioContext{nullptr};
    VAsioPeerInfo _peerInfo;
    IConnectPeerListener* _listener{nullptr};
    bool _pending{false};

public:
    explicit MockConnectPeerThatSucceeds(IIoContext& ioContext, VAsioPeerInfo peerInfo)
        : _ioContext{&ioContext}
        , _peerInfo{std::move(peerInfo)}
    {
    }

public:
    void SetListener(IConnectPeerListener& listener) override
    {
        DoSetListener(listener);

        ASSERT_EQ(_listener, nullptr);
        _listener = &listener;
    }

    void AsyncConnect(size_t numberOfAttempts, std::chrono::milliseconds timeout) override
    {
        DoAsyncConnect(numberOfAttempts, timeout);

        _pending = true;

        _ioContext->Post([this] {
            if (!_pending)
            {
                return;
            }

            _pending = false;
            if (_peerInfo.acceptorUris.empty())
            {
                _listener->OnConnectPeerFailure(*this, _peerInfo);
            }
            else
            {
                _listener->OnConnectPeerSuccess(*this, _peerInfo, MakeRawByteStream(_peerInfo.acceptorUris.front()));
            }
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
            _listener->OnConnectPeerFailure(*this, _peerInfo);
        });
    }

public:
    MOCK_METHOD(void, DoSetListener, (IConnectPeerListener&));
    MOCK_METHOD(void, DoAsyncConnect, (size_t, std::chrono::milliseconds));
    MOCK_METHOD(void, DoShutdown, ());

    MOCK_METHOD(std::unique_ptr<IRawByteStream>, MakeRawByteStream, (const std::string&));
};


/// IConnectPeer implementation that always fails.
class MockConnectPeerThatFails : public IConnectPeer
{
    IIoContext* _ioContext{nullptr};
    VAsioPeerInfo _peerInfo;
    IConnectPeerListener* _listener{nullptr};
    bool _pending{false};

public:
    explicit MockConnectPeerThatFails(IIoContext& ioContext, VAsioPeerInfo peerInfo)
        : _ioContext{&ioContext}
        , _peerInfo{std::move(peerInfo)}
    {
    }

public:
    void SetListener(IConnectPeerListener& listener) override
    {
        DoSetListener(listener);

        ASSERT_EQ(_listener, nullptr);
        _listener = &listener;
    }

    void AsyncConnect(size_t numberOfAttempts, std::chrono::milliseconds timeout) override
    {
        DoAsyncConnect(numberOfAttempts, timeout);

        _pending = true;

        _ioContext->Post([this] {
            if (!_pending)
            {
                return;
            }

            _pending = false;
            _listener->OnConnectPeerFailure(*this, _peerInfo);
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
            _listener->OnConnectPeerFailure(*this, _peerInfo);
        });
    }

public:
    MOCK_METHOD(void, DoSetListener, (IConnectPeerListener&));
    MOCK_METHOD(void, DoAsyncConnect, (size_t, std::chrono::milliseconds));
    MOCK_METHOD(void, DoShutdown, ());

    MOCK_METHOD(std::unique_ptr<IRawByteStream>, MakeRawByteStream, (const std::string&));
};


} // namespace Core
} // namespace SilKit
