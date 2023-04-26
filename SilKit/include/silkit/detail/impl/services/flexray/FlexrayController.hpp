// Copyright (c) 2023 Vector Informatik GmbH
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include "silkit/capi/Flexray.h"

#include "silkit/services/flexray/IFlexrayController.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Services {
namespace Flexray {

class FlexrayController : public SilKit::Services::Flexray::IFlexrayController
{
public:
    inline FlexrayController(SilKit_Participant *participant, const std::string &canonicalName,
                             const std::string &networkName);

    inline ~FlexrayController() override = default;

    inline void Configure(SilKit::Services::Flexray::FlexrayControllerConfig const &config) override;

    inline void ReconfigureTxBuffer(
        uint16_t txBufferIdx,
        SilKit::Services::Flexray::FlexrayTxBufferConfig const &cxxFlexrayTxBufferConfig) override;

    inline void UpdateTxBuffer(
        const SilKit::Services::Flexray::FlexrayTxBufferUpdate &cxxFlexrayTxBufferUpdate) override;

    inline void Run() override;

    inline void DeferredHalt() override;

    inline void Freeze() override;

    inline void AllowColdstart() override;

    inline void AllSlots() override;

    inline void Wakeup() override;

    inline auto AddFrameHandler(FrameHandler handler) -> Util::HandlerId override;

    inline void RemoveFrameHandler(Util::HandlerId handlerId) override;

    inline auto AddFrameTransmitHandler(FrameTransmitHandler handler) -> Util::HandlerId override;

    inline void RemoveFrameTransmitHandler(Util::HandlerId handlerId) override;

    inline auto AddWakeupHandler(WakeupHandler handler) -> Util::HandlerId override;

    inline void RemoveWakeupHandler(Util::HandlerId handlerId) override;

    inline auto AddPocStatusHandler(PocStatusHandler handler) -> Util::HandlerId override;

    inline void RemovePocStatusHandler(Util::HandlerId handlerId) override;

    inline auto AddSymbolHandler(SymbolHandler handler) -> Util::HandlerId override;

    inline void RemoveSymbolHandler(Util::HandlerId handlerId) override;

    inline auto AddSymbolTransmitHandler(SymbolTransmitHandler handler) -> Util::HandlerId override;

    inline void RemoveSymbolTransmitHandler(Util::HandlerId handlerId) override;

    inline auto AddCycleStartHandler(CycleStartHandler handler) -> Util::HandlerId override;

    inline void RemoveCycleStartHandler(Util::HandlerId handlerId) override;

private:
    template <typename HandlerFunction>
    struct HandlerData
    {
        SilKit::Services::Flexray::IFlexrayController *controller{nullptr};
        HandlerFunction handler{};
    };

    template <typename HandlerFunction>
    using HandlerDataMap = std::unordered_map<SilKit::Util::HandlerId, std::unique_ptr<HandlerData<HandlerFunction>>>;

private:
    SilKit_FlexrayController *_flexrayController{nullptr};

    HandlerDataMap<FrameHandler> _frameHandlers;
    HandlerDataMap<FrameTransmitHandler> _frameTransmitHandlers;
    HandlerDataMap<WakeupHandler> _wakeupHandlers;
    HandlerDataMap<PocStatusHandler> _pocStatusHandlers;
    HandlerDataMap<SymbolHandler> _symbolHandlers;
    HandlerDataMap<SymbolTransmitHandler> _symbolTransmitHandlers;
    HandlerDataMap<CycleStartHandler> _cycleStartHandlers;
};

} // namespace Flexray
} // namespace Services
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


// ================================================================================
//  Inline Implementations
// ================================================================================

#include "silkit/detail/impl/ThrowOnError.hpp"

namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Services {
namespace Flexray {

namespace {

inline void CxxToC(const SilKit::Services::Flexray::FlexrayClusterParameters &cxxFlexrayClusterParameters,
                   SilKit_FlexrayClusterParameters &cFlexrayClusterParameters);

inline void CxxToC(const SilKit::Services::Flexray::FlexrayNodeParameters &cxxFlexrayNodeParameters,
                   SilKit_FlexrayNodeParameters &cFlexrayNodeParameters);

inline void CxxToC(const SilKit::Services::Flexray::FlexrayTxBufferConfig &cxxFlexrayTxBufferConfig,
                   SilKit_FlexrayTxBufferConfig &cFlexrayTxBufferConfig);

inline void CxxToC(const SilKit::Services::Flexray::FlexrayTxBufferUpdate &cxxFlexrayTxBufferUpdate,
                   SilKit_FlexrayTxBufferUpdate &cFlexrayTxBufferUpdate);

inline void CToCxx(const SilKit_FlexrayHeader &cFlexrayHeader,
                   SilKit::Services::Flexray::FlexrayHeader &cxxFlexrayHeader);

} // namespace

FlexrayController::FlexrayController(SilKit_Participant *participant, const std::string &canonicalName,
                                     const std::string &networkName)
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    const auto returnCode =
        SilKit_FlexrayController_Create(&_flexrayController, participant, canonicalName.c_str(), networkName.c_str());
    ThrowOnError(returnCode);
}

void FlexrayController::Configure(SilKit::Services::Flexray::FlexrayControllerConfig const &config)
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    SilKit_FlexrayClusterParameters cFlexrayClusterParameters;
    CxxToC(config.clusterParams, cFlexrayClusterParameters);

    SilKit_FlexrayNodeParameters cFlexrayNodeParameters;
    CxxToC(config.nodeParams, cFlexrayNodeParameters);

    std::vector<SilKit_FlexrayTxBufferConfig> cFlexrayTxBufferConfigs;
    for (const auto &cxxFlexrayTxBufferConfigs : config.bufferConfigs)
    {
        SilKit_FlexrayTxBufferConfig cFlexrayTxBufferConfig;
        CxxToC(cxxFlexrayTxBufferConfigs, cFlexrayTxBufferConfig);

        cFlexrayTxBufferConfigs.emplace_back(cFlexrayTxBufferConfig);
    }

    SilKit_FlexrayControllerConfig cFlexrayControllerConfig;
    SilKit_Struct_Init(SilKit_FlexrayControllerConfig, cFlexrayControllerConfig);
    cFlexrayControllerConfig.clusterParams = &cFlexrayClusterParameters;
    cFlexrayControllerConfig.nodeParams = &cFlexrayNodeParameters;
    cFlexrayControllerConfig.numBufferConfigs = static_cast<uint32_t>(cFlexrayTxBufferConfigs.size());
    cFlexrayControllerConfig.bufferConfigs = cFlexrayTxBufferConfigs.data();

    const auto returnCode = SilKit_FlexrayController_Configure(_flexrayController, &cFlexrayControllerConfig);
    ThrowOnError(returnCode);
}

void FlexrayController::ReconfigureTxBuffer(
    uint16_t txBufferIdx, SilKit::Services::Flexray::FlexrayTxBufferConfig const &cxxFlexrayTxBufferConfig)
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    SilKit_FlexrayTxBufferConfig cFlexrayTxBufferConfig;
    CxxToC(cxxFlexrayTxBufferConfig, cFlexrayTxBufferConfig);

    const auto returnCode =
        SilKit_FlexrayController_ReconfigureTxBuffer(_flexrayController, txBufferIdx, &cFlexrayTxBufferConfig);
    ThrowOnError(returnCode);
}

void FlexrayController::UpdateTxBuffer(const SilKit::Services::Flexray::FlexrayTxBufferUpdate &cxxFlexrayTxBufferUpdate)
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    SilKit_FlexrayTxBufferUpdate cFlexrayTxBufferUpdate;
    CxxToC(cxxFlexrayTxBufferUpdate, cFlexrayTxBufferUpdate);

    const auto returnCode = SilKit_FlexrayController_UpdateTxBuffer(_flexrayController, &cFlexrayTxBufferUpdate);
    ThrowOnError(returnCode);
}

void FlexrayController::Run()
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST
    const auto returnCode = SilKit_FlexrayController_ExecuteCmd(_flexrayController, SilKit_FlexrayChiCommand_RUN);
    ThrowOnError(returnCode);
}

void FlexrayController::DeferredHalt()
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST
    const auto returnCode =
        SilKit_FlexrayController_ExecuteCmd(_flexrayController, SilKit_FlexrayChiCommand_DEFERRED_HALT);
    ThrowOnError(returnCode);
}

void FlexrayController::Freeze()
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST
    const auto returnCode = SilKit_FlexrayController_ExecuteCmd(_flexrayController, SilKit_FlexrayChiCommand_FREEZE);
    ThrowOnError(returnCode);
}

void FlexrayController::AllowColdstart()
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST
    const auto returnCode =
        SilKit_FlexrayController_ExecuteCmd(_flexrayController, SilKit_FlexrayChiCommand_ALLOW_COLDSTART);
    ThrowOnError(returnCode);
}

void FlexrayController::AllSlots()
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST
    const auto returnCode = SilKit_FlexrayController_ExecuteCmd(_flexrayController, SilKit_FlexrayChiCommand_ALL_SLOTS);
    ThrowOnError(returnCode);
}

void FlexrayController::Wakeup()
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST
    const auto returnCode = SilKit_FlexrayController_ExecuteCmd(_flexrayController, SilKit_FlexrayChiCommand_WAKEUP);
    ThrowOnError(returnCode);
}

auto FlexrayController::AddFrameHandler(FrameHandler handler) -> Util::HandlerId
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    const auto cHandler = [](void *context, SilKit_FlexrayController *controller,
                             const SilKit_FlexrayFrameEvent *message) {
        SILKIT_UNUSED_ARG(controller);

        SilKit::Services::Flexray::FlexrayFrameEvent event{};
        event.timestamp = std::chrono::nanoseconds{message->timestamp};
        event.channel = static_cast<SilKit::Services::Flexray::FlexrayChannel>(message->channel);
        CToCxx(*message->frame->header, event.frame.header);
        event.frame.payload = SilKit::Util::ToSpan(message->frame->payload);

        const auto data = static_cast<HandlerData<FrameHandler> *>(context);
        data->handler(data->controller, event);
    };

    SilKit_HandlerId handlerId;

    auto handlerData = std::make_unique<HandlerData<FrameHandler>>();
    handlerData->controller = this;
    handlerData->handler = std::move(handler);

    const auto returnCode =
        SilKit_FlexrayController_AddFrameHandler(_flexrayController, handlerData.get(), cHandler, &handlerId);
    ThrowOnError(returnCode);

    _frameHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

    return static_cast<SilKit::Util::HandlerId>(handlerId);
}

void FlexrayController::RemoveFrameHandler(Util::HandlerId handlerId)
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    const auto returnCode =
        SilKit_FlexrayController_RemoveFrameHandler(_flexrayController, static_cast<SilKit_HandlerId>(handlerId));
    ThrowOnError(returnCode);

    _frameHandlers.erase(handlerId);
}

auto FlexrayController::AddFrameTransmitHandler(FrameTransmitHandler handler) -> Util::HandlerId
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    const auto cHandler = [](void *context, SilKit_FlexrayController *controller,
                             const SilKit_FlexrayFrameTransmitEvent *cEvent) {
        SILKIT_UNUSED_ARG(controller);

        SilKit::Services::Flexray::FlexrayFrameTransmitEvent cxxEvent{};
        cxxEvent.timestamp = std::chrono::nanoseconds{cEvent->timestamp};
        cxxEvent.channel = static_cast<SilKit::Services::Flexray::FlexrayChannel>(cEvent->channel);
        CToCxx(*cEvent->frame->header, cxxEvent.frame.header);
        cxxEvent.frame.payload = SilKit::Util::ToSpan(cEvent->frame->payload);
        cxxEvent.txBufferIndex = cEvent->txBufferIndex;

        const auto data = static_cast<HandlerData<FrameTransmitHandler> *>(context);
        data->handler(data->controller, cxxEvent);
    };

    SilKit_HandlerId handlerId;

    auto handlerData = std::make_unique<HandlerData<FrameTransmitHandler>>();
    handlerData->controller = this;
    handlerData->handler = std::move(handler);

    const auto returnCode =
        SilKit_FlexrayController_AddFrameTransmitHandler(_flexrayController, handlerData.get(), cHandler, &handlerId);
    ThrowOnError(returnCode);

    _frameTransmitHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

    return static_cast<SilKit::Util::HandlerId>(handlerId);
}

void FlexrayController::RemoveFrameTransmitHandler(Util::HandlerId handlerId)
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    const auto returnCode = SilKit_FlexrayController_RemoveFrameTransmitHandler(
        _flexrayController, static_cast<SilKit_HandlerId>(handlerId));
    ThrowOnError(returnCode);

    _frameTransmitHandlers.erase(handlerId);
}

auto FlexrayController::AddWakeupHandler(WakeupHandler handler) -> Util::HandlerId
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    const auto cHandler = [](void *context, SilKit_FlexrayController *controller,
                             const SilKit_FlexrayWakeupEvent *cEvent) {
        SILKIT_UNUSED_ARG(controller);

        SilKit::Services::Flexray::FlexrayWakeupEvent cxxEvent{};
        cxxEvent.timestamp = std::chrono::nanoseconds{cEvent->timestamp};
        cxxEvent.channel = static_cast<SilKit::Services::Flexray::FlexrayChannel>(cEvent->channel);
        cxxEvent.pattern = static_cast<SilKit::Services::Flexray::FlexraySymbolPattern>(cEvent->pattern);

        const auto data = static_cast<HandlerData<WakeupHandler> *>(context);
        data->handler(data->controller, cxxEvent);
    };

    SilKit_HandlerId handlerId;

    auto handlerData = std::make_unique<HandlerData<WakeupHandler>>();
    handlerData->controller = this;
    handlerData->handler = std::move(handler);

    const auto returnCode =
        SilKit_FlexrayController_AddWakeupHandler(_flexrayController, handlerData.get(), cHandler, &handlerId);
    ThrowOnError(returnCode);

    _wakeupHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

    return static_cast<SilKit::Util::HandlerId>(handlerId);
}

void FlexrayController::RemoveWakeupHandler(Util::HandlerId handlerId)
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    const auto returnCode =
        SilKit_FlexrayController_RemoveWakeupHandler(_flexrayController, static_cast<SilKit_HandlerId>(handlerId));
    ThrowOnError(returnCode);

    _wakeupHandlers.erase(handlerId);
}

auto FlexrayController::AddPocStatusHandler(PocStatusHandler handler) -> Util::HandlerId
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    const auto cHandler = [](void *context, SilKit_FlexrayController *controller,
                             const SilKit_FlexrayPocStatusEvent *cEvent) {
        SILKIT_UNUSED_ARG(controller);

        SilKit::Services::Flexray::FlexrayPocStatusEvent cxxEvent{};
        cxxEvent.timestamp = std::chrono::nanoseconds{cEvent->timestamp};
        cxxEvent.state = static_cast<SilKit::Services::Flexray::FlexrayPocState>(cEvent->state);
        cxxEvent.chiHaltRequest = cEvent->chiHaltRequest;
        cxxEvent.chiReadyRequest = cEvent->chiReadyRequest;
        cxxEvent.coldstartNoise = cEvent->coldstartNoise;
        cxxEvent.errorMode = static_cast<SilKit::Services::Flexray::FlexrayErrorModeType>(cEvent->errorMode);
        cxxEvent.freeze = cEvent->freeze;
        cxxEvent.slotMode = static_cast<SilKit::Services::Flexray::FlexraySlotModeType>(cEvent->slotMode);
        cxxEvent.startupState = static_cast<SilKit::Services::Flexray::FlexrayStartupStateType>(cEvent->startupState);
        cxxEvent.wakeupStatus = static_cast<SilKit::Services::Flexray::FlexrayWakeupStatusType>(cEvent->wakeupStatus);

        const auto data = static_cast<HandlerData<PocStatusHandler> *>(context);
        data->handler(data->controller, cxxEvent);
    };

    SilKit_HandlerId handlerId;

    auto handlerData = std::make_unique<HandlerData<PocStatusHandler>>();
    handlerData->controller = this;
    handlerData->handler = std::move(handler);

    const auto returnCode =
        SilKit_FlexrayController_AddPocStatusHandler(_flexrayController, handlerData.get(), cHandler, &handlerId);
    ThrowOnError(returnCode);

    _pocStatusHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

    return static_cast<SilKit::Util::HandlerId>(handlerId);
}

void FlexrayController::RemovePocStatusHandler(Util::HandlerId handlerId)
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    const auto returnCode =
        SilKit_FlexrayController_RemovePocStatusHandler(_flexrayController, static_cast<SilKit_HandlerId>(handlerId));
    ThrowOnError(returnCode);

    _pocStatusHandlers.erase(handlerId);
}

auto FlexrayController::AddSymbolHandler(SymbolHandler handler) -> Util::HandlerId
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    const auto cHandler = [](void *context, SilKit_FlexrayController *controller,
                             const SilKit_FlexraySymbolEvent *cEvent) {
        SILKIT_UNUSED_ARG(controller);

        SilKit::Services::Flexray::FlexraySymbolEvent cxxEvent{};
        cxxEvent.timestamp = std::chrono::nanoseconds{cEvent->timestamp};
        cxxEvent.channel = static_cast<SilKit::Services::Flexray::FlexrayChannel>(cEvent->channel);
        cxxEvent.pattern = static_cast<SilKit::Services::Flexray::FlexraySymbolPattern>(cEvent->pattern);

        const auto data = static_cast<HandlerData<SymbolHandler> *>(context);
        data->handler(data->controller, cxxEvent);
    };

    SilKit_HandlerId handlerId;

    auto handlerData = std::make_unique<HandlerData<SymbolHandler>>();
    handlerData->controller = this;
    handlerData->handler = std::move(handler);

    const auto returnCode =
        SilKit_FlexrayController_AddSymbolHandler(_flexrayController, handlerData.get(), cHandler, &handlerId);
    ThrowOnError(returnCode);

    _symbolHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

    return static_cast<SilKit::Util::HandlerId>(handlerId);
}

void FlexrayController::RemoveSymbolHandler(Util::HandlerId handlerId)
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    const auto returnCode =
        SilKit_FlexrayController_RemoveSymbolHandler(_flexrayController, static_cast<SilKit_HandlerId>(handlerId));
    ThrowOnError(returnCode);

    _symbolHandlers.erase(handlerId);
}

auto FlexrayController::AddSymbolTransmitHandler(SymbolTransmitHandler handler) -> Util::HandlerId
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    const auto cHandler = [](void *context, SilKit_FlexrayController *controller,
                             const SilKit_FlexraySymbolTransmitEvent *cEvent) {
        SILKIT_UNUSED_ARG(controller);

        SilKit::Services::Flexray::FlexraySymbolTransmitEvent cxxEvent{};
        cxxEvent.timestamp = std::chrono::nanoseconds{cEvent->timestamp};
        cxxEvent.channel = static_cast<SilKit::Services::Flexray::FlexrayChannel>(cEvent->channel);
        cxxEvent.pattern = static_cast<SilKit::Services::Flexray::FlexraySymbolPattern>(cEvent->pattern);

        const auto data = static_cast<HandlerData<SymbolTransmitHandler> *>(context);
        data->handler(data->controller, cxxEvent);
    };

    SilKit_HandlerId handlerId;

    auto handlerData = std::make_unique<HandlerData<SymbolTransmitHandler>>();
    handlerData->controller = this;
    handlerData->handler = std::move(handler);

    const auto returnCode =
        SilKit_FlexrayController_AddSymbolTransmitHandler(_flexrayController, handlerData.get(), cHandler, &handlerId);
    ThrowOnError(returnCode);

    _symbolTransmitHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

    return static_cast<SilKit::Util::HandlerId>(handlerId);
}

void FlexrayController::RemoveSymbolTransmitHandler(Util::HandlerId handlerId)
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    const auto returnCode = SilKit_FlexrayController_RemoveSymbolTransmitHandler(
        _flexrayController, static_cast<SilKit_HandlerId>(handlerId));
    ThrowOnError(returnCode);

    _symbolTransmitHandlers.erase(handlerId);
}

auto FlexrayController::AddCycleStartHandler(CycleStartHandler handler) -> Util::HandlerId
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    const auto cHandler = [](void *context, SilKit_FlexrayController *controller,
                             const SilKit_FlexrayCycleStartEvent *cEvent) {
        SILKIT_UNUSED_ARG(controller);

        SilKit::Services::Flexray::FlexrayCycleStartEvent cxxEvent{};
        cxxEvent.timestamp = std::chrono::nanoseconds{cEvent->timestamp};
        cxxEvent.cycleCounter = cEvent->cycleCounter;

        const auto data = static_cast<HandlerData<CycleStartHandler> *>(context);
        data->handler(data->controller, cxxEvent);
    };

    SilKit_HandlerId handlerId;

    auto handlerData = std::make_unique<HandlerData<CycleStartHandler>>();
    handlerData->controller = this;
    handlerData->handler = std::move(handler);

    const auto returnCode =
        SilKit_FlexrayController_AddCycleStartHandler(_flexrayController, handlerData.get(), cHandler, &handlerId);
    ThrowOnError(returnCode);

    _cycleStartHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

    return static_cast<SilKit::Util::HandlerId>(handlerId);
}

void FlexrayController::RemoveCycleStartHandler(Util::HandlerId handlerId)
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    const auto returnCode =
        SilKit_FlexrayController_RemoveCycleStartHandler(_flexrayController, static_cast<SilKit_HandlerId>(handlerId));
    ThrowOnError(returnCode);

    _cycleStartHandlers.erase(handlerId);
}

namespace {

void CxxToC(const SilKit::Services::Flexray::FlexrayClusterParameters &cxxFlexrayClusterParameters,
            SilKit_FlexrayClusterParameters &cFlexrayClusterParameters)
{
    SilKit_Struct_Init(SilKit_FlexrayClusterParameters, cFlexrayClusterParameters);
    cFlexrayClusterParameters.gColdstartAttempts = cxxFlexrayClusterParameters.gColdstartAttempts;
    cFlexrayClusterParameters.gCycleCountMax = cxxFlexrayClusterParameters.gCycleCountMax;
    cFlexrayClusterParameters.gdActionPointOffset = cxxFlexrayClusterParameters.gdActionPointOffset;
    cFlexrayClusterParameters.gdDynamicSlotIdlePhase = cxxFlexrayClusterParameters.gdDynamicSlotIdlePhase;
    cFlexrayClusterParameters.gdMiniSlot = cxxFlexrayClusterParameters.gdMiniSlot;
    cFlexrayClusterParameters.gdMiniSlotActionPointOffset = cxxFlexrayClusterParameters.gdMiniSlotActionPointOffset;
    cFlexrayClusterParameters.gdStaticSlot = cxxFlexrayClusterParameters.gdStaticSlot;
    cFlexrayClusterParameters.gdSymbolWindow = cxxFlexrayClusterParameters.gdSymbolWindow;
    cFlexrayClusterParameters.gdSymbolWindowActionPointOffset =
        cxxFlexrayClusterParameters.gdSymbolWindowActionPointOffset;
    cFlexrayClusterParameters.gdTSSTransmitter = cxxFlexrayClusterParameters.gdTSSTransmitter;
    cFlexrayClusterParameters.gdWakeupTxActive = cxxFlexrayClusterParameters.gdWakeupTxActive;
    cFlexrayClusterParameters.gdWakeupTxIdle = cxxFlexrayClusterParameters.gdWakeupTxIdle;
    cFlexrayClusterParameters.gListenNoise = cxxFlexrayClusterParameters.gListenNoise;
    cFlexrayClusterParameters.gMacroPerCycle = cxxFlexrayClusterParameters.gMacroPerCycle;
    cFlexrayClusterParameters.gMaxWithoutClockCorrectionFatal =
        cxxFlexrayClusterParameters.gMaxWithoutClockCorrectionFatal;
    cFlexrayClusterParameters.gMaxWithoutClockCorrectionPassive =
        cxxFlexrayClusterParameters.gMaxWithoutClockCorrectionPassive;
    cFlexrayClusterParameters.gNumberOfMiniSlots = cxxFlexrayClusterParameters.gNumberOfMiniSlots;
    cFlexrayClusterParameters.gNumberOfStaticSlots = cxxFlexrayClusterParameters.gNumberOfStaticSlots;
    cFlexrayClusterParameters.gPayloadLengthStatic = cxxFlexrayClusterParameters.gPayloadLengthStatic;
    cFlexrayClusterParameters.gSyncFrameIDCountMax = cxxFlexrayClusterParameters.gSyncFrameIDCountMax;
}

void CxxToC(const SilKit::Services::Flexray::FlexrayNodeParameters &cxxFlexrayNodeParameters,
            SilKit_FlexrayNodeParameters &cFlexrayNodeParameters)
{
    SilKit_Struct_Init(SilKit_FlexrayNodeParameters, cFlexrayNodeParameters);
    cFlexrayNodeParameters.pAllowHaltDueToClock = cxxFlexrayNodeParameters.pAllowHaltDueToClock;
    cFlexrayNodeParameters.pAllowPassiveToActive = cxxFlexrayNodeParameters.pAllowPassiveToActive;
    cFlexrayNodeParameters.pChannels = static_cast<SilKit_FlexrayChannel>(cxxFlexrayNodeParameters.pChannels);
    cFlexrayNodeParameters.pClusterDriftDamping = cxxFlexrayNodeParameters.pClusterDriftDamping;
    cFlexrayNodeParameters.pdAcceptedStartupRange = cxxFlexrayNodeParameters.pdAcceptedStartupRange;
    cFlexrayNodeParameters.pdListenTimeout = cxxFlexrayNodeParameters.pdListenTimeout;
    cFlexrayNodeParameters.pKeySlotId = cxxFlexrayNodeParameters.pKeySlotId;
    cFlexrayNodeParameters.pKeySlotOnlyEnabled = cxxFlexrayNodeParameters.pKeySlotOnlyEnabled;
    cFlexrayNodeParameters.pKeySlotUsedForStartup = cxxFlexrayNodeParameters.pKeySlotUsedForStartup;
    cFlexrayNodeParameters.pKeySlotUsedForSync = cxxFlexrayNodeParameters.pKeySlotUsedForSync;
    cFlexrayNodeParameters.pLatestTx = cxxFlexrayNodeParameters.pLatestTx;
    cFlexrayNodeParameters.pMacroInitialOffsetA = cxxFlexrayNodeParameters.pMacroInitialOffsetA;
    cFlexrayNodeParameters.pMacroInitialOffsetB = cxxFlexrayNodeParameters.pMacroInitialOffsetB;
    cFlexrayNodeParameters.pMicroInitialOffsetA = cxxFlexrayNodeParameters.pMicroInitialOffsetA;
    cFlexrayNodeParameters.pMicroInitialOffsetB = cxxFlexrayNodeParameters.pMicroInitialOffsetB;
    cFlexrayNodeParameters.pMicroPerCycle = cxxFlexrayNodeParameters.pMicroPerCycle;
    cFlexrayNodeParameters.pOffsetCorrectionOut = cxxFlexrayNodeParameters.pOffsetCorrectionOut;
    cFlexrayNodeParameters.pOffsetCorrectionStart = cxxFlexrayNodeParameters.pOffsetCorrectionStart;
    cFlexrayNodeParameters.pRateCorrectionOut = cxxFlexrayNodeParameters.pRateCorrectionOut;
    cFlexrayNodeParameters.pWakeupChannel = static_cast<SilKit_FlexrayChannel>(cxxFlexrayNodeParameters.pWakeupChannel);
    cFlexrayNodeParameters.pWakeupPattern = cxxFlexrayNodeParameters.pWakeupPattern;
    cFlexrayNodeParameters.pdMicrotick = static_cast<SilKit_FlexrayClockPeriod>(cxxFlexrayNodeParameters.pdMicrotick);
    cFlexrayNodeParameters.pSamplesPerMicrotick = cxxFlexrayNodeParameters.pSamplesPerMicrotick;
}

void CxxToC(const SilKit::Services::Flexray::FlexrayTxBufferConfig &cxxFlexrayTxBufferConfig,
            SilKit_FlexrayTxBufferConfig &cFlexrayTxBufferConfig)
{
    SilKit_Struct_Init(SilKit_FlexrayTxBufferConfig, cFlexrayTxBufferConfig);
    cFlexrayTxBufferConfig.channels = static_cast<SilKit_FlexrayChannel>(cxxFlexrayTxBufferConfig.channels);
    cFlexrayTxBufferConfig.slotId = cxxFlexrayTxBufferConfig.slotId;
    cFlexrayTxBufferConfig.offset = cxxFlexrayTxBufferConfig.offset;
    cFlexrayTxBufferConfig.repetition = cxxFlexrayTxBufferConfig.repetition;
    cFlexrayTxBufferConfig.hasPayloadPreambleIndicator = cxxFlexrayTxBufferConfig.hasPayloadPreambleIndicator;
    cFlexrayTxBufferConfig.headerCrc = cxxFlexrayTxBufferConfig.headerCrc;
    cFlexrayTxBufferConfig.transmissionMode =
        static_cast<SilKit_FlexrayTransmissionMode>(cxxFlexrayTxBufferConfig.transmissionMode);
}

void CxxToC(const SilKit::Services::Flexray::FlexrayTxBufferUpdate &cxxFlexrayTxBufferUpdate,
            SilKit_FlexrayTxBufferUpdate &cFlexrayTxBufferUpdate)
{
    SilKit_Struct_Init(SilKit_FlexrayTxBufferUpdate, cFlexrayTxBufferUpdate);
    cFlexrayTxBufferUpdate.txBufferIndex = cxxFlexrayTxBufferUpdate.txBufferIndex;
    cFlexrayTxBufferUpdate.payloadDataValid = cxxFlexrayTxBufferUpdate.payloadDataValid;
    cFlexrayTxBufferUpdate.payload = ToSilKitByteVector(cxxFlexrayTxBufferUpdate.payload);
}

void CToCxx(const SilKit_FlexrayHeader &cFlexrayHeader, SilKit::Services::Flexray::FlexrayHeader &cxxFlexrayHeader)
{
    cxxFlexrayHeader.cycleCount = cFlexrayHeader.cycleCount;
    cxxFlexrayHeader.flags = cFlexrayHeader.flags;
    cxxFlexrayHeader.frameId = cFlexrayHeader.frameId;
    cxxFlexrayHeader.headerCrc = cFlexrayHeader.headerCrc;
    cxxFlexrayHeader.payloadLength = cFlexrayHeader.payloadLength;
}

} // namespace

} // namespace Flexray
} // namespace Services
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
