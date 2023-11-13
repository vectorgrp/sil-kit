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
