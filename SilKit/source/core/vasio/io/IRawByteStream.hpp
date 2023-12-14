// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include "util/Buffer.hpp"


namespace VSilKit {


struct IIoContext;
struct IRawByteStreamListener;


struct IRawByteStream
{
    virtual ~IRawByteStream() = default;

    virtual void SetListener(IRawByteStreamListener& listener) = 0;

    virtual auto GetLocalEndpoint() const -> std::string = 0;

    virtual auto GetRemoteEndpoint() const -> std::string = 0;

    virtual void AsyncReadSome(MutableBufferSequence bufferSequence) = 0;

    virtual void AsyncWriteSome(ConstBufferSequence bufferSequence) = 0;

    virtual void Shutdown() = 0;
};


struct IRawByteStreamListener
{
    virtual ~IRawByteStreamListener() = default;

    virtual void OnAsyncReadSomeDone(IRawByteStream& stream, size_t bytesTransferred) = 0;

    virtual void OnAsyncWriteSomeDone(IRawByteStream& stream, size_t bytesTransferred) = 0;

    virtual void OnShutdown(IRawByteStream& stream) = 0;
};


} // namespace VSilKit


namespace SilKit {
namespace Core {
using VSilKit::IRawByteStream;
using VSilKit::IRawByteStreamListener;
} // namespace Core
} // namespace SilKit
