// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/sim/fr/IFlexrayController.hpp"
#include "ib/mw/fwd_decl.hpp"

#include <tuple>
#include <vector>

#include "IIbToFlexrayController.hpp"
#include "IParticipantInternal.hpp"
#include "IIbServiceEndpoint.hpp"
#include "ITraceMessageSource.hpp"

#include "ParticipantConfiguration.hpp"

namespace ib {
namespace sim {
namespace fr {

/*! \brief FlexRay Controller implementation for Network Simulator usage
 *
 * Acts as a proxy to the controllers implemented and simulated by the Network Simulator. For operation
 * without a Network Simulator cf. FrController.
 */
class FlexrayController
    : public IFlexrayController
    , public IIbToFlexrayController
    , public extensions::ITraceMessageSource
    , public mw::IIbServiceEndpoint
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    FlexrayController() = delete;
    FlexrayController(const FlexrayController&) = default;
    FlexrayController(FlexrayController&&) = default;
    FlexrayController(mw::IParticipantInternal* participant, cfg::FlexrayController config,
                      mw::sync::ITimeProvider* /*timeProvider*/);

public:
    // ----------------------------------------
    // Operator Implementations
    FlexrayController& operator=(FlexrayController& other) = default;
    FlexrayController& operator=(FlexrayController&& other) = default;

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

    // IIbToFlexrayController
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const FlexrayFrameEvent& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const FlexrayFrameTransmitEvent& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const FlexraySymbolEvent& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const FlexraySymbolTransmitEvent& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const FlexrayCycleStartEvent& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const FlexrayPocStatusEvent& msg) override;

    // ITraceMessageSource
    inline void AddSink(extensions::ITraceMessageSink* sink) override;

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;

public:
    // ----------------------------------------
    // Public  methods
    //
    void SetDetailedBehavior(const mw::ServiceDescriptor& remoteServiceDescriptor);

    void RegisterServiceDiscovery();

private:
    void WarnOverride(const std::string& parameterName);

private:
    // ----------------------------------------
    // private data types
    template<typename MsgT>
    using CallbackVector = std::vector<CallbackT<MsgT>>;

private:
    // ----------------------------------------
    // private methods
    template<typename MsgT>
    void AddHandler(CallbackT<MsgT> handler);

    template<typename MsgT>
    void CallHandlers(const MsgT& msg);

    template<typename MsgT>
    inline void SendIbMessage(MsgT&& msg);

    // Check, which config parameters are configurable
    bool IsClusterParametersConfigurable();
    bool IsNodeParametersConfigurable();
    bool IsTxBufferConfigsConfigurable();

    auto IsRelevantNetwork(const mw::ServiceDescriptor& remoteServiceDescriptor) const -> bool;
    auto AllowReception(const IIbServiceEndpoint* from) const -> bool;

private:
    // ----------------------------------------
    // private members
    mw::IParticipantInternal* _participant = nullptr;
    ::ib::mw::ServiceDescriptor _serviceDescriptor;
    std::vector<FlexrayTxBufferConfig> _bufferConfigs;

    bool _simulatedLinkDetected = false;
    mw::ServiceDescriptor _simulatedLink;

    std::tuple<
        CallbackVector<FlexrayFrameEvent>,
        CallbackVector<FlexrayFrameTransmitEvent>,
        CallbackVector<FlexraySymbolEvent>,
        CallbackVector<FlexraySymbolTransmitEvent>,
        CallbackVector<FlexrayCycleStartEvent>,
        CallbackVector<FlexrayPocStatusEvent>,
        CallbackVector<FlexrayWakeupEvent>
    > _callbacks;

    extensions::Tracer _tracer;

    cfg::FlexrayController _config;
};


// ==================================================================
//  Inline Implementations
// ==================================================================
void FlexrayController::AddSink(extensions::ITraceMessageSink* sink)
{
    _tracer.AddSink(ib::mw::EndpointAddress{}, *sink);
}

void FlexrayController::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto FlexrayController::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}
} // namespace fr
} // SimModels
} // namespace ib
