// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <tuple>
#include <vector>
#include <map>
#include <mutex>

#include "ib/sim/can/ICanController.hpp"
#include "ib/mw/fwd_decl.hpp"
#include "ib/mw/sync/ITimeConsumer.hpp"

#include "IIbToCanController.hpp"
#include "IParticipantInternal.hpp"
#include "ITraceMessageSource.hpp"
#include "ParticipantConfiguration.hpp"

#include "SimBehavior.hpp"

namespace ib {
namespace sim {
namespace can {

class CanController
    : public ICanController
    , public IIbToCanController
    , public extensions::ITraceMessageSource
    , public mw::IIbServiceEndpoint
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    // Deletions because of mutex that cannot be copied
    CanController() = delete;
    CanController(const CanController&) = delete;
    CanController(CanController&&) = delete;
    CanController(mw::IParticipantInternal* participant, ib::cfg::CanController config,
                   mw::sync::ITimeProvider* timeProvider);

public:
    // ----------------------------------------
    // Operator Implementations
    CanController& operator=(CanController& other) = delete;
    CanController& operator=(CanController&& other) = delete;

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

    HandlerId AddFrameHandler(FrameHandler handler,
                              DirectionMask directionMask = (DirectionMask)TransmitDirection::RX
                                                            | (DirectionMask)TransmitDirection::TX) override;
    void RemoveFrameHandler(HandlerId handlerId) override;

    HandlerId AddStateChangeHandler(StateChangeHandler handler) override;
    void RemoveStateChangeHandler(HandlerId handlerId) override;

    HandlerId AddErrorStateChangeHandler(ErrorStateChangeHandler handler) override;
    void RemoveErrorStateChangeHandler(HandlerId handlerId) override;

    HandlerId AddFrameTransmitHandler(
        FrameTransmitHandler handler,
        CanTransmitStatusMask statusMask = (CanTransmitStatusMask)CanTransmitStatus::Transmitted
                                           | (CanTransmitStatusMask)CanTransmitStatus::Canceled
                                           | (CanTransmitStatusMask)CanTransmitStatus::DuplicatedTransmitId
                                           | (CanTransmitStatusMask)CanTransmitStatus::TransmitQueueFull) override;
    void RemoveFrameTransmitHandler(HandlerId handlerId) override;

    // IIbToCanController
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const sim::can::CanFrameEvent& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const sim::can::CanControllerStatus& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const sim::can::CanFrameTransmitEvent& msg) override;

    //ITraceMessageSource
    inline void AddSink(extensions::ITraceMessageSink* sink) override;

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;

public:
    // ----------------------------------------
    // Public methods

    void RegisterServiceDiscovery();

    // Expose the simulated/trivial mode for unit tests
    void SetDetailedBehavior(const mw::ServiceDescriptor& remoteServiceDescriptor);
    void SetTrivialBehavior();

private:
    // ----------------------------------------
    // private data types

    template <typename MsgT>
    using FilterT = std::function<bool(const MsgT& msg)>;

    template <typename MsgT>
    struct FilteredCallback
    {
        CallbackT<MsgT> callback;
        FilterT<MsgT> filter;
    };

    template<typename MsgT>
    using CallbackMap = std::map<HandlerId, FilteredCallback<MsgT>>;

private:
    // ----------------------------------------
    // private methods
    void ChangeControllerMode(CanControllerState state);

    template<typename MsgT>
    HandlerId AddHandler(CallbackT<MsgT> handler, std::function<bool(const MsgT& msg)> filter = nullptr);

    template <typename MsgT>
    void RemoveHandler(HandlerId handlerId);

    template <typename MsgT>
    void CallHandlers(const MsgT& msg);

    auto IsRelevantNetwork(const mw::ServiceDescriptor& remoteServiceDescriptor) const -> bool;
    auto AllowReception(const IIbServiceEndpoint* from) const -> bool;

    inline auto MakeTxId() -> CanTxId;

    template <typename MsgT>
    inline void SendIbMessage(MsgT&& msg);

private:
    // ----------------------------------------
    // private members
    mw::IParticipantInternal* _participant = nullptr;
    cfg::CanController _config;
    SimBehavior _simulationBehavior;
    mw::ServiceDescriptor _serviceDescriptor;
    extensions::Tracer _tracer;

    CanTxId _canTxId = 0;
    CanControllerState _controllerState = CanControllerState::Uninit;
    CanErrorState _errorState = CanErrorState::NotAvailable;
    CanConfigureBaudrate _baudRate = { 0, 0 };

    std::tuple<
        CallbackMap<CanFrameEvent>,
        CallbackMap<CanStateChangeEvent>,
        CallbackMap<CanErrorStateChangeEvent>,
        CallbackMap<CanFrameTransmitEvent>
    > _callbacks;

    mutable std::recursive_mutex _callbacksMx;
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
