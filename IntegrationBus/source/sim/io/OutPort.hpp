// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/fwd_decl.hpp"

#include "ib/sim/io/IOutPort.hpp"
#include "ib/sim/io/IIbToOutPort.hpp"
#include "ib/mw/IComAdapter.hpp"
#include "ib/cfg/Config.hpp"

#include <tuple>
#include <vector>

namespace ib {
namespace sim {
namespace io {

template<typename MsgT>
class OutPort :
    public IOutPort<MsgT>,
    public IIbToOutPort<MsgT>
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
    OutPort(mw::IComAdapter* comAdapter);
    OutPort(mw::IComAdapter* comAdapter, ConfigType config);

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

    ValueType _lastValue;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
template<typename MsgT>
OutPort<MsgT>::OutPort(mw::IComAdapter* comAdapter)
    : _comAdapter{comAdapter}
{
}

template<typename MsgT>
OutPort<MsgT>::OutPort(mw::IComAdapter* comAdapter, ConfigType config)
    : _comAdapter{comAdapter}
    , _config{std::move(config)}
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
template<typename T>
void OutPort<MsgT>::SendIbMessage(T&& msg)
{
    _comAdapter->SendIbMessage(_endpointAddr, std::forward<T>(msg));
}


} // namespace io
} // namespace sim
} // namespace ib
