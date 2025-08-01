// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <map>

#include "silkit/services/ethernet/IEthernetController.hpp"

#include "ITimeConsumer.hpp"
#include "IParticipantInternal.hpp"
#include "ITraceMessageSource.hpp"
#include "IReplayDataController.hpp"
#include "ParticipantConfiguration.hpp"
#include "IMsgForEthController.hpp"
#include "SimBehavior.hpp"

#include "SynchronizedHandlers.hpp"
#include "LoggerMessage.hpp"

namespace SilKit {
namespace Services {
namespace Ethernet {


class EthController
    : public IEthernetController
    , public IMsgForEthController
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
    EthController() = delete;
    EthController(const EthController&) = delete;
    EthController(EthController&&) = delete;
    EthController(Core::IParticipantInternal* participant, Config::EthernetController config,
                  Services::Orchestration::ITimeProvider* timeProvider);

public:
    // ----------------------------------------
    // Operator Implementations
    EthController& operator=(EthController& other) = delete;
    EthController& operator=(EthController&& other) = delete;

public:
    // ----------------------------------------
    // Public interface methods
    //
    // IEthernetController
    void Activate() override;
    void Deactivate() override;

    void SendFrame(EthernetFrame frame, void* userContext = nullptr) override;

    HandlerId AddFrameHandler(FrameHandler handler, DirectionMask directionMask = 0xFF) override;
    HandlerId AddFrameTransmitHandler(FrameTransmitHandler handler,
                                      EthernetTransmitStatusMask transmitStatusMask = 0xFFFF'FFFF) override;
    HandlerId AddStateChangeHandler(StateChangeHandler handler) override;
    HandlerId AddBitrateChangeHandler(BitrateChangeHandler handler) override;

    void RemoveFrameHandler(HandlerId handlerId) override;
    void RemoveFrameTransmitHandler(HandlerId handlerId) override;
    void RemoveStateChangeHandler(HandlerId handlerId) override;
    void RemoveBitrateChangeHandler(HandlerId handlerId) override;

    // IMsgForEthController
    void ReceiveMsg(const IServiceEndpoint* from, const WireEthernetFrameEvent& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from, const EthernetFrameTransmitEvent& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from, const EthernetStatus& msg) override;

    // ITraceMessageSource
    inline void AddSink(ITraceMessageSink* sink, SilKit::Config::NetworkType networkType) override;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor& override;

    // IReplayDataProvider
    void ReplayMessage(const IReplayMessage* message) override;

public:
    // ----------------------------------------
    // Public methods

    void RegisterServiceDiscovery();

    // Expose for unit tests
    void SetDetailedBehavior(const Core::ServiceDescriptor& remoteServiceDescriptor);
    void SetTrivialBehavior();

    EthernetState GetState();

    inline auto GetTracer() -> Tracer*;

private:
    // ----------------------------------------
    // private methods

    template <typename MsgT>
    HandlerId AddHandler(CallbackT<MsgT> handler);

    template <typename MsgT>
    auto RemoveHandler(HandlerId handlerId) -> bool;

    template <typename MsgT>
    void CallHandlers(const MsgT& msg);

    auto IsRelevantNetwork(const Core::ServiceDescriptor& remoteServiceDescriptor) const -> bool;
    auto AllowReception(const IServiceEndpoint* from) const -> bool;

    template <typename MsgT>
    inline void SendMsg(MsgT&& msg);

    // IReplayDataProvider Implementation
    void ReplaySend(const IReplayMessage* replayMessage);
    void ReplayReceive(const IReplayMessage* replayMessage);
    void SendFrameInternal(EthernetFrame frame, void* userContext);
    void ReceiveMsgInternal(const IServiceEndpoint* from, const WireEthernetFrameEvent& msg);

private:
    // ----------------------------------------
    // private members
    Core::IParticipantInternal* _participant{nullptr};
    Config::EthernetController _config;
    ::SilKit::Core::ServiceDescriptor _serviceDescriptor;
    SimBehavior _simulationBehavior;

    EthernetState _ethState{EthernetState::Inactive};
    uint32_t _ethBitRate{0};
    Orchestration::ITimeProvider* _timeProvider{nullptr};
    Tracer _tracer;
    bool _replayActive{false};
    Services::Logging::ILogger* _logger;
    Services::Logging::LogOnceFlag _logOnce;

    template <typename MsgT>
    using CallbacksT = Util::SynchronizedHandlers<CallbackT<MsgT>>;

    std::tuple<CallbacksT<EthernetFrameEvent>, CallbacksT<EthernetFrameTransmitEvent>,
               CallbacksT<EthernetStateChangeEvent>, CallbacksT<EthernetBitrateChangeEvent>>
        _callbacks;
};

// ================================================================================
//  Inline Implementations
// ================================================================================

void EthController::AddSink(ITraceMessageSink* sink, SilKit::Config::NetworkType /*networkType*/)
{
    _tracer.AddSink(GetServiceDescriptor(), *sink);
}

void EthController::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto EthController::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}

auto EthController::GetTracer() -> Tracer*
{
    return &_tracer;
}

} // namespace Ethernet
} // namespace Services
} // namespace SilKit
