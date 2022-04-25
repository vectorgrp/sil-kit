// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>

#include "ib/sim/eth/IEthernetController.hpp"
#include "ib/mw/fwd_decl.hpp"

#include "IIbToEthControllerFacade.hpp"
#include "IParticipantInternal.hpp"
#include "ITraceMessageSource.hpp"

#include "EthController.hpp"
#include "EthControllerProxy.hpp"
#include "ParticipantConfiguration.hpp"

namespace ib {
namespace sim {
namespace eth {


class EthControllerFacade
    : public IEthernetController
    , public IIbToEthControllerFacade
    , public extensions::ITraceMessageSource
    , public mw::IIbServiceEndpoint
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    EthControllerFacade() = delete;
    EthControllerFacade(EthControllerFacade&&) = default;
    EthControllerFacade(mw::IParticipantInternal* participant, cfg::EthernetController config,
                        mw::sync::ITimeProvider* timeProvider);

public:
    // ----------------------------------------
    // Operator Implementations
    EthControllerFacade& operator=(EthControllerFacade&& other) = default;

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
    void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;

private:
    // ----------------------------------------
    // Private helper methods
    //
    auto AllowForwardToDefault(const IIbServiceEndpoint* from) const -> bool;
    auto AllowForwardToProxy(const IIbServiceEndpoint* from) const -> bool;
    auto IsNetworkSimulated() const -> bool;
    auto IsRelevantNetwork(const mw::ServiceDescriptor& remoteServiceDescriptor) const -> bool;

private:
    // ----------------------------------------
    // private members
    mw::IParticipantInternal* _participant = nullptr;
    mw::ServiceDescriptor _serviceDescriptor;
    cfg::EthernetController _config;

    bool _simulatedLinkDetected = false;
    mw::ServiceDescriptor _simulatedLink;

    IEthernetController* _currentController;
    std::unique_ptr<EthController> _ethController;
    std::unique_ptr<EthControllerProxy> _ethControllerProxy;
};

} // namespace eth
} // namespace sim
} // namespace ib
