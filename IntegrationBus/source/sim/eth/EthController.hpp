// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/sim/eth/IEthernetController.hpp"
#include "ib/mw/sync/ITimeConsumer.hpp"
#include "ib/mw/fwd_decl.hpp"

#include "IIbToEthController.hpp"
#include "IParticipantInternal.hpp"
#include "IIbServiceEndpoint.hpp"
#include "ITraceMessageSource.hpp"
#include "ParticipantConfiguration.hpp"

#include <memory>

namespace ib {
namespace sim {
namespace eth {

class EthController
    : public IEthernetController
    , public IIbToEthController
    , public ib::mw::sync::ITimeConsumer
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
    EthController(EthController&&) = default;
    EthController(mw::IParticipantInternal* participant, cfg::EthernetController config,
                  mw::sync::ITimeProvider* timeProvider, IEthernetController* facade = nullptr);

public:
    // ----------------------------------------
    // Operator Implementations
    EthController& operator=(EthController&& other) = default;

public:
    // ----------------------------------------
    // Public interface methods
    //
    // IEthernetController
    void Activate() override;
    void Deactivate() override;

    auto SendFrameEvent(EthernetFrameEvent msg) -> EthernetTxId;

    auto SendFrame(EthernetFrame msg) -> EthernetTxId override;

    void AddFrameHandler(FrameHandler handler) override;
    void AddFrameTransmitHandler(FrameTransmitHandler handler) override;
    void AddStateChangeHandler(StateChangeHandler handler) override;
    void AddBitrateChangeHandler(BitrateChangeHandler handler) override;

    // IIbToEthController
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const EthernetFrameEvent& msg) override;

    // ib::mw::sync::ITimeConsumer
    void SetTimeProvider(ib::mw::sync::ITimeProvider*) override;

    // ITraceMessageSource
    inline void AddSink(extensions::ITraceMessageSink* sink) override;

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;

private:
    // ----------------------------------------
    // private data types
    template<typename MsgT>
    using CallbackVector = std::vector<CallbackT<MsgT>>;

private:
    // ----------------------------------------
    // private methods
    template<typename MsgT>
    void RegisterHandler(CallbackT<MsgT> handler);

    template<typename MsgT>
    void CallHandlers(const MsgT& msg);

    inline auto MakeTxId() -> EthernetTxId;

private:
    // ----------------------------------------
    // private members
    ::ib::mw::IParticipantInternal* _participant = nullptr;
    ::ib::mw::ServiceDescriptor _serviceDescriptor;
    ::ib::mw::sync::ITimeProvider* _timeProvider{ nullptr };
    IEthernetController* _facade{nullptr};

    EthernetTxId _ethernetTxId = 0;

    std::tuple<
        CallbackVector<EthernetFrameEvent>,
        CallbackVector<EthernetFrameTransmitEvent>
    > _callbacks;

    extensions::Tracer _tracer;
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
