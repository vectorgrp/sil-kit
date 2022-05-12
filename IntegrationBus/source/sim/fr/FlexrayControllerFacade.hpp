// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/sim/fr/IFlexrayController.hpp"
#include "ib/mw/fwd_decl.hpp"

#include <tuple>
#include <vector>

#include "IIbToFlexrayControllerFacade.hpp"
#include "IParticipantInternal.hpp"
#include "IIbServiceEndpoint.hpp"
#include "ITraceMessageSource.hpp"

#include "FlexrayControllerProxy.hpp"
#include "ParticipantConfiguration.hpp"

namespace ib {
namespace sim {
namespace fr {

/*! \brief FlexRay Controller implementation for Network Simulator usage
 *
 * Acts as a proxy to the controllers implemented and simulated by the Network Simulator. For operation
 * without a Network Simulator cf. FrController.
 */
class FlexrayControllerFacade
    : public IFlexrayController
    , public IIbToFlexrayControllerFacade
    , public extensions::ITraceMessageSource
    , public mw::IIbServiceEndpoint
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    FlexrayControllerFacade() = delete;
    FlexrayControllerFacade(FlexrayControllerFacade&&) = default;
    FlexrayControllerFacade(mw::IParticipantInternal* participant, cfg::FlexrayController config, mw::sync::ITimeProvider* timeProvider);

public:
    // ----------------------------------------
    // Operator Implementations
    FlexrayControllerFacade& operator=(FlexrayControllerFacade&& other) = default;

public:
    // ----------------------------------------
    // Public interface methods
    //
    // IFlexrayController
    void Configure(const FlexrayControllerConfig& config) override;

    void ReconfigureTxBuffer(uint16_t txBufferIdx, const FlexrayTxBufferConfig& config) override;

    /*! \brief Update the content of a previously configured TX buffer.
     *
     * The FlexRay message will be sent immediately and only once.
     * I.e., the configuration according to cycle, repetition, and transmission mode is
     * ignored. In particular, even with FlexrayTransmissionMode::Continuous, the message will be
     * sent only once.
     *
     *  \see IFlexrayController::Configure(const FlexrayControllerConfig&)
     */
    void UpdateTxBuffer(const FlexrayTxBufferUpdate& update) override;

    void Run() override;
    void DeferredHalt() override;
    void Freeze() override;
    void AllowColdstart() override;
    void AllSlots() override;
    void Wakeup() override;

    void AddFrameHandler(FrameHandler handler) override;
    void AddFrameTransmitHandler(FrameTransmitHandler handler) override;
    void AddWakeupHandler(WakeupHandler handler) override;
    void AddPocStatusHandler(PocStatusHandler handler) override;
    void AddSymbolHandler(SymbolHandler handler) override;
    void AddSymbolTransmitHandler(SymbolTransmitHandler handler) override;
    void AddCycleStartHandler(CycleStartHandler handler) override;

    // IIbToFlexrayControllerFacade
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const FlexrayFrameEvent& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const FlexrayFrameTransmitEvent& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const FlexraySymbolEvent& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const FlexraySymbolTransmitEvent& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const FlexrayCycleStartEvent& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const FlexrayPocStatusEvent& msg) override;

    // ITraceMessageSource
    void AddSink(extensions::ITraceMessageSink* sink) override;

    // IIbServiceEndpoint
    void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;

private:
    // ----------------------------------------
    // Private helper methods
    //
    auto AllowForwardToProxy(const IIbServiceEndpoint* from) const -> bool;
    auto IsNetworkSimulated() const -> bool;
    auto IsRelevantNetwork(const mw::ServiceDescriptor& remoteServiceDescriptor) const -> bool;

private:
    // ----------------------------------------
    // private members
    mw::IParticipantInternal* _participant = nullptr;
    mw::ServiceDescriptor _serviceDescriptor;
    cfg::FlexrayController _config;

    bool _simulatedLinkDetected = false;
    mw::ServiceDescriptor _simulatedLink;

    IFlexrayController* _currentController;
    std::unique_ptr<FlexrayControllerProxy> _flexrayControllerProxy;

};
} // namespace fr
} // namespace sim
} // namespace ib
