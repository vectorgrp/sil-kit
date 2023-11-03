/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include "silkit/services/flexray/IFlexrayController.hpp"

#include <tuple>
#include <vector>

#include "IMsgForFlexrayController.hpp"
#include "IParticipantInternal.hpp"
#include "IServiceEndpoint.hpp"
#include "ITraceMessageSource.hpp"
#include "ITimeProvider.hpp"

#include "ParticipantConfiguration.hpp"

#include "SynchronizedHandlers.hpp"

namespace SilKit {
namespace Services {
namespace Flexray {

/*! \brief FlexRay Controller implementation for network simulator usage
 *
 * Acts as a proxy to the controllers implemented and simulated by the network simulator.
 */
class FlexrayController
    : public IFlexrayController
    , public IMsgForFlexrayController
    , public ITraceMessageSource
    , public Core::IServiceEndpoint
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors and Destructor
    FlexrayController() = delete;
    FlexrayController(const FlexrayController&) = delete;
    FlexrayController(FlexrayController&&) = delete;
    FlexrayController(Core::IParticipantInternal* participant, Config::FlexrayController config,
                      Services::Orchestration::ITimeProvider* /*timeProvider*/);

public:
    // ----------------------------------------
    // Operator Implementations
    FlexrayController& operator=(FlexrayController& other) = delete;
    FlexrayController& operator=(FlexrayController&& other) = delete;

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

    HandlerId AddFrameHandler(FrameHandler handler) override;
    HandlerId AddFrameTransmitHandler(FrameTransmitHandler handler) override;
    HandlerId AddWakeupHandler(WakeupHandler handler) override;
    HandlerId AddPocStatusHandler(PocStatusHandler handler) override;
    HandlerId AddSymbolHandler(SymbolHandler handler) override;
    HandlerId AddSymbolTransmitHandler(SymbolTransmitHandler handler) override;
    HandlerId AddCycleStartHandler(CycleStartHandler handler) override;

    void RemoveFrameHandler(HandlerId handlerId) override;
    void RemoveFrameTransmitHandler(HandlerId handlerId) override;
    void RemoveWakeupHandler(HandlerId handlerId) override;
    void RemovePocStatusHandler(HandlerId handlerId) override;
    void RemoveSymbolHandler(HandlerId handlerId) override;
    void RemoveSymbolTransmitHandler(HandlerId handlerId) override;
    void RemoveCycleStartHandler(HandlerId handlerId) override;

    // IMsgForFlexrayController
    void ReceiveMsg(const IServiceEndpoint* from, const WireFlexrayFrameEvent& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from, const WireFlexrayFrameTransmitEvent& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from, const FlexraySymbolEvent& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from, const FlexraySymbolTransmitEvent& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from, const FlexrayCycleStartEvent& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from, const FlexrayPocStatusEvent& msg) override;

    // ITraceMessageSource
    inline void AddSink(ITraceMessageSink* sink, SilKit::Config::NetworkType networkType) override;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor & override;

public:
    // ----------------------------------------
    // Public  methods
    //
    void SetDetailedBehavior(const Core::ServiceDescriptor& remoteServiceDescriptor);

    void RegisterServiceDiscovery();

private:
    void WarnOverride(const std::string& parameterName);

private:
    // ----------------------------------------
    // private methods
    template <typename MsgT>
    HandlerId AddHandler(CallbackT<MsgT> handler);

    template <typename MsgT>
    auto RemoveHandler(HandlerId handlerId) -> bool;

    template <typename MsgT>
    void CallHandlers(const MsgT& msg);

    template<typename MsgT>
    inline void SendMsg(MsgT&& msg);

    // Check, which config parameters are configurable
    bool IsClusterParametersConfigurable();
    bool IsNodeParametersConfigurable();
    bool IsTxBufferConfigsConfigurable();

    auto IsRelevantNetwork(const Core::ServiceDescriptor& remoteServiceDescriptor) const -> bool;
    auto AllowReception(const IServiceEndpoint* from) const -> bool;

private:
    // ----------------------------------------
    // private members
    Core::IParticipantInternal* _participant = nullptr;
    Config::FlexrayController _config;
    ::SilKit::Core::ServiceDescriptor _serviceDescriptor;
    std::vector<FlexrayTxBufferConfig> _bufferConfigs;
    Tracer _tracer;
    Services::Logging::ILogger* _logger;
    bool _simulatedLinkDetected = false;
    Core::ServiceDescriptor _simulatedLink;

    template <typename MsgT>
    using CallbacksT = Util::SynchronizedHandlers<CallbackT<MsgT>>;

    std::tuple<
        CallbacksT<FlexrayFrameEvent>,
        CallbacksT<FlexrayFrameTransmitEvent>,
        CallbacksT<FlexraySymbolEvent>,
        CallbacksT<FlexraySymbolTransmitEvent>,
        CallbacksT<FlexrayCycleStartEvent>,
        CallbacksT<FlexrayPocStatusEvent>,
        CallbacksT<FlexrayWakeupEvent>
    > _callbacks;
};

// ==================================================================
//  Inline Implementations
// ==================================================================
void FlexrayController::AddSink(ITraceMessageSink* sink, SilKit::Config::NetworkType /*networkType*/)
{
    _tracer.AddSink(GetServiceDescriptor(), *sink);
}

void FlexrayController::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto FlexrayController::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}
} // namespace Flexray
} // namespace Services
} // namespace SilKit
