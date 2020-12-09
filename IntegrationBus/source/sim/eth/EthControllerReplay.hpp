// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IReplayDataController.hpp"
#include "EthController.hpp"

namespace ib {
namespace sim {
namespace eth {

class EthControllerReplay
    : public IEthController
    , public IIbToEthController
    , public ib::mw::sync::ITimeConsumer
    , public extensions::ITraceMessageSource
    , public tracing::IReplayDataController
{
public:
    // Constructors 
    EthControllerReplay() = delete;
    EthControllerReplay(mw::IComAdapter* comAdapter, cfg::EthernetController config, mw::sync::ITimeProvider* timeProvider)
        :_controller{comAdapter, config, timeProvider}
    {

    }

public:
    // ----------------------------------------
    // Public interface methods
    //
    // IEthController
    void Activate() override
    {
        _controller.Activate();
    }
    void Deactivate() override
    {
        _controller.Deactivate();
    }

    auto SendMessage(EthMessage msg) -> EthTxId override
    {
        if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
        {
            return 0;
        }

        return _controller.SendMessage(std::move(msg));
    }

    auto SendFrame(EthFrame msg) -> EthTxId override
    {
        if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
        {
            return 0;
        }

        return _controller.SendFrame(std::move(msg));
    }
    auto SendFrame(EthFrame msg, std::chrono::nanoseconds timestamp) -> EthTxId override
    {
        if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Send))
        {
            return 0;
        }

        return _controller.SendFrame(std::move(msg), timestamp);
    }

    void RegisterReceiveMessageHandler(ReceiveMessageHandler handler) override
    {
        _controller.RegisterReceiveMessageHandler(handler);
    }

    void RegisterMessageAckHandler(MessageAckHandler handler) override
    {
        _controller.RegisterMessageAckHandler(handler);
    }
    void RegisterStateChangedHandler(StateChangedHandler handler) override
    {
        _controller.RegisterStateChangedHandler(handler);
    }

    void RegisterBitRateChangedHandler(BitRateChangedHandler handler) override
    {
        _controller.RegisterBitRateChangedHandler(handler);
    }

    // IIbToEthController
    void ReceiveIbMessage(ib::mw::EndpointAddress from, const EthMessage& msg) override
    {
        if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Receive))
        {
            return;
        }

        _controller.ReceiveIbMessage(from, msg);
    }

    void ReceiveIbMessage(ib::mw::EndpointAddress from, const EthTransmitAcknowledge& msg) override
    {
        if (tracing::IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Receive))
        {
            return;
        }

        _controller.ReceiveIbMessage(from, msg);
    }

    void SetEndpointAddress(const ib::mw::EndpointAddress& endpointAddress) override
    {
        _controller.SetEndpointAddress(endpointAddress);
    }

    auto EndpointAddress() const -> const ib::mw::EndpointAddress & override
    {
        return _controller.EndpointAddress();
    }

    // ib::mw::sync::ITimeConsumer
    void SetTimeProvider(ib::mw::sync::ITimeProvider* timeProvider) override
    {
        _controller.SetTimeProvider(timeProvider);
    }

    // ITraceMessageSource
    inline void AddSink(extensions::ITraceMessageSink* sink) override
    {
        _controller.AddSink(sink);
    }

    // IReplayDataProvider

    void ConfigureReplay(const cfg::Replay& replayConfig) override
    {
        _replayConfig = replayConfig;
    }

    void ReplayMessage(const extensions::IReplayMessage* replayMessage) override
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
            if (IsReplayEnabledFor(_replayConfig, cfg::Replay::Direction::Receive))
            {
                ReplayReceive(replayMessage);
            }
            break;
        default:
            throw std::runtime_error("ReplayController: replay message has undefined Direction");
            break;
        }

    }

private:
    //methods

    void ReplaySend(const extensions::IReplayMessage* replayMessage)
    {
        // need to copy the message here.
        // will throw if invalid message type.
        sim::eth::EthFrame msg = dynamic_cast<const sim::eth::EthFrame&>(*replayMessage);
        _controller.SendFrame(std::move(msg));
        // TODO what about acknowledges?
    }

    void ReplayReceive(const extensions::IReplayMessage* replayMessage)
    {
        sim::eth::EthFrame frame = dynamic_cast<const sim::eth::EthFrame&>(*replayMessage);
        sim::eth::EthMessage msg{};
        msg.ethFrame = std::move(frame);
        msg.timestamp = replayMessage->Timestamp();
        // XXX there is no guarantee that the EndpointAddress from the trace is valid
        // it might even be the same as ours --> will be dropped.
        _controller.ReceiveIbMessage(replayMessage->EndpointAddress(), msg); //XXX will trigger an ACK here
    }
private:
    cfg::Replay _replayConfig;
    EthController _controller;
};

} // namespace eth
} // namespace sim
} // namespace ib
