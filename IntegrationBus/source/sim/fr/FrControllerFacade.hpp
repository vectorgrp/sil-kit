// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/sim/fr/IFrController.hpp"
#include "ib/extensions/ITraceMessageSource.hpp"
#include "ib/mw/fwd_decl.hpp"

#include <tuple>
#include <vector>

#include "IIbToFrControllerFacade.hpp"
#include "IComAdapterInternal.hpp"
#include "IIbServiceEndpoint.hpp"

#include "FrController.hpp"
#include "FrControllerProxy.hpp"

namespace ib {
namespace sim {
namespace fr {

/*! \brief FlexRay Controller implementation for Network Simulator usage
 *
 * Acts as a proxy to the controllers implemented and simulated by the Network Simulator. For operation
 * without a Network Simulator cf. FrController.
 */
class FrControllerFacade
    : public IFrController
    , public IIbToFrControllerFacade
    , public mw::sync::ITimeConsumer
    , public extensions::ITraceMessageSource
    , public mw::IIbServiceEndpoint
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    FrControllerFacade() = delete;
    FrControllerFacade(const FrControllerFacade&) = default;
    FrControllerFacade(FrControllerFacade&&) = default;
    FrControllerFacade(mw::IComAdapterInternal* comAdapter, cfg::FlexrayController config, mw::sync::ITimeProvider* timeProvider);

public:
    // ----------------------------------------
    // Operator Implementations
    FrControllerFacade& operator=(FrControllerFacade& other) = default;
    FrControllerFacade& operator=(FrControllerFacade&& other) = default;

public:
    // ----------------------------------------
    // Public interface methods
    //
    // IFrController
    void Configure(const ControllerConfig& config) override;

    void ReconfigureTxBuffer(uint16_t txBufferIdx, const TxBufferConfig& config) override;

    /*! \brief Update the content of a previously configured TX buffer.
     *
     * The FlexRay message will be sent immediately and only once.
     * I.e., the configuration according to cycle, repetition, and transmission mode is
     * ignored. In particular, even with TransmissionMode::Continuous, the message will be
     * sent only once.
     *
     *  \see IFrController::Configure(const ControllerConfig&)
     */
    void UpdateTxBuffer(const TxBufferUpdate& update) override;

    void Run() override;
    void DeferredHalt() override;
    void Freeze() override;
    void AllowColdstart() override;
    void AllSlots() override;
    void Wakeup() override;

    void RegisterMessageHandler(MessageHandler handler) override;
    void RegisterMessageAckHandler(MessageAckHandler handler) override;
    void RegisterWakeupHandler(WakeupHandler handler) override;

    [[deprecated("superseded by RegisterPocStatusHandler")]] 
    void RegisterControllerStatusHandler(ControllerStatusHandler handler) override;

    void RegisterPocStatusHandler(PocStatusHandler handler) override;
    void RegisterSymbolHandler(SymbolHandler handler) override;
    void RegisterSymbolAckHandler(SymbolAckHandler handler) override;
    void RegisterCycleStartHandler(CycleStartHandler handler) override;

    // IIbToFrController
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const FrMessage& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const FrMessageAck& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const FrSymbol& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const FrSymbolAck& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const CycleStart& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const PocStatus& msg) override;

    //ITimeConsumer
    void SetTimeProvider(mw::sync::ITimeProvider* timeProvider) override;

    // ITraceMessageSource
    void AddSink(extensions::ITraceMessageSink* sink) override;

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
    cfg::FlexrayController _config;

    bool _simulatedLinkDetected = false;
    mw::ServiceDescriptor _simulatedLink;

    IFrController* _currentController;
    std::unique_ptr<FrController> _frController;
    std::unique_ptr<FrControllerProxy> _frControllerProxy;

};
} // namespace fr
} // namespace sim
} // namespace ib
