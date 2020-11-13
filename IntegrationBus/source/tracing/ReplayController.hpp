// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/extensions/IReplay.hpp"

#include "EthController.hpp"
#include "CanController.hpp"
#include "FrController.hpp"
#include "LinController.hpp"

namespace ib {
namespace tracing {
class ReplayController
{
public:
    ~ReplayController() = default;
    ReplayController(const cfg::Replay& replayConfig)
        : _replayConfig{replayConfig}
    {

    }
    void ReplayMessage(extensions::IReplayMessage* replayMessage)
    {
        switch (replayMessage->GetDirection())
        {
        case extensions::Direction::Send:
            if (IsEnabledFor(cfg::Replay::Direction::Send))
            {
                ReplaySend(replayMessage);
            }
            break;
        case extensions::Direction::Receive:
            if (IsEnabledFor(cfg::Replay::Direction::Receive))
            {
                ReplayReceive(replayMessage);
            }
            break;
        default:
            throw std::runtime_error("ReplayController: replay message has undefined Direction");
            break;
        }
    }

    bool IsEnabledFor(cfg::Replay::Direction dir) const
    {
        return _replayConfig.direction == dir
            || _replayConfig.direction == cfg::Replay::Direction::Both;
    }
protected:
    //methods
    virtual void ReplaySend(extensions::IReplayMessage* )
    {
        throw std::runtime_error("ReplayController::ReplaySend not implemented");
    }
    virtual void ReplayReceive(extensions::IReplayMessage* )
    {
        throw std::runtime_error("ReplayController::ReplayReceive not implemented");
    }
private:
    // Members
    cfg::Replay _replayConfig;
};

//////////////////////////////////////////////////////////////////////
// Actual Replay Simulation Controllers
//////////////////////////////////////////////////////////////////////

class EthControllerReplay
    : public ReplayController
    , public sim::eth::EthController
{
public:
    EthControllerReplay(mw::IComAdapter* comAdapter, cfg::EthernetController cfg, mw::sync::ITimeProvider* timeProvider)
        : ReplayController{cfg.replay}
        , EthController{comAdapter, std::move(cfg), timeProvider}
    {
    }

    // Make sure the participant only sees traffic that is replayed through our
    // Replay{Send,Receive} methods.
    void ReceiveIbMessage(mw::EndpointAddress from, const sim::eth::EthMessage& msg) override
    {
        // cut of the participants ReceiveIbMessage only if we are not replaying on Receive direction
        // TODO should this be the default?
        if (!IsEnabledFor(cfg::Replay::Direction::Receive))
        {
            EthController::ReceiveIbMessage(from, msg);
        }
    }
    auto SendFrame(sim::eth::EthFrame msg, std::chrono::nanoseconds timestamp) -> sim::eth::EthTxId override
    {
        if (!IsEnabledFor(cfg::Replay::Direction::Send))
        {
            return EthController::SendFrame(std::move(msg), timestamp);
        }
        return 0; //XXX the user has a way to register a ack message handler and check this, what to do?
    }

private:
    void ReplaySend(extensions::IReplayMessage* replayMessage) override
    {
        // need to copy the message here.
        // will throw if invalid message type.
        sim::eth::EthFrame msg = dynamic_cast<sim::eth::EthFrame&>(*replayMessage);
        EthController::SendFrame(std::move(msg));
        // TODO what about acknowledges?
    }
    void ReplayReceive(extensions::IReplayMessage* replayMessage) override
    {
        sim::eth::EthFrame frame = dynamic_cast<sim::eth::EthFrame&>(*replayMessage);
        sim::eth::EthMessage msg{};
        msg.ethFrame = std::move(frame);
        msg.timestamp = replayMessage->Timestamp();
        // XXX there is no guarantee that the EndpointAddress from the trace is valid
        // it might even be the same as ours --> will be dropped.
        EthController::ReceiveIbMessage(replayMessage->EndpointAddress(), msg); //XXX will trigger an ACK here
    }
};


} //end namespace tracing
} //end namespace ib
