// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/fwd_decl.hpp"

#include "ib/sim/can/ICanController.hpp"
#include "ib/mw/sync/ITimeConsumer.hpp"

#include "IIbToCanController.hpp"
#include "IParticipantInternal.hpp"
#include "IIbServiceEndpoint.hpp"
#include "IReplayDataController.hpp"
#include "ITraceMessageSource.hpp"

#include "ParticipantConfiguration.hpp"

#include <tuple>
#include <vector>

namespace ib {
namespace sim {
namespace can {

class CanController
    : public ICanController
    , public IIbToCanController
    , public mw::sync::ITimeConsumer
    , public extensions::ITraceMessageSource
    , public tracing::IReplayDataController
    , public mw::IIbServiceEndpoint
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    CanController() = delete;
    CanController(const CanController&) = default;
    CanController(CanController&&) = default;
    CanController(mw::IParticipantInternal* participant, const ib::cfg::CanController& config,
                  mw::sync::ITimeProvider* timeProvider, ICanController* facade = nullptr);

public:
    // ----------------------------------------
    // Operator Implementations
    CanController& operator=(CanController& other) = default;
    CanController& operator=(CanController&& other) = default;

public:
    // ----------------------------------------
    // Public interface methods
    //
    // ICanController
    void SetBaudRate(uint32_t rate, uint32_t fdRate) override;

    void Reset() override;
    void Start() override;
    void Stop() override;
    void Sleep() override;

    auto SendFrame(const CanFrame& msg, void* userContext = nullptr) -> CanTxId override;

    void AddFrameHandler(FrameHandler handler, DirectionMask directionMask = (DirectionMask)TransmitDirection::RX | (DirectionMask)TransmitDirection::TX) override;

    void AddStateChangeHandler(StateChangeHandler handler) override;
    void AddErrorStateChangeHandler(ErrorStateChangeHandler handler) override;
    void AddFrameTransmitHandler(FrameTransmitHandler handler, CanTransmitStatusMask statusMask = (CanTransmitStatusMask)CanTransmitStatus::Transmitted
        | (CanTransmitStatusMask)CanTransmitStatus::Canceled
        | (CanTransmitStatusMask)CanTransmitStatus::DuplicatedTransmitId
        | (CanTransmitStatusMask)CanTransmitStatus::TransmitQueueFull) override;

    // IIbToCanController
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const sim::can::CanFrameEvent& msg) override;

    //ITimeConsumer
    void SetTimeProvider(ib::mw::sync::ITimeProvider* timeProvider) override;

    // ITraceMessageSource
    inline void AddSink(extensions::ITraceMessageSink* sink) override;

    // IReplayDataProvider
    void ReplayMessage(const extensions::IReplayMessage* replayMessage) override;

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;
public:
    // ----------------------------------------
    // Public interface methods

private:
    // ----------------------------------------
    // private data types
    //template<typename MsgT>
    //using CallbackVector = std::vector<CallbackT<MsgT>>;
    template<typename MsgT>
    using CallbackVector = std::vector<std::tuple<CallbackT<MsgT>, std::function<bool(const MsgT& msg)>>>;

private:
    // ----------------------------------------
    // private methods
    template<typename MsgT>
    void RegisterHandler(CallbackT<MsgT> handler, std::function<bool(const MsgT& msg)> filter = nullptr);

    template<typename MsgT>
    void CallHandlers(const MsgT& msg);

    inline auto MakeTxId() -> CanTxId;

    // Replay
    void ReplaySend(const extensions::IReplayMessage* replayMessage);
    void ReplayReceive(const extensions::IReplayMessage* replayMessage);

private:
    // ----------------------------------------
    // private members
    ::ib::mw::IParticipantInternal* _participant{nullptr};
    cfg::CanController _config;
    mw::ServiceDescriptor _serviceDescriptor;
    mw::sync::ITimeProvider* _timeProvider{nullptr};
    ICanController* _facade{nullptr};

    CanTxId _canTxId = 0;

    std::tuple<
        CallbackVector<CanFrameEvent>,
        CallbackVector<CanFrameTransmitEvent>
    > _callbacks;

    extensions::Tracer _tracer;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
auto CanController::MakeTxId() -> CanTxId
{
    return ++_canTxId;
}

void CanController::AddSink(extensions::ITraceMessageSink* sink)
{
    _tracer.AddSink(ib::mw::EndpointAddress{}, *sink);
}

void CanController::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}
auto CanController::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace can
} // namespace sim
} // namespace ib
