/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

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
#include "IReplayDataController.hpp"
#include "ParticipantConfiguration.hpp"

#include "SimBehavior.hpp"

#include "SynchronizedHandlers.hpp"
#include "ILogger.hpp"

namespace SilKit {
namespace Services {
namespace Can {

class CanController
    : public ICanController
    , public IMsgForCanController
    , public ITraceMessageSource
    , public Core::IServiceEndpoint
    , public Tracing::IReplayDataController
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
    void SetBaudRate(uint32_t rate, uint32_t fdRate, uint32_t xlRate) override;

    void Reset() override;
    void Start() override;
    void Stop() override;
    void Sleep() override;

    void SendFrame(const CanFrame& msg, void* userContext = nullptr) override;

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
        CanTransmitStatusMask statusMask = SilKit_CanTransmitStatus_DefaultMask) override;
    void RemoveFrameTransmitHandler(HandlerId handlerId) override;

    // IMsgForCanController
    void ReceiveMsg(const IServiceEndpoint* from, const Services::Can::WireCanFrameEvent& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from, const Services::Can::CanControllerStatus& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from, const Services::Can::CanFrameTransmitEvent& msg) override;

    //ITraceMessageSource
    inline void AddSink(ITraceMessageSink* sink, SilKit::Config::NetworkType networkType) override;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor & override;

    // IReplayDataController
    void ReplayMessage(const SilKit::IReplayMessage *message) override;

public:
    // ----------------------------------------
    // Public methods

    void RegisterServiceDiscovery();

    // Expose the simulated/trivial mode for unit tests
    void SetDetailedBehavior(const Core::ServiceDescriptor& remoteServiceDescriptor);
    void SetTrivialBehavior();

    auto GetState() -> CanControllerState;

    auto GetTracer() -> Tracer *;

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

    template <typename MsgT>
    inline void SendMsg(MsgT&& msg);

    // IReplayDataProvider Implementation
    void ReplaySend(const IReplayMessage * replayMessage);
    void ReplayReceive(const IReplayMessage * replayMessage);

private:
    // ----------------------------------------
    // private members
    Core::IParticipantInternal* _participant = nullptr;
    Config::CanController _config;
    SimBehavior _simulationBehavior;
    Core::ServiceDescriptor _serviceDescriptor;
    Tracer _tracer;
    bool _replayActive{false};
    Services::Logging::ILogger* _logger;
    Services::Logging::LogOnceFlag _logOnce;

    CanControllerState _controllerState = CanControllerState::Uninit;
    CanErrorState _errorState = CanErrorState::NotAvailable;
    CanConfigureBaudrate _baudRate = { 0, 0, 0 };

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

void CanController::AddSink(ITraceMessageSink* sink, SilKit::Config::NetworkType /*networkType*/)
{
    _tracer.AddSink(GetServiceDescriptor(), *sink);
}

void CanController::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto CanController::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}

inline auto CanController::GetTracer() -> Tracer *
{
    return &_tracer;
}

} // namespace Can
} // namespace Services
} // namespace SilKit
