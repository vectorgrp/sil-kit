// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include "IRawByteStream.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"


namespace VSilKit {


/// Implementation of IRawByteStream that provides most methods as mock-methods.
///
/// The methods GetLocalEndpoint and GetRemoteEndpoint are not mock-methods, to enable their usage in matchers. See
/// https://github.com/google/googletest/issues/3016.
struct MockRawByteStream : IRawByteStream
{
    std::string localEndpoint;
    std::string remoteEndpoint;

    auto GetLocalEndpoint() const -> std::string override
    {
        return localEndpoint;
    }

    auto GetRemoteEndpoint() const -> std::string override
    {
        return remoteEndpoint;
    }

    MOCK_METHOD(void, SetListener, (IRawByteStreamListener&), (override));
    MOCK_METHOD(void, AsyncReadSome, (MutableBufferSequence), (override));
    MOCK_METHOD(void, AsyncWriteSome, (ConstBufferSequence), (override));
    MOCK_METHOD(void, Shutdown, (), (override));
};


struct MockRawByteStreamListener : IRawByteStreamListener
{
    MOCK_METHOD(void, OnAsyncReadSomeDone, (VSilKit::IRawByteStream&, size_t), (override));
    MOCK_METHOD(void, OnAsyncWriteSomeDone, (VSilKit::IRawByteStream&, size_t), (override));
    MOCK_METHOD(void, OnShutdown, (VSilKit::IRawByteStream&), (override));
};


} // namespace VSilKit
