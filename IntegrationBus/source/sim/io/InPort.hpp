// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once


#include <tuple>
#include <vector>

#include "ib/mw/fwd_decl.hpp"

#include "ib/sim/io/IInPort.hpp"
#include "ib/cfg/Config.hpp"
#include "ib/mw/sync/ITimeConsumer.hpp"
#include "ib/extensions/ITraceMessageSource.hpp"

#include "IIbToInPort.hpp"
#include "IComAdapterInternal.hpp"
#include "IIbServiceEndpoint.hpp"

namespace ib {
namespace sim {
namespace io {

template<typename MsgT>
class InPort
    : public IInPort<MsgT>
    , public IIbToInPort<MsgT>
    , public ib::mw::sync::ITimeConsumer
    , public extensions::ITraceMessageSource
    , public mw::IIbServiceEndpoint
{
public:
    // ----------------------------------------
    // Public Data Types
    using typename IInPort<MsgT>::MessageType;
    using typename IInPort<MsgT>::ValueType;
    using typename IInPort<MsgT>::ConfigType;

    template<typename T>
    using CallbackT = typename IInPort<MsgT>::template CallbackT<T>;
    using typename IInPort<MsgT>::MessageHandler;
    using typename IInPort<MsgT>::ValueHandler;

public:
    // ----------------------------------------
    // Constructors and Destructor
    InPort() = delete;
    InPort(const InPort&) = default;
    InPort(InPort&&) = default;
    InPort(mw::IComAdapterInternal* comAdapter, mw::sync::ITimeProvider* timeProvider);
    InPort(mw::IComAdapterInternal* comAdapter, ConfigType config, mw::sync::ITimeProvider* timeProvider);

public:
    // ----------------------------------------
    // Operator Implementations
    InPort& operator=(InPort& other) = default;
    InPort& operator=(InPort&& other) = default;

public:
    // ----------------------------------------
    // Public interface methods
    //
    // IInPort
    auto Config() const -> const ConfigType& override;
    auto Read() const -> const ValueType& override;
    void RegisterHandler(MessageHandler handler) override;
    void RegisterHandler(ValueHandler handler) override;

    // IIbToInPort
    //! \brief Accepts messages originating from IB communications.
    void ReceiveIbMessage(const mw::IIbServiceEndpoint* from, const MessageType& msg) override;

    //! \brief Accepts any message, e.g. also from trace replays.
    void ReceiveMessage(const MessageType& msg);

    //! \brief Setter and getter for the EndpointAddress associated with this controller
    void SetEndpointAddress(const mw::EndpointAddress& endpointAddress) override;
    auto EndpointAddress() const -> const mw::EndpointAddress& override;

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;

public:
    // ----------------------------------------
    // Public interface methods
    // ITimeConsumer
    void SetTimeProvider(mw::sync::ITimeProvider* timeProvider) override;

    // ITraceMessageSource
    inline void AddSink(extensions::ITraceMessageSink* sink) override;

private:
    // ----------------------------------------
    // private data types
    template<typename T>
    using CallbackVector = std::vector<CallbackT<T>>;

private:
    // ----------------------------------------
    // private methods
    template<typename T>
    void RegisterHandlerImpl(CallbackT<T> handler);

    template<typename T>
    void CallHandlers(const T& msg);

private:
    // ----------------------------------------
    // private members
    ConfigType _config;
    mw::IComAdapterInternal* _comAdapter{nullptr};
    mw::ServiceDescriptor _serviceDescriptor;
    MessageType _lastMessage;

    std::tuple<
        CallbackVector<MessageType>,
        CallbackVector<ValueType>
    > _callbacks;

    mw::sync::ITimeProvider* _timeProvider{nullptr};
    extensions::Tracer _tracer;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
template<typename MsgT>
InPort<MsgT>::InPort(mw::IComAdapterInternal* comAdapter, mw::sync::ITimeProvider* timeProvider)
    : _comAdapter{comAdapter}
    , _timeProvider{timeProvider}
{
}

template<typename MsgT>
InPort<MsgT>::InPort(mw::IComAdapterInternal* comAdapter, ConfigType config, mw::sync::ITimeProvider* timeProvider)
    : _comAdapter{comAdapter}
    , _config{config}
    , _timeProvider{timeProvider}
{
}

template<typename MsgT>
auto InPort<MsgT>::Config() const -> const ConfigType&
{
    return _config;
}

template<typename MsgT>
auto InPort<MsgT>::Read() const -> const ValueType&
{
    return _lastMessage.value;
}

template<typename MsgT>
void InPort<MsgT>::RegisterHandler(MessageHandler handler)
{
    RegisterHandlerImpl(std::move(handler));
}

template<typename MsgT>
void InPort<MsgT>::RegisterHandler(ValueHandler handler)
{
    RegisterHandlerImpl(std::move(handler));
}

template<typename MsgT>
void InPort<MsgT>::ReceiveIbMessage(const mw::IIbServiceEndpoint* from, const MessageType& msg)
{
    if (AllowMessageProcessing(from->GetServiceDescriptor(), _serviceDescriptor))
        return;
    ReceiveMessage(msg);
}

template<typename MsgT>
void InPort<MsgT>::ReceiveMessage(const MessageType& msg)
{
    _tracer.Trace(extensions::Direction::Receive, _timeProvider->Now(), msg);

    _lastMessage = msg;
    CallHandlers(msg);
    CallHandlers(msg.value);
}

template<typename MsgT>
void InPort<MsgT>::SetTimeProvider(mw::sync::ITimeProvider* timeProvider)
{
    _timeProvider = timeProvider;
}


template<typename MsgT>
void InPort<MsgT>::SetEndpointAddress(const mw::EndpointAddress& endpointAddress)
{
    _serviceDescriptor.legacyEpa = endpointAddress;
}

template<typename MsgT>
auto InPort<MsgT>::EndpointAddress() const -> const mw::EndpointAddress&
{
    return _serviceDescriptor.legacyEpa;
}


template<typename MsgT>
template<typename T>
void InPort<MsgT>::RegisterHandlerImpl(CallbackT<T> handler)
{
    auto&& handlers = std::get<CallbackVector<T>>(_callbacks);
    handlers.emplace_back(std::move(handler));
}

template<typename MsgT>
template<typename T>
void InPort<MsgT>::CallHandlers(const T& t)
{
    auto&& handlers = std::get<CallbackVector<T>>(_callbacks);
    for (auto&& handler : handlers)
    {
        handler(this, t);
    }
}

template<typename MsgT>
void InPort<MsgT>::AddSink(extensions::ITraceMessageSink* sink)
{
    _tracer.AddSink(EndpointAddress(), *sink);
}

template<typename MsgT>
void InPort<MsgT>::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}
template<typename MsgT>
auto InPort<MsgT>::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace io
} // namespace sim
} // namespace ib
