// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IConnector.hpp"
#include "IIoContext.hpp"

#include "AsioCleanupEndpoint.hpp"
#include "AsioGenericRawByteStream.hpp"
#include "AsioFormatEndpoint.hpp"
#include "SetAsioSocketOptions.hpp"

#include "AsioSocketOptions.hpp"
#include "util/Atomic.hpp"
#include "util/Exceptions.hpp"
#include "util/TracingMacros.hpp"

#include "ILogger.hpp"

#include <memory>
#include <mutex>

#include "asio.hpp"


#if SILKIT_ENABLE_TRACING_INSTRUMENTATION_AsioConnector
#    define SILKIT_TRACE_METHOD_(logger, ...) SILKIT_TRACE_METHOD(logger, __VA_ARGS__)
#else
#    define SILKIT_TRACE_METHOD_(...)
#endif


namespace VSilKit {


template <typename T>
class AsioConnector final : public IConnector
{
    using AsioProtocolType = T;
    using AsioSocketType = typename AsioProtocolType::socket;
    using AsioEndpointType = typename AsioProtocolType::endpoint;

    enum State
    {
        IDLE,
        PENDING,
        CONNECTED,
        TIMED_OUT,
    };

    class Op : public std::enable_shared_from_this<Op>
    {
        std::atomic<AsioConnector*> _parent;

        std::weak_ptr<asio::io_context> _asioIoContext;
        AsioSocketOptions _asioSocketOptions;
        AsioEndpointType _remoteEndpoint;

        AtomicEnum<State> _state{IDLE};

        AsioSocketType _socket;
        asio::cancellation_signal _connectCancelSignal;

        asio::steady_timer _timeoutTimer;
        asio::cancellation_signal _timeoutCancelSignal;

        SilKit::Services::Logging::ILogger* _logger{nullptr};

    public:
        Op(AsioConnector& connector, const AsioSocketOptions& asioSocketOptions,
           const AsioEndpointType& remoteEndpoint);

        void Initiate(std::chrono::milliseconds timeout);
        void Shutdown();
        void Abandon();

    private:
        void HandleSuccess(std::unique_ptr<IRawByteStream> stream);
        void HandleFailure();

    private:
        void OnAsioAsyncConnectComplete(const asio::error_code& asioErrorCode);
        void OnAsioAsyncWaitComplete(const asio::error_code& asioErrorCode);
    };

    std::shared_ptr<asio::io_context> _asioIoContext;
    SilKit::Services::Logging::ILogger* _logger{nullptr};

    IConnectorListener* _listener{nullptr};

    std::shared_ptr<Op> _op;

public:
    AsioConnector(std::shared_ptr<asio::io_context> asioIoContext, const AsioSocketOptions& socketOptions,
                  const AsioEndpointType& remoteEndpoint, SilKit::Services::Logging::ILogger& logger);
    ~AsioConnector() override;

public: // IAcceptor
    void SetListener(IConnectorListener& listener) override;
    void AsyncConnect(std::chrono::milliseconds timeout) override;
    void Shutdown() override;
};


template <typename T>
AsioConnector<T>::AsioConnector(std::shared_ptr<asio::io_context> asioIoContext, const AsioSocketOptions& socketOptions,
                                const AsioEndpointType& remoteEndpoint, SilKit::Services::Logging::ILogger& logger)
    : _asioIoContext{std::move(asioIoContext)}
    , _logger{&logger}
    , _op{std::make_shared<Op>(*this, socketOptions, remoteEndpoint)}
{
    SILKIT_TRACE_METHOD_(_logger, "(...)");
}


template <typename T>
AsioConnector<T>::~AsioConnector()
{
    SILKIT_TRACE_METHOD_(_logger, "()");

    _op->Abandon();
    _op->Shutdown();
}


template <typename T>
void AsioConnector<T>::SetListener(IConnectorListener& listener)
{
    SILKIT_TRACE_METHOD_(_logger, "({})", static_cast<const void*>(&listener));

    _listener = &listener;
}


template <typename T>
void AsioConnector<T>::AsyncConnect(std::chrono::milliseconds timeout)
{
    SILKIT_TRACE_METHOD_(_logger, "({}ms)", timeout.count());

    _op->Initiate(timeout);
}


template <typename T>
void AsioConnector<T>::Shutdown()
{
    SILKIT_TRACE_METHOD_(_logger, "()");

    _op->Shutdown();
}


template <typename T>
AsioConnector<T>::Op::Op(AsioConnector& connector, const AsioSocketOptions& asioSocketOptions,
                         const AsioEndpointType& remoteEndpoint)
    : _parent{&connector}
    , _asioIoContext{connector._asioIoContext}
    , _asioSocketOptions{asioSocketOptions}
    , _remoteEndpoint{remoteEndpoint}
    , _socket{*connector._asioIoContext, _remoteEndpoint.protocol()}
    , _timeoutTimer{*connector._asioIoContext}
    , _logger{connector._logger}
{
    SILKIT_TRACE_METHOD_(_logger, "(...)");
}


template <typename T>
void AsioConnector<T>::Op::Initiate(std::chrono::milliseconds timeout)
{
    SILKIT_TRACE_METHOD_(_logger, "({})", timeout.count());

    if (!_state.ExchangeIfExpected(IDLE, PENDING))
    {
        throw InvalidStateError{};
    }

    std::error_code errorCode;

    SetAsioSocketOptions(_logger, _socket, _asioSocketOptions, errorCode);
    if (errorCode)
    {
        SILKIT_TRACE_METHOD_(_logger, "failed to set socket options: {}", errorCode.message());
        HandleFailure();
        return;
    }

    auto connectCompletionHandler =
        asio::bind_cancellation_slot(_connectCancelSignal.slot(), [self = this->shared_from_this()](const auto& e) {
            self->OnAsioAsyncConnectComplete(e);
        });

    if (timeout.count() > 0)
    {
        auto timeoutCompletionHandler =
            asio::bind_cancellation_slot(_timeoutCancelSignal.slot(), [self = this->shared_from_this()](const auto& e) {
                self->OnAsioAsyncWaitComplete(e);
            });

        _timeoutTimer.expires_after(timeout);
        _timeoutTimer.async_wait(timeoutCompletionHandler);
    }

    _socket.async_connect(_remoteEndpoint, connectCompletionHandler);
}


template <typename T>
void AsioConnector<T>::Op::Shutdown()
{
    SILKIT_TRACE_METHOD_(_logger, "()");

    _socket.close();
    _timeoutTimer.cancel();
}


template <typename T>
void AsioConnector<T>::Op::Abandon()
{
    SILKIT_TRACE_METHOD_(_logger, "()");

    _parent = nullptr;
}


template <typename T>
void AsioConnector<T>::Op::HandleSuccess(std::unique_ptr<IRawByteStream> stream)
{
    SILKIT_TRACE_METHOD_(_logger, "({})", static_cast<const void*>(stream.get()));

    auto* connector{_parent.load()};
    if (connector == nullptr)
    {
        return;
    }

    connector->_listener->OnAsyncConnectSuccess(*connector, std::move(stream));
}


template <typename T>
void AsioConnector<T>::Op::HandleFailure()
{
    SILKIT_TRACE_METHOD_(_logger, "()");

    auto* connector{_parent.load()};
    if (connector == nullptr)
    {
        return;
    }

    connector->_listener->OnAsyncConnectFailure(*connector);
}


template <typename T>
void AsioConnector<T>::Op::OnAsioAsyncConnectComplete(const asio::error_code& asioErrorCode)
{
    SILKIT_TRACE_METHOD_(_logger, "({}, ...)", asioErrorCode.message());

    if (!_state.ExchangeIfExpected(PENDING, CONNECTED))
    {
        throw InvalidStateError{};
    }

    if (asioErrorCode)
    {
        HandleFailure();
        return;
    }

    auto asioIoContext{_asioIoContext.lock()};
    if (asioIoContext == nullptr)
    {
        return;
    }

    using std::swap;

    AsioSocketType socket{*asioIoContext};
    swap(_socket, socket);

    const auto family{socket.local_endpoint().protocol().family()};
    const bool isTcp{family == asio::ip::tcp::v4().family() || family == asio::ip::tcp::v6().family()};

    AsioGenericRawByteStreamOptions options{};
    options.tcp.quickAck = isTcp && _asioSocketOptions.tcp.quickAck;

    auto stream{
        std::make_unique<AsioGenericRawByteStream>(options, std::move(asioIoContext), std::move(socket), *_logger)};

    _timeoutCancelSignal.emit(asio::cancellation_type::total);
    HandleSuccess(std::move(stream));
}


template <typename T>
void AsioConnector<T>::Op::OnAsioAsyncWaitComplete(const asio::error_code& errorCode)
{
    SILKIT_TRACE_METHOD_(_logger, "({})", errorCode.message());

    if (errorCode)
    {
        return;
    }

    if (_state.Get() == PENDING)
    {
        _connectCancelSignal.emit(asio::cancellation_type::total);
    }
}


} // namespace VSilKit


#undef SILKIT_TRACE_METHOD_
