// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <tuple>
#include <vector>
#include <map>
#include <mutex>

#include "silkit/services/can/ICanController.hpp"
#include "ITimeConsumer.hpp"

#include "IMsgForCanController.hpp"
#include "IParticipantInternal.hpp"
#include "ITraceMessageSource.hpp"
#include "ParticipantConfiguration.hpp"

#include "SimBehavior.hpp"

#include "SynchronizedHandlers.hpp"

namespace SilKit {
namespace Services {
namespace Can {

class CanController
    : public ICanController
    , public IMsgForCanController
    , public ITraceMessageSource
    , public Core::IServiceEndpoint
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    CanController() = delete;
    CanController(const CanController&) = delete;
    CanController(CanController&&) = delete;
    CanController(Core::IParticipantInternal* participant, SilKit::Config::CanController config,
                   Services::Orchestration::ITimeProvider* timeProvider);

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

    // IMsgForCanController
    void ReceiveSilKitMessage(const IServiceEndpoint* from, const Services::Can::CanFrameEvent& msg) override;
    void ReceiveSilKitMessage(const IServiceEndpoint* from, const Services::Can::CanControllerStatus& msg) override;
    void ReceiveSilKitMessage(const IServiceEndpoint* from, const Services::Can::CanFrameTransmitEvent& msg) override;

    //ITraceMessageSource
    inline void AddSink(ITraceMessageSink* sink) override;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor & override;

public:
    // ----------------------------------------
    // Public methods

    void RegisterServiceDiscovery();

    // Expose the simulated/trivial mode for unit tests
    void SetDetailedBehavior(const Core::ServiceDescriptor& remoteServiceDescriptor);
    void SetTrivialBehavior();

    auto GetState() -> CanControllerState;

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

        void operator()(ICanController* controller, const MsgT& msg) const
        {
            if (!filter || filter(msg))
            {
                callback(controller, msg);
            }
        }
    };

private:
    // ----------------------------------------
    // private methods
    void ChangeControllerMode(CanControllerState state);

    template <typename MsgT>
    HandlerId AddHandler(CallbackT<MsgT> handler, FilterT<MsgT> filter = nullptr);

    template <typename MsgT>
    bool RemoveHandler(HandlerId handlerId);

    template <typename MsgT>
    void CallHandlers(const MsgT& msg);

    auto IsRelevantNetwork(const Core::ServiceDescriptor& remoteServiceDescriptor) const -> bool;
    auto AllowReception(const IServiceEndpoint* from) const -> bool;

    inline auto MakeTxId() -> CanTxId;

    template <typename MsgT>
    inline void SendMsg(MsgT&& msg);

private:
    // ----------------------------------------
    // private members
    Core::IParticipantInternal* _participant = nullptr;
    Config::CanController _config;
    SimBehavior _simulationBehavior;
    Core::ServiceDescriptor _serviceDescriptor;
    Tracer _tracer;

    CanTxId _canTxId = 0;
    CanControllerState _controllerState = CanControllerState::Uninit;
    CanErrorState _errorState = CanErrorState::NotAvailable;
    CanConfigureBaudrate _baudRate = { 0, 0 };

    template <typename MsgT>
    using FilteredCallbacks = Util::SynchronizedHandlers<FilteredCallback<MsgT>>;

    std::tuple<
        FilteredCallbacks<CanFrameEvent>,
        FilteredCallbacks<CanStateChangeEvent>,
        FilteredCallbacks<CanErrorStateChangeEvent>,
        FilteredCallbacks<CanFrameTransmitEvent>
    > _callbacks;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
auto CanController::MakeTxId() -> CanTxId
{
    return ++_canTxId;
}

void CanController::AddSink(ITraceMessageSink* sink)
{
    _tracer.AddSink(SilKit::Core::EndpointAddress{}, *sink);
}

void CanController::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}
auto CanController::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace Can
} // namespace Services
} // namespace SilKit
