// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>
#include <map>

#include "ib/sim/eth/IEthernetController.hpp"
#include "ib/mw/fwd_decl.hpp"

#include "IIbToEthControllerProxy.hpp"
#include "IParticipantInternal.hpp"
#include "ITraceMessageSource.hpp"

#include "ParticipantConfiguration.hpp"

namespace ib {
namespace sim {
namespace eth {


class EthControllerProxy
    : public IEthernetController
    , public IIbToEthControllerProxy
    , public extensions::ITraceMessageSource
    , public mw::IIbServiceEndpoint
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    EthControllerProxy() = delete;
    EthControllerProxy(const EthControllerProxy&) = default;
    EthControllerProxy(EthControllerProxy&&) = default;
    EthControllerProxy(mw::IParticipantInternal* participant, cfg::EthernetController config, IEthernetController* facade = nullptr);

public:
    // ----------------------------------------
    // Operator Implementations
    EthControllerProxy& operator=(EthControllerProxy& other) = default;
    EthControllerProxy& operator=(EthControllerProxy&& other) = default;

public:
    // ----------------------------------------
    // Public interface methods
    //
    // IEthernetController
    void Activate() override;
    void Deactivate() override;

    auto SendFrameEvent(EthernetFrameEvent msg) -> EthernetTxId;

    auto SendFrame(EthernetFrame frame) -> EthernetTxId override;

    void AddFrameHandler(FrameHandler handler) override;
    void AddFrameTransmitHandler(FrameTransmitHandler handler) override;
    void AddStateChangeHandler(StateChangeHandler handler) override;
    void AddBitrateChangeHandler(BitrateChangeHandler handler) override;

    // IIbToEthController
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const EthernetFrameEvent& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const EthernetFrameTransmitEvent& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const EthernetStatus& msg) override;

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
    void RegisterHandler(CallbackT<MsgT>&& handler);

    template<typename MsgT>
    void CallHandlers(const MsgT& msg);

    inline auto MakeTxId() -> EthernetTxId;

private:
    // ----------------------------------------
    // private members
    mw::IParticipantInternal* _participant = nullptr;
    ::ib::mw::ServiceDescriptor _serviceDescriptor;
    IEthernetController* _facade{nullptr};

    EthernetTxId _ethernetTxId = 0;
    EthernetState _ethState = EthernetState::Inactive;
    uint32_t _ethBitRate = 0;

    std::tuple<
        CallbackVector<EthernetFrameEvent>,
        CallbackVector<EthernetFrameTransmitEvent>,
        CallbackVector<EthernetStateChangeEvent>,
        CallbackVector<EthernetBitrateChangeEvent>
    > _callbacks;

    extensions::Tracer _tracer;
    std::map<EthernetTxId, EthernetFrame> _transmittedMessages;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
auto EthControllerProxy::MakeTxId() -> EthernetTxId
{
    return ++_ethernetTxId;
}

void EthControllerProxy::AddSink(extensions::ITraceMessageSink* sink)
{
    _tracer.AddSink(ib::mw::EndpointAddress{}, *sink);
}
void EthControllerProxy::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}
auto EthControllerProxy::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace eth
} // namespace sim
} // namespace ib
