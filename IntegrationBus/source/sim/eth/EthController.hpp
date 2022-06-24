// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>
#include <map>

#include "ib/sim/eth/IEthernetController.hpp"
#include "ib/mw/fwd_decl.hpp"
#include "ib/mw/sync/ITimeConsumer.hpp"

#include "IParticipantInternal.hpp"
#include "ITraceMessageSource.hpp"
#include "ParticipantConfiguration.hpp"
#include "IIbToEthController.hpp"
#include "SimBehavior.hpp"

#include "SynchronizedHandlers.hpp"

namespace ib {
namespace sim {
namespace eth {


class EthController
    : public IEthernetController
    , public IIbToEthController
    , public extensions::ITraceMessageSource
    , public mw::IIbServiceEndpoint
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
    EthController(mw::IParticipantInternal* participant, cfg::EthernetController config,
                   mw::sync::ITimeProvider* timeProvider);

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

    // IIbToEthController
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const EthernetFrameEvent& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const EthernetFrameTransmitEvent& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const EthernetStatus& msg) override;

    // ITraceMessageSource
    inline void AddSink(extensions::ITraceMessageSink* sink) override;

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;

public:
    // ----------------------------------------
    // Public methods

    void RegisterServiceDiscovery();

    // Expose for unit tests
    auto SendFrameEvent(EthernetFrameEvent msg) -> EthernetTxId;
    void SetDetailedBehavior(const mw::ServiceDescriptor& remoteServiceDescriptor);
    void SetTrivialBehavior();

private:
    // ----------------------------------------
    // private methods

    template <typename MsgT>
    HandlerId AddHandler(CallbackT<MsgT> handler);

    template <typename MsgT>
    auto RemoveHandler(HandlerId handlerId) -> bool;

    template <typename MsgT>
    void CallHandlers(const MsgT& msg);

    auto IsRelevantNetwork(const mw::ServiceDescriptor& remoteServiceDescriptor) const -> bool;
    auto AllowReception(const IIbServiceEndpoint* from) const -> bool;

    inline auto MakeTxId() -> EthernetTxId;

    template <typename MsgT>
    inline void SendIbMessage(MsgT&& msg);

private:
    // ----------------------------------------
    // private members
    mw::IParticipantInternal* _participant = nullptr;
    cfg::EthernetController _config;
    ::ib::mw::ServiceDescriptor _serviceDescriptor;
    SimBehavior _simulationBehavior;

    EthernetTxId _ethernetTxId = 0;
    EthernetState _ethState = EthernetState::Inactive;
    uint32_t _ethBitRate = 0;
    extensions::Tracer _tracer;

    template <typename MsgT>
    using CallbacksT = util::SynchronizedHandlers<CallbackT<MsgT>>;

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

void EthController::AddSink(extensions::ITraceMessageSink* sink)
{
    _tracer.AddSink(ib::mw::EndpointAddress{}, *sink);
}
void EthController::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}
auto EthController::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace eth
} // namespace sim
} // namespace ib
