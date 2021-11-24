// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once


#include "IReplayDataController.hpp"
#include "InPort.hpp"


namespace ib {
namespace sim {
namespace io {

template<typename MsgT>
class InPortReplay
    : public IInPort<MsgT>
    , public IIbToInPort<MsgT>
    , public ib::mw::sync::ITimeConsumer
    , public extensions::ITraceMessageSource
    , public tracing::IReplayDataController
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
    InPortReplay(mw::IComAdapterInternal* comAdapter, ConfigType config, mw::sync::ITimeProvider* timeProvider);

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
    void ReceiveIbMessage(const mw::IIbServiceEndpoint* from, const MessageType& msg) override;

    //! \brief Setter and getter for the EndpointAddress associated with this controller
    void SetEndpointAddress(const mw::EndpointAddress& endpointAddress) override;
    auto EndpointAddress() const -> const mw::EndpointAddress& override;

    // IIbServiceEndpoint
    inline void SetServiceId(const mw::ServiceId& serviceId) override;
    inline auto GetServiceId() const -> const mw::ServiceId & override;

public:
    // ----------------------------------------
    // Public interface methods
    // ITimeConsumer
    void SetTimeProvider(mw::sync::ITimeProvider* timeProvider) override;

    // ITraceMessageSource
    inline void AddSink(extensions::ITraceMessageSink* sink) override;

    // IReplayDataProvider

    void ReplayMessage(const extensions::IReplayMessage* replayMessage) override;

private:
    // ----------------------------------------
    //Private methods
private:
    // ----------------------------------------
    // private members
    cfg::Replay _replayConfig;
    InPort<MsgT> _inPort;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
template<typename MsgT>
InPortReplay<MsgT>::InPortReplay(mw::IComAdapterInternal* comAdapter, ConfigType config, mw::sync::ITimeProvider* timeProvider)
    : _inPort{comAdapter, config, timeProvider}
    , _replayConfig{config.replay}
{
}

template<typename MsgT>
auto InPortReplay<MsgT>::Config() const -> const ConfigType&
{
    return _inPort.Config();
}

template<typename MsgT>
auto InPortReplay<MsgT>::Read() const -> const ValueType&
{
    return _inPort.Read();
}

template<typename MsgT>
void InPortReplay<MsgT>::RegisterHandler(MessageHandler handler)
{
    _inPort.RegisterHandler(std::move(handler));
}

template<typename MsgT>
void InPortReplay<MsgT>::RegisterHandler(ValueHandler handler)
{
    _inPort.RegisterHandler(std::move(handler));
}

template<typename MsgT>
void InPortReplay<MsgT>::ReceiveIbMessage(const mw::IIbServiceEndpoint* from, const MessageType& msg)
{
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Receive))
    {
        return;
    }
    return _inPort.ReceiveIbMessage(from, msg);
}

template<typename MsgT>
void InPortReplay<MsgT>::SetTimeProvider(mw::sync::ITimeProvider* timeProvider)
{
    _inPort.SetTimeProvider(timeProvider);
}


template<typename MsgT>
void InPortReplay<MsgT>::SetEndpointAddress(const mw::EndpointAddress& endpointAddress)
{
    _inPort.SetEndpointAddress(endpointAddress);
}

template<typename MsgT>
auto InPortReplay<MsgT>::EndpointAddress() const -> const mw::EndpointAddress&
{
    return _inPort.EndpointAddress();
}

template<typename MsgT>
void InPortReplay<MsgT>::AddSink(extensions::ITraceMessageSink* sink)
{
    _inPort.AddSink(sink);
}

template<typename MsgT>
void InPortReplay<MsgT>::ReplayMessage(const extensions::IReplayMessage* replayMessage)
{
    using namespace ib::tracing;
    switch (replayMessage->GetDirection())
    {
    case extensions::Direction::Receive:
        if (IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Receive))
        {
            auto msg = dynamic_cast<const MsgT&>(*replayMessage);
            _inPort.ReceiveMessage(msg);
        }
        break;
    case extensions::Direction::Send:
        break;
    default:
        throw std::runtime_error("InPortReplay: replay message has undefined Direction");
        break;
    }
}

template<typename MsgT>
void InPortReplay<MsgT>::SetServiceId(const ib::mw::ServiceId& serviceId)
{
    _inPort.SetServiceId(serviceId);
}
template<typename MsgT>
auto InPortReplay<MsgT>::GetServiceId() const -> const mw::ServiceId&
{
    return _inPort.GetServiceId();
}

} // namespace io
} // namespace sim
} // namespace ib
