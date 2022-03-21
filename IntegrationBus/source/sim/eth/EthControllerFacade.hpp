// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <memory>

#include "ib/sim/eth/IEthController.hpp"
#include "ib/extensions/ITraceMessageSource.hpp"
#include "ib/mw/fwd_decl.hpp"

#include "IIbToEthControllerFacade.hpp"
#include "IComAdapterInternal.hpp"

#include "EthController.hpp"
#include "EthControllerProxy.hpp"
#include "ParticipantConfiguration.hpp"

namespace ib {
namespace sim {
namespace eth {


class EthControllerFacade
    : public IEthController
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
    EthControllerFacade(mw::IComAdapterInternal* comAdapter, cfg::EthernetController config,
                        mw::sync::ITimeProvider* timeProvider);

public:
    // ----------------------------------------
    // Operator Implementations
    EthControllerFacade& operator=(EthControllerFacade&& other) = default;

public:
    // ----------------------------------------
    // Public interface methods
    //
    // IEthController
    void Activate() override;
    void Deactivate() override;

    auto SendMessage(EthMessage msg) -> EthTxId;

    auto SendFrame(EthFrame frame) -> EthTxId override;
    auto SendFrame(EthFrame frame, std::chrono::nanoseconds timestamp) -> EthTxId override;

    void RegisterReceiveMessageHandler(ReceiveMessageHandler handler) override;
    void RegisterMessageAckHandler(MessageAckHandler handler) override;
    void RegisterStateChangedHandler(StateChangedHandler handler) override;
    void RegisterBitRateChangedHandler(BitRateChangedHandler handler) override;

    // IIbToEthController
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const EthMessage& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const EthTransmitAcknowledge& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const EthStatus& msg) override;

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
    mw::IComAdapterInternal* _comAdapter = nullptr;
    mw::ServiceDescriptor _serviceDescriptor;
    cfg::EthernetController _config;

    bool _simulatedLinkDetected = false;
    mw::ServiceDescriptor _simulatedLink;

    IEthController* _currentController;
    std::unique_ptr<EthController> _ethController;
    std::unique_ptr<EthControllerProxy> _ethControllerProxy;
};

} // namespace eth
} // namespace sim
} // namespace ib
