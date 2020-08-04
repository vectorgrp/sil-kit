// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <tuple>
#include <vector>

#include "ib/mw/fwd_decl.hpp"

#include "ib/sim/io/IOutPort.hpp"
#include "ib/sim/io/IIbToOutPort.hpp"
#include "ib/mw/IComAdapter.hpp"
#include "ib/mw/sync/ITimeConsumer.hpp"
#include "ib/extensions/ITraceMessageSource.hpp"
#include "ib/cfg/Config.hpp"


namespace ib {
namespace sim {
namespace io {

template<typename MsgT>
class OutPort
    : public IOutPort<MsgT>
    , public IIbToOutPort<MsgT>
    , public ib::mw::sync::ITimeConsumer
    , public extensions::ITraceMessageSource
{
public:
    // ----------------------------------------
    // Public Data Types
    using typename IOutPort<MsgT>::MessageType;
    using typename IOutPort<MsgT>::ValueType;
    using typename IOutPort<MsgT>::ConfigType;

public:
    // ----------------------------------------
    // Constructors and Destructor
    OutPort() = delete;
    OutPort(const OutPort&) = default;
    OutPort(OutPort&&) = default;
    OutPort(mw::IComAdapter* comAdapter, mw::sync::ITimeProvider* timeProvider);
    OutPort(mw::IComAdapter* comAdapter, ConfigType config, mw::sync::ITimeProvider* timeProvider);

public:
    // ----------------------------------------
    // Operator Implementations
    OutPort& operator=(OutPort& other) = default;
    OutPort& operator=(OutPort&& other) = default;

public:
    // ----------------------------------------
    // Public interface methods
    //
    // IOutPort
    auto Config() const -> const ConfigType& override;
    void Write(ValueType value, std::chrono::nanoseconds timestamp) override;
    auto Read() const -> const ValueType& override;

     // IIbToOutPort
    void SetEndpointAddress(const mw::EndpointAddress& endpointAddress) override;
    auto EndpointAddress() const -> const mw::EndpointAddress& override;

public:
    // ----------------------------------------
    // Public interface methods

    // ITimeConsumer
    void SetTimeProvider(mw::sync::ITimeProvider* timeProvider) override;

    // ITraceMessageSource
    inline void AddSink(extensions::ITraceMessageSink* sink) override;

private:
    // ----------------------------------------
    // private methods
    template<typename T>
    inline void SendIbMessage(T&& msg);

private:
    // ----------------------------------------
    // private members
    ConfigType _config{};
    mw::IComAdapter* _comAdapter{nullptr};
    mw::EndpointAddress _endpointAddr;
    mw::sync::ITimeProvider* _timeProvider{nullptr};

    ValueType _lastValue;

    extensions::Tracer _tracer;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
template<typename MsgT>
OutPort<MsgT>::OutPort(mw::IComAdapter* comAdapter,  mw::sync::ITimeProvider* timeProvider)
    : _comAdapter{comAdapter}
    , _timeProvider{timeProvider}
{
}

template<typename MsgT>
OutPort<MsgT>::OutPort(mw::IComAdapter* comAdapter, ConfigType config, mw::sync::ITimeProvider* timeProvider)
    : _comAdapter{comAdapter}
    , _config{std::move(config)}
    , _timeProvider{timeProvider}
{
}

template<typename MsgT>
auto OutPort<MsgT>::Config() const -> const ConfigType&
{
    return _config;
}

template<typename MsgT>
void OutPort<MsgT>::Write(ValueType value, std::chrono::nanoseconds timestamp)
{
    _lastValue = value;
    MessageType msg;
    msg.timestamp = timestamp;
    msg.value = value;

    SendIbMessage(std::move(msg));
}

template<typename MsgT>
auto OutPort<MsgT>::Read() const -> const ValueType&
{
    return _lastValue;
}

template<typename MsgT>
void OutPort<MsgT>::SetEndpointAddress(const mw::EndpointAddress& endpointAddress)
{
    _endpointAddr = endpointAddress;
}

template<typename MsgT>
auto OutPort<MsgT>::EndpointAddress() const -> const mw::EndpointAddress&
{
    return _endpointAddr;
}

template<typename MsgT>
void OutPort<MsgT>::SetTimeProvider(mw::sync::ITimeProvider* timeProvider)
{
    _timeProvider = timeProvider;
}

template<typename MsgT>
template<typename T>
void OutPort<MsgT>::SendIbMessage(T&& msg)
{
    _tracer.Trace(extensions::Direction::Send, _timeProvider->Now(), msg);

    _comAdapter->SendIbMessage(_endpointAddr, std::forward<T>(msg));
}

template<typename MsgT>
void OutPort<MsgT>::AddSink(extensions::ITraceMessageSink* sink)
{
    _tracer.AddSink(EndpointAddress(), *sink);
}

} // namespace io
} // namespace sim
} // namespace ib
