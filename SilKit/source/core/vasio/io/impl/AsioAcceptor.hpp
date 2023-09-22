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
#include "util/Ptr.hpp"
#include "util/TracingMacros.hpp"

#include "ILogger.hpp"

#include <memory>

#include "asio.hpp"


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

    IIoContext* _ioContext{nullptr};
    IAcceptorListener* _listener{nullptr};

    AtomicEnum<State> _state{IDLE};

    AsioSocketOptions _socketOptions;

    AsioAcceptorType _acceptor;
    asio::cancellation_signal _acceptCancelSignal;

    asio::steady_timer _timeoutTimer;
    asio::cancellation_signal _timeoutCancelSignal;

    AsioEndpointType _localEndpoint;

    SilKit::Services::Logging::ILogger* _logger{nullptr};

public:
    AsioAcceptor(IIoContext& ioContext, const AsioSocketOptions& socketOptions, AsioAcceptorType acceptor,
                 SilKit::Services::Logging::ILogger& logger);
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
AsioAcceptor<T>::AsioAcceptor(IIoContext& ioContext, const AsioSocketOptions& socketOptions, AsioAcceptorType acceptor,
                              SilKit::Services::Logging::ILogger& logger)
    : _ioContext{&ioContext}
    , _socketOptions{socketOptions}
    , _acceptor{std::move(acceptor)}
    , _timeoutTimer{_acceptor.get_executor()}
    , _localEndpoint{_acceptor.local_endpoint()}
    , _logger{&logger}
{
    SILKIT_TRACE_METHOD(_logger, "({}, ...)", static_cast<const void*>(&ioContext));
}


template <typename T>
AsioAcceptor<T>::~AsioAcceptor()
{
    SILKIT_TRACE_METHOD(_logger, "()");
    CleanupEndpoint(_localEndpoint);
}


template <typename T>
void AsioAcceptor<T>::SetListener(IAcceptorListener& listener)
{
    SILKIT_TRACE_METHOD(_logger, "({})", static_cast<const void*>(&listener));

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
    SILKIT_TRACE_METHOD(_logger, "({})", timeout.count());

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
    SILKIT_TRACE_METHOD(_logger, "()");
    _acceptor.close();
}


template <typename T>
void AsioAcceptor<T>::OnAsioAsyncAcceptComplete(const asio::error_code& asioErrorCode, AsioSocketType socket)
{
    SILKIT_TRACE_METHOD(_logger, "({}, ...)", asioErrorCode.message());

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
        SILKIT_TRACE_METHOD(_logger, "failed to set socket options: {}", errorCode.message());
        _listener->OnAsyncAcceptFailure(*this);
        return;
    }

    const auto family{socket.local_endpoint().protocol().family()};
    const bool isTcp{family == asio::ip::tcp::v4().family() || family == asio::ip::tcp::v6().family()};

    AsioGenericRawByteStreamOptions options{};
    options.tcp.quickAck = isTcp && _socketOptions.tcp.quickAck;

    auto stream{std::make_unique<AsioGenericRawByteStream>(*_ioContext, options, std::move(socket), *_logger)};

    _timeoutCancelSignal.emit(asio::cancellation_type::total);
    _listener->OnAsyncAcceptSuccess(*this, std::move(stream));
}


template <typename T>
void AsioAcceptor<T>::OnAsioAsyncWaitComplete(const asio::error_code& errorCode)
{
    SILKIT_TRACE_METHOD(_logger, "({})", errorCode.message());

    if (!_state.ExchangeIfExpected(PENDING, IDLE))
    {
        throw InvalidStateError{};
    }

    if (errorCode)
    {
        return;
    }

    _acceptCancelSignal.emit(asio::cancellation_type::total);
}


} // namespace VSilKit
