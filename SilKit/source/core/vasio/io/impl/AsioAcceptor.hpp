// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IAcceptor.hpp"
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

#include "asio.hpp"


#if SILKIT_ENABLE_TRACING_INSTRUMENTATION_AsioAcceptor
#    define SILKIT_TRACE_METHOD_(logger, ...) SILKIT_TRACE_METHOD(logger, __VA_ARGS__)
#else
#    define SILKIT_TRACE_METHOD_(...)
#endif


namespace VSilKit {


template <typename T>
class AsioAcceptor final : public IAcceptor
{
    using AsioAcceptorType = T;
    using AsioEndpointType = typename AsioAcceptorType::protocol_type::endpoint;
    using AsioSocketType = typename AsioAcceptorType::protocol_type::socket;

    enum State
    {
        IDLE,
        PENDING,
    };

    IAcceptorListener* _listener{nullptr};

    AtomicEnum<State> _state{IDLE};

    AsioSocketOptions _socketOptions;

    std::shared_ptr<asio::io_context> _asioIoContext;

    AsioAcceptorType _acceptor;
    asio::cancellation_signal _acceptCancelSignal;

    asio::steady_timer _timeoutTimer;
    asio::cancellation_signal _timeoutCancelSignal;

    AsioEndpointType _localEndpoint;

    SilKit::Services::Logging::ILogger* _logger{nullptr};

public:
    AsioAcceptor(const AsioSocketOptions& socketOptions, std::shared_ptr<asio::io_context> asioIoContext,
                 AsioAcceptorType acceptor, SilKit::Services::Logging::ILogger& logger);
    ~AsioAcceptor() override;

public: // IAcceptor
    void SetListener(IAcceptorListener& listener) override;
    auto GetLocalEndpoint() const -> std::string override;
    void AsyncAccept(std::chrono::milliseconds timeout) override;
    void Shutdown() override;

private:
    void OnAsioAsyncAcceptComplete(const asio::error_code& asioErrorCode, AsioSocketType socket);
    void OnAsioAsyncWaitComplete(const asio::error_code& errorCode);
};


template <typename T>
AsioAcceptor<T>::AsioAcceptor(const AsioSocketOptions& socketOptions, std::shared_ptr<asio::io_context> asioIoContext,
                              AsioAcceptorType acceptor, SilKit::Services::Logging::ILogger& logger)
    : _socketOptions{socketOptions}
    , _asioIoContext{std::move(asioIoContext)}
    , _acceptor{std::move(acceptor)}
    , _timeoutTimer{_acceptor.get_executor()}
    , _localEndpoint{_acceptor.local_endpoint()}
    , _logger{&logger}
{
    SILKIT_TRACE_METHOD_(_logger, "(...)");
}


template <typename T>
AsioAcceptor<T>::~AsioAcceptor()
{
    SILKIT_TRACE_METHOD_(_logger, "()");
    CleanupEndpoint(_localEndpoint);
}


template <typename T>
void AsioAcceptor<T>::SetListener(IAcceptorListener& listener)
{
    SILKIT_TRACE_METHOD_(_logger, "({})", static_cast<const void*>(&listener));

    _listener = &listener;
}


template <typename T>
auto AsioAcceptor<T>::GetLocalEndpoint() const -> std::string
{
    return FormatEndpoint(_acceptor.local_endpoint());
}


template <typename T>
void AsioAcceptor<T>::AsyncAccept(std::chrono::milliseconds timeout)
{
    SILKIT_TRACE_METHOD_(_logger, "({})", timeout.count());

    if (!_state.ExchangeIfExpected(IDLE, PENDING))
    {
        throw InvalidStateError{};
    }

    auto acceptCompletionHandler =
        asio::bind_cancellation_slot(_acceptCancelSignal.slot(), [this](const auto& e, auto s) {
            OnAsioAsyncAcceptComplete(e, std::move(s));
        });

    if (timeout.count() > 0)
    {
        auto timeoutCompletionHandler =
            asio::bind_cancellation_slot(_timeoutCancelSignal.slot(), [this](const auto& e) {
                OnAsioAsyncWaitComplete(e);
            });

        _timeoutTimer.expires_after(timeout);
        _timeoutTimer.async_wait(timeoutCompletionHandler);
    }

    _acceptor.async_accept(acceptCompletionHandler);
}


template <typename T>
void AsioAcceptor<T>::Shutdown()
{
    SILKIT_TRACE_METHOD_(_logger, "()");
    _acceptor.close();
}


template <typename T>
void AsioAcceptor<T>::OnAsioAsyncAcceptComplete(const asio::error_code& asioErrorCode, AsioSocketType socket)
{
    SILKIT_TRACE_METHOD_(_logger, "({}, ...)", asioErrorCode.message());

    if (!_state.ExchangeIfExpected(PENDING, IDLE))
    {
        throw InvalidStateError{};
    }

    if (asioErrorCode)
    {
        _listener->OnAsyncAcceptFailure(*this);
        return;
    }

    std::error_code errorCode;

    SetAsioSocketOptions(_logger, socket, _socketOptions, errorCode);
    if (errorCode)
    {
        SILKIT_TRACE_METHOD_(_logger, "failed to set socket options: {}", errorCode.message());
        _listener->OnAsyncAcceptFailure(*this);
        return;
    }

    const auto family{socket.local_endpoint().protocol().family()};
    const bool isTcp{family == asio::ip::tcp::v4().family() || family == asio::ip::tcp::v6().family()};

    AsioGenericRawByteStreamOptions options{};
    options.tcp.quickAck = isTcp && _socketOptions.tcp.quickAck;

    auto stream{std::make_unique<AsioGenericRawByteStream>(options, _asioIoContext, std::move(socket), *_logger)};

    _timeoutCancelSignal.emit(asio::cancellation_type::total);
    _listener->OnAsyncAcceptSuccess(*this, std::move(stream));
}


template <typename T>
void AsioAcceptor<T>::OnAsioAsyncWaitComplete(const asio::error_code& errorCode)
{
    SILKIT_TRACE_METHOD_(_logger, "({})", errorCode.message());

    if (errorCode)
    {
        return;
    }

    if (_state.Get() == PENDING)
    {
        _acceptCancelSignal.emit(asio::cancellation_type::total);
    }
}


} // namespace VSilKit


#undef SILKIT_TRACE_METHOD_
