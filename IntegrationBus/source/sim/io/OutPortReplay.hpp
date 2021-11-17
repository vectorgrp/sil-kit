// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IReplayDataController.hpp"
#include "OutPort.hpp"


namespace ib {
namespace sim {
namespace io {

template<typename MsgT>
class OutPortReplay
    : public IOutPort<MsgT>
    , public IIbToOutPort<MsgT>
    , public ib::mw::sync::ITimeConsumer
    , public extensions::ITraceMessageSource
    , public tracing::IReplayDataController
{
public:
    // ----------------------------------------
    // Public Data Types
    using typename IOutPort<MsgT>::MessageType;
    using typename IOutPort<MsgT>::ValueType;
    using typename IOutPort<MsgT>::ConfigType;

public:
    // ----------------------------------------
    OutPortReplay(mw::IComAdapterInternal* comAdapter, ConfigType config, mw::sync::ITimeProvider* timeProvider);

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


    // ITimeConsumer
    void SetTimeProvider(mw::sync::ITimeProvider* timeProvider) override;

    // ITraceMessageSource
    inline void AddSink(extensions::ITraceMessageSink* sink) override;

    // IReplayDataProvider

    void ReplayMessage(const extensions::IReplayMessage* replayMessage) override;

private:
    //Private methods
    void ReplaySend(const extensions::IReplayMessage* replayMessage);

private:
    // ----------------------------------------
    // private members
    cfg::Replay _replayConfig;
    OutPort<MsgT> _outPort;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

template<typename MsgT>
OutPortReplay<MsgT>::OutPortReplay(mw::IComAdapterInternal* comAdapter, ConfigType config, mw::sync::ITimeProvider* timeProvider)
    : _outPort{comAdapter,config, timeProvider}
    , _replayConfig{config.replay}
{
}

template<typename MsgT>
auto OutPortReplay<MsgT>::Config() const -> const ConfigType&
{
    return _outPort.Config();
}

template<typename MsgT>
void OutPortReplay<MsgT>::Write(ValueType value, std::chrono::nanoseconds timestamp)
{
    if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
    {
        return;
    }
    _outPort.Write(std::move(value), timestamp);
}

template<typename MsgT>
auto OutPortReplay<MsgT>::Read() const -> const ValueType&
{
    return _outPort.Read();
}

template<typename MsgT>
void OutPortReplay<MsgT>::SetEndpointAddress(const mw::EndpointAddress& endpointAddress)
{
    _outPort.SetEndpointAddress(endpointAddress);
}

template<typename MsgT>
auto OutPortReplay<MsgT>::EndpointAddress() const -> const mw::EndpointAddress&
{
    return _outPort.EndpointAddress();
}

template<typename MsgT>
void OutPortReplay<MsgT>::SetTimeProvider(mw::sync::ITimeProvider* timeProvider)
{
    _outPort.SetTimeProvider(timeProvider);
}


template<typename MsgT>
void OutPortReplay<MsgT>::AddSink(extensions::ITraceMessageSink* sink)
{
    _outPort.AddSink(sink);
}

template<typename MsgT>
void OutPortReplay<MsgT>::ReplayMessage(const extensions::IReplayMessage* replayMessage)
{
    using namespace ib::tracing;
    switch (replayMessage->GetDirection())
    {
    case extensions::Direction::Send:
        if (IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
        {
            ReplaySend(replayMessage);
        }
        break;
    case extensions::Direction::Receive:
        break;
    default:
        throw std::runtime_error("OutPortReplay: replay message has undefined Direction");
        break;
    }
}

template<typename MsgT>
void OutPortReplay<MsgT>::ReplaySend(const extensions::IReplayMessage* replayMessage)
{
    MsgT msg = dynamic_cast<const MsgT&>(*replayMessage);
    _outPort.Write(std::move(msg.value), replayMessage->Timestamp());
}

} // namespace io
} // namespace sim
} // namespace ib
