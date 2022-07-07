// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <map>

#include "silkit/services/ethernet/IEthernetController.hpp"
#include "ITimeConsumer.hpp"

#include "IParticipantInternal.hpp"
#include "ITraceMessageSource.hpp"
#include "ParticipantConfiguration.hpp"
#include "IMsgForEthController.hpp"
#include "SimBehavior.hpp"

#include "SynchronizedHandlers.hpp"

namespace SilKit {
namespace Services {
namespace Ethernet {


class EthController
    : public IEthernetController
    , public IMsgForEthController
    , public ITraceMessageSource
    , public Core::IServiceEndpoint
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

    auto SendFrame(EthernetFrame frame) -> EthernetTxId override;

    HandlerId AddFrameHandler(FrameHandler handler) override;
    HandlerId AddFrameTransmitHandler(FrameTransmitHandler handler) override;
    HandlerId AddStateChangeHandler(StateChangeHandler handler) override;
    HandlerId AddBitrateChangeHandler(BitrateChangeHandler handler) override;

    void RemoveFrameHandler(HandlerId handlerId) override;
    void RemoveFrameTransmitHandler(HandlerId handlerId) override;
    void RemoveStateChangeHandler(HandlerId handlerId) override;
    void RemoveBitrateChangeHandler(HandlerId handlerId) override;

    // IMsgForEthController
    void ReceiveSilKitMessage(const IServiceEndpoint* from, const EthernetFrameEvent& msg) override;
    void ReceiveSilKitMessage(const IServiceEndpoint* from, const EthernetFrameTransmitEvent& msg) override;
    void ReceiveSilKitMessage(const IServiceEndpoint* from, const EthernetStatus& msg) override;

    // ITraceMessageSource
    inline void AddSink(ITraceMessageSink* sink) override;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor & override;

public:
    // ----------------------------------------
    // Public methods

    void RegisterServiceDiscovery();

    // Expose for unit tests
    auto SendFrameEvent(EthernetFrameEvent msg) -> EthernetTxId;
    void SetDetailedBehavior(const Core::ServiceDescriptor& remoteServiceDescriptor);
    void SetTrivialBehavior();

    EthernetState GetState();

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

    inline auto MakeTxId() -> EthernetTxId;

    template <typename MsgT>
    inline void SendMsg(MsgT&& msg);

private:
    // ----------------------------------------
    // private members
    Core::IParticipantInternal* _participant = nullptr;
    Config::EthernetController _config;
    ::SilKit::Core::ServiceDescriptor _serviceDescriptor;
    SimBehavior _simulationBehavior;

    EthernetTxId _ethernetTxId = 0;
    EthernetState _ethState = EthernetState::Inactive;
    uint32_t _ethBitRate = 0;
    Tracer _tracer;

    template <typename MsgT>
    using CallbacksT = Util::SynchronizedHandlers<CallbackT<MsgT>>;

    std::tuple<
        CallbacksT<EthernetFrameEvent>,
        CallbacksT<EthernetFrameTransmitEvent>,
        CallbacksT<EthernetStateChangeEvent>,
        CallbacksT<EthernetBitrateChangeEvent>
    > _callbacks;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
auto EthController::MakeTxId() -> EthernetTxId
{
    return ++_ethernetTxId;
}

void EthController::AddSink(ITraceMessageSink* sink)
{
    _tracer.AddSink(SilKit::Core::EndpointAddress{}, *sink);
}
void EthController::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}
auto EthController::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace Ethernet
} // namespace Services
} // namespace SilKit
