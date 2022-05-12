// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/sim/fr/IFlexrayController.hpp"
#include "ib/mw/fwd_decl.hpp"

#include <tuple>
#include <vector>

#include "IIbToFlexrayControllerProxy.hpp"
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
class FlexrayControllerProxy
    : public IFlexrayController
    , public IIbToFlexrayControllerProxy
    , public extensions::ITraceMessageSource
    , public mw::IIbServiceEndpoint
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    FlexrayControllerProxy() = delete;
    FlexrayControllerProxy(const FlexrayControllerProxy&) = default;
    FlexrayControllerProxy(FlexrayControllerProxy&&) = default;
    FlexrayControllerProxy(mw::IParticipantInternal* participant, cfg::FlexrayController config,
                           IFlexrayController* facade = nullptr);

public:
    // ----------------------------------------
    // Operator Implementations
    FlexrayControllerProxy& operator=(FlexrayControllerProxy& other) = default;
    FlexrayControllerProxy& operator=(FlexrayControllerProxy&& other) = default;

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

    // IIbToFlexrayControllerProxy
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
    void RegisterHandler(CallbackT<MsgT> handler);

    template<typename MsgT>
    void CallHandlers(const MsgT& msg);

    template<typename MsgT>
    inline void SendIbMessage(MsgT&& msg);

    // Check, which config parameters are configurable
    bool IsClusterParametersConfigurable();
    bool IsNodeParametersConfigurable();
    bool IsTxBufferConfigsConfigurable();

private:
    // ----------------------------------------
    // private members
    mw::IParticipantInternal* _participant = nullptr;
    ::ib::mw::ServiceDescriptor _serviceDescriptor;
    IFlexrayController* _facade = nullptr;

    std::vector<FlexrayTxBufferConfig> _bufferConfigs;

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
void FlexrayControllerProxy::AddSink(extensions::ITraceMessageSink* sink)
{
    _tracer.AddSink(ib::mw::EndpointAddress{}, *sink);
}

void FlexrayControllerProxy::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto FlexrayControllerProxy::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}
} // namespace fr
} // SimModels
} // namespace ib
