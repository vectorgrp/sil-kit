// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "AsioGenericRawByteStream.hpp"

#include "AsioFormatEndpoint.hpp"
#include "IIoContext.hpp"

#include "util/Atomic.hpp"
#include "util/Exceptions.hpp"
#include "util/TracingMacros.hpp"

#include <algorithm>
#include <vector>

#include <cassert>

#include "asio.hpp"


// For EnableQuickAck
#if defined(__linux__)
#    include <sys/types.h>
#    include <sys/socket.h>
#    include <errno.h>
#endif


#if SILKIT_ENABLE_TRACING_INSTRUMENTATION_AsioGenericRawByteStream
#    define SILKIT_TRACE_METHOD_(logger, ...) SILKIT_TRACE_METHOD(logger, __VA_ARGS__)
#else
#    define SILKIT_TRACE_METHOD_(...)
#endif


namespace {


namespace Log = SilKit::Services::Logging;

auto IsErrorToTryAgain(const asio::error_code& ec) -> bool
{
    return ec == asio::error::no_descriptors //
           || ec == asio::error::no_buffer_space //
           || ec == asio::error::no_memory //
           || ec == asio::error::timed_out //
           || ec == asio::error::try_again;
}


} // namespace


namespace VSilKit {


AsioGenericRawByteStream::AsioGenericRawByteStream(const AsioGenericRawByteStreamOptions& options,
                                                   std::shared_ptr<asio::io_context> asioIoContext, AsioSocket socket,
                                                   SilKit::Services::Logging::ILogger& logger)
    : _options{options}
    , _asioIoContext{std::move(asioIoContext)}
    , _socket{std::move(socket)}
    , _logger{&logger}
{
    SILKIT_TRACE_METHOD_(_logger, "(...)");

    EnableQuickAck();
}


AsioGenericRawByteStream::~AsioGenericRawByteStream()
{
    SILKIT_TRACE_METHOD_(_logger, "()");
}


void AsioGenericRawByteStream::SetListener(IRawByteStreamListener& listener)
{
    SILKIT_TRACE_METHOD_(_logger, "({})", static_cast<const void*>(&listener));

    _listener = &listener;
}


auto AsioGenericRawByteStream::GetLocalEndpoint() const -> std::string
{
    const auto endpoint{_socket.local_endpoint()};
    return FormatEndpoint(endpoint);
}


auto AsioGenericRawByteStream::GetRemoteEndpoint() const -> std::string
{
    const auto endpoint{_socket.remote_endpoint()};
    return FormatEndpoint(endpoint);
}


void AsioGenericRawByteStream::AsyncReadSome(MutableBufferSequence bufferSequence)
{
    SILKIT_TRACE_METHOD_(_logger, "(...)");

    {
        std::unique_lock<decltype(_mutex)> lock{_mutex};

        if (_shutdownPending)
        {
            SILKIT_TRACE_METHOD_(_logger, "ignored, already shutting down");
            return;
        }

        if (_reading)
        {
            throw InvalidStateError{};
        }
        else
        {
            _reading = true;
        }

        _readBufferSequence.resize(bufferSequence.size());
        std::transform(bufferSequence.begin(), bufferSequence.end(), _readBufferSequence.begin(),
                       [](const MutableBuffer& buffer) -> asio::mutable_buffer {
                           return asio::mutable_buffer{buffer.GetData(), buffer.GetSize()};
                       });

        _socket.async_read_some(_readBufferSequence, [this](const auto& e, auto s) {
            OnAsioAsyncReadSomeComplete(e, s);
        });
    }
}


void AsioGenericRawByteStream::AsyncWriteSome(ConstBufferSequence bufferSequence)
{
    SILKIT_TRACE_METHOD_(_logger, "(...)");

    {
        std::unique_lock<decltype(_mutex)> lock{_mutex};

        if (_shutdownPending)
        {
            SILKIT_TRACE_METHOD_(_logger, "ignored, already shutting down");
            return;
        }

        if (_writing)
        {
            throw InvalidStateError{};
        }
        else
        {
            _writing = true;
        }

        _writeBufferSequence.resize(bufferSequence.size());
        std::transform(bufferSequence.begin(), bufferSequence.end(), _writeBufferSequence.begin(),
                       [](const ConstBuffer& buffer) -> asio::const_buffer {
                           return asio::const_buffer{buffer.GetData(), buffer.GetSize()};
                       });

        _socket.async_write_some(_writeBufferSequence, [this](const auto& e, auto s) {
            OnAsioAsyncWriteSomeComplete(e, s);
        });
    }
}


void AsioGenericRawByteStream::Shutdown()
{
    SILKIT_TRACE_METHOD_(_logger, "()");

    {
        std::unique_lock<decltype(_mutex)> lock{_mutex};
        HandleShutdownOrError();
    }
}


void AsioGenericRawByteStream::OnAsioAsyncReadSomeComplete(asio::error_code const& errorCode, size_t bytesTransferred)
{
    SILKIT_TRACE_METHOD_(_logger, "({}, {})", errorCode.message(), bytesTransferred);

    {
        std::unique_lock<decltype(_mutex)> lock{_mutex};

        if (_reading)
        {
            _reading = false;
        }
        else
        {
            throw InvalidStateError{};
        }

        if (_shutdownPending || (errorCode && !IsErrorToTryAgain(errorCode)))
        {
            HandleShutdownOrError();
            return;
        }

        EnableQuickAck();

        if (errorCode && bytesTransferred == 0)
        {
            // only re-trigger the read if no bytes were transferred, otherwise treat it as a 'normal' completion

            _reading = true;
            _socket.async_read_some(_readBufferSequence, [this](const auto& e, auto s) {
                OnAsioAsyncReadSomeComplete(e, s);
            });

            return;
        }
    }

    _listener->OnAsyncReadSomeDone(*this, bytesTransferred);
}


void AsioGenericRawByteStream::OnAsioAsyncWriteSomeComplete(asio::error_code const& errorCode, size_t bytesTransferred)
{
    SILKIT_TRACE_METHOD_(_logger, "({}, {})", errorCode.message(), bytesTransferred);

    {
        std::unique_lock<decltype(_mutex)> lock{_mutex};

        if (_writing)
        {
            _writing = false;
        }
        else
        {
            throw InvalidStateError{};
        }

        if (_shutdownPending || (errorCode && !IsErrorToTryAgain(errorCode)))
        {
            HandleShutdownOrError();
            return;
        }

        if (errorCode && bytesTransferred == 0)
        {
            // only re-trigger the write if no bytes were transferred, otherwise treat it as a 'normal' completion

            _writing = true;
            _socket.async_write_some(_writeBufferSequence, [this](const auto& e, auto s) {
                OnAsioAsyncWriteSomeComplete(e, s);
            });

            return;
        }
    }

    _listener->OnAsyncWriteSomeDone(*this, bytesTransferred);
}


void AsioGenericRawByteStream::HandleShutdownOrError()
{
    SILKIT_TRACE_METHOD_(_logger, "() [shutdownPosted={}, shutdownPending={}, reading={}, writing={}]", _shutdownPending,
                        _shutdownPosted, _reading, _writing);

    if (!_shutdownPending)
    {
        _shutdownPending = true;

        asio::error_code errorCode;

        SILKIT_TRACE_METHOD_(_logger, "closing underlying socket");

        _socket.close(errorCode);
        if (errorCode)
        {
            Log::Warn(_logger, "AsioGenericRawByteStream::HandleShutdownOrError: socket close failed: {}",
                      errorCode.message());
        }
    }

    if (!_reading && !_writing)
    {
        if (!_shutdownPosted)
        {
            SILKIT_TRACE_METHOD_(_logger, "posting shutdown on listener {}", static_cast<const void*>(_listener));

            _shutdownPosted = true;

            _asioIoContext->post([this] {
                _listener->OnShutdown(*this);
            });
        }
    }
}


#if defined(__linux__)

void AsioGenericRawByteStream::EnableQuickAck()
{
    if (!_options.tcp.quickAck)
    {
        return;
    }

    int val{1};

    // Disable Delayed Acknowledgments on the receiving side
    int e = setsockopt(_socket.native_handle(), IPPROTO_TCP, TCP_QUICKACK, (void*)&val, sizeof(val));
    if (e != 0)
    {
        Log::Warn(
            _logger,
            "AsioGenericRawByteStream({})::EnableQuickAck: failed to set Linux-specific socket option 'TCP_QUICKACK'",
            static_cast<const void*>(this));
    }
}

#else

void AsioGenericRawByteStream::EnableQuickAck()
{
    SILKIT_UNUSED_ARG(_options);
}

#endif


} // namespace VSilKit


#undef SILKIT_TRACE_METHOD_
