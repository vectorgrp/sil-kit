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
    EthControllerFacade(const EthControllerFacade&) = default;
    EthControllerFacade(EthControllerFacade&&) = default;
    EthControllerFacade(mw::IComAdapterInternal* comAdapter, cfg::EthernetController config, mw::sync::ITimeProvider* timeProvider);

public:
    // ----------------------------------------
    // Operator Implementations
    EthControllerFacade& operator=(EthControllerFacade& other) = default;
    EthControllerFacade& operator=(EthControllerFacade&& other) = default;

public:
    // ----------------------------------------
    // Public interface methods
    //
    // IEthController
    void Activate() override;
    void Deactivate() override;

    [[deprecated("For MDF4 support, you should migrate to the SendFrame(...) API")]]
    auto SendMessage(EthMessage msg) -> EthTxId override;

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

    void SetEndpointAddress(const mw::EndpointAddress& endpointAddress) override;
    auto EndpointAddress() const -> const mw::EndpointAddress& override;

    // ITraceMessageSource
    inline void AddSink(extensions::ITraceMessageSink* sink) override;

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;

private:
    // ----------------------------------------
    // Private helper methods
    //
    auto DefaultFilter(const IIbServiceEndpoint* from) const -> bool;
    auto ProxyFilter(const IIbServiceEndpoint* from) const -> bool;
    auto IsLinkSimulated() const -> bool;

private:
    // ----------------------------------------
    // private members
    mw::IComAdapterInternal* _comAdapter = nullptr;
    ::ib::mw::ServiceDescriptor _serviceDescriptor;

    mw::ServiceDescriptor _remoteBusSimulator;

    IEthController* _currentController;
    std::unique_ptr<EthController> _ethController;
    std::unique_ptr<EthControllerProxy> _ethControllerProxy;
};

} // namespace eth
} // namespace sim
} // namespace ib
