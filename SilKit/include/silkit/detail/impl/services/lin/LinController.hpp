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

#include <memory>
#include <unordered_map>

#include "silkit/capi/Lin.h"

#include "silkit/services/lin/ILinController.hpp"
#include "silkit/experimental/services/lin/LinDatatypesExtensions.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Services {
namespace Lin {

class LinController : public SilKit::Services::Lin::ILinController
{
public:
    inline LinController(SilKit_Participant *participant, const std::string &canonicalName,
                         const std::string &networkName);

    inline ~LinController() override = default;

    inline void Init(SilKit::Services::Lin::LinControllerConfig config) override;

    inline auto Status() const noexcept -> SilKit::Services::Lin::LinControllerStatus override;

    inline void SendFrame(SilKit::Services::Lin::LinFrame frame,
                          SilKit::Services::Lin::LinFrameResponseType responseType) override;

    inline void SendFrameHeader(SilKit::Services::Lin::LinId linId) override;

    inline void UpdateTxBuffer(SilKit::Services::Lin::LinFrame frame) override;

    inline void SetFrameResponse(SilKit::Services::Lin::LinFrameResponse response) override;

    inline void GoToSleep() override;

    inline void GoToSleepInternal() override;

    inline void Wakeup() override;

    inline void WakeupInternal() override;

    inline auto AddFrameStatusHandler(FrameStatusHandler handler) -> Util::HandlerId override;

    inline void RemoveFrameStatusHandler(Util::HandlerId handlerId) override;

    inline auto AddGoToSleepHandler(GoToSleepHandler handler) -> Util::HandlerId override;

    inline void RemoveGoToSleepHandler(Util::HandlerId handlerId) override;

    inline auto AddWakeupHandler(WakeupHandler handler) -> Util::HandlerId override;

    inline void RemoveWakeupHandler(Util::HandlerId handlerId) override;

public:
    inline auto ExperimentalAddLinSlaveConfigurationHandler(
        SilKit::Experimental::Services::Lin::LinSlaveConfigurationHandler handler) -> SilKit::Util::HandlerId;

    inline void ExperimentalRemoveLinSlaveConfigurationHandler(SilKit::Util::HandlerId handlerId);

    inline auto ExperimentalGetSlaveConfiguration() -> SilKit::Experimental::Services::Lin::LinSlaveConfiguration;

    inline void ExperimentalInitDynamic(const SilKit::Experimental::Services::Lin::LinControllerDynamicConfig& dynamicConfig);

    inline auto ExperimentalAddFrameHeaderHandler(SilKit::Experimental::Services::Lin::LinFrameHeaderHandler handler) -> Util::HandlerId;

    inline void ExperimentalRemoveFrameHeaderHandler(Util::HandlerId handlerId);

    inline void ExperimentalSendDynamicResponse(const SilKit::Services::Lin::LinFrame& linFrame);

private:
    template <typename HandlerFunction>
    struct HandlerData
    {
        SilKit::Services::Lin::ILinController *controller{nullptr};
        HandlerFunction handler{};
    };

    template <typename HandlerFunction>
    using HandlerDataMap = std::unordered_map<SilKit::Util::HandlerId, std::unique_ptr<HandlerData<HandlerFunction>>>;

private:
    SilKit_LinController *_linController{nullptr};

    HandlerDataMap<FrameStatusHandler> _frameStatusHandlers;
    HandlerDataMap<GoToSleepHandler> _goToSleepHandlers;
    HandlerDataMap<WakeupHandler> _wakeupHandlers;
    HandlerDataMap<SilKit::Experimental::Services::Lin::LinFrameHeaderHandler> _frameHeaderHandlers;
    HandlerDataMap<SilKit::Experimental::Services::Lin::LinSlaveConfigurationHandler> _slaveConfigurationHandlers;
};

} // namespace Lin
} // namespace Services
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


// ================================================================================
//  Inline Implementations
// ================================================================================

#include <algorithm>

#include "silkit/detail/impl/ThrowOnError.hpp"

namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Services {
namespace Lin {

namespace {

inline void CxxToC(const SilKit::Services::Lin::LinFrame &cxxLinFrame, SilKit_LinFrame &cLinFrame);
inline void CxxToC(const SilKit::Services::Lin::LinFrameResponse &cxxLinFrameResponse,
                   SilKit_LinFrameResponse &cLinFrameResponse, SilKit_LinFrame &cLinFrame);

} // namespace

LinController::LinController(SilKit_Participant *participant, const std::string &canonicalName,
                             const std::string &networkName)
{
    const auto returnCode =
        SilKit_LinController_Create(&_linController, participant, canonicalName.c_str(), networkName.c_str());
    ThrowOnError(returnCode);
}

void LinController::Init(SilKit::Services::Lin::LinControllerConfig config)
{
    std::vector<SilKit_LinFrame> frames;
    frames.reserve(config.frameResponses.size());

    std::vector<SilKit_LinFrameResponse> frameResponses;
    std::transform(config.frameResponses.begin(), config.frameResponses.end(), std::back_inserter(frameResponses),
                   [&frames](const SilKit::Services::Lin::LinFrameResponse &frameResponse) -> SilKit_LinFrameResponse {
                       frames.emplace_back();

                       SilKit_LinFrame &cFrame = frames.back();
                       SilKit_Struct_Init(SilKit_LinFrame, cFrame);
                       cFrame.id = frameResponse.frame.id;
                       cFrame.checksumModel = static_cast<SilKit_LinChecksumModel>(frameResponse.frame.checksumModel);
                       cFrame.dataLength = frameResponse.frame.dataLength;
                       std::copy_n(frameResponse.frame.data.data(), frameResponse.frame.data.size(), cFrame.data);

                       SilKit_LinFrameResponse cFrameResponse;
                       SilKit_Struct_Init(SilKit_LinFrameResponse, cFrameResponse);
                       cFrameResponse.frame = &cFrame;
                       cFrameResponse.responseMode =
                           static_cast<SilKit_LinFrameResponseMode>(frameResponse.responseMode);
                       return cFrameResponse;
                   });

    SilKit_LinControllerConfig linControllerConfig;
    SilKit_Struct_Init(SilKit_LinControllerConfig, linControllerConfig);
    linControllerConfig.controllerMode = static_cast<SilKit_LinControllerMode>(config.controllerMode);
    linControllerConfig.baudRate = config.baudRate;
    linControllerConfig.numFrameResponses = frameResponses.size();
    linControllerConfig.frameResponses = frameResponses.data();

    const auto returnCode = SilKit_LinController_Init(_linController, &linControllerConfig);
    ThrowOnError(returnCode);
}

auto LinController::Status() const noexcept -> SilKit::Services::Lin::LinControllerStatus
{
    SilKit_LinControllerStatus status{SilKit_LinControllerStatus_Unknown};

    const auto returnCode = SilKit_LinController_Status(_linController, &status);
    ThrowOnError(returnCode); // will call std::terminate on exception (! because noexcept !)

    return static_cast<SilKit::Services::Lin::LinControllerStatus>(status);
}

void LinController::SendFrame(SilKit::Services::Lin::LinFrame frame,
                              SilKit::Services::Lin::LinFrameResponseType responseType)
{
    SilKit_LinFrame linFrame;
    CxxToC(frame, linFrame);

    const auto returnCode = SilKit_LinController_SendFrame(_linController, &linFrame,
                                                           static_cast<SilKit_LinFrameResponseType>(responseType));
    ThrowOnError(returnCode);
}

void LinController::SendFrameHeader(SilKit::Services::Lin::LinId linId)
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST
    const auto returnCode = SilKit_LinController_SendFrameHeader(_linController, static_cast<SilKit_LinId>(linId));
    ThrowOnError(returnCode);
}

void LinController::UpdateTxBuffer(SilKit::Services::Lin::LinFrame frame)
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    SilKit_LinFrame linFrame;
    CxxToC(frame, linFrame);

    const auto returnCode = SilKit_LinController_UpdateTxBuffer(_linController, &linFrame);
    ThrowOnError(returnCode);
}

void LinController::SetFrameResponse(SilKit::Services::Lin::LinFrameResponse response)
{
    // TODO: SILKIT_HOURGLASS_NOT_UNDER_TEST

    SilKit_LinFrame linFrame;
    CxxToC(response.frame, linFrame);

    SilKit_LinFrameResponse linFrameResponse;
    SilKit_Struct_Init(SilKit_LinFrameResponse, linFrameResponse);
    linFrameResponse.frame = &linFrame;
    linFrameResponse.responseMode = static_cast<SilKit_LinFrameResponseMode>(response.responseMode);

    const auto returnCode = SilKit_LinController_SetFrameResponse(_linController, &linFrameResponse);
    ThrowOnError(returnCode);
}

void LinController::GoToSleep()
{
    const auto returnCode = SilKit_LinController_GoToSleep(_linController);
    ThrowOnError(returnCode);
}

void LinController::GoToSleepInternal()
{
    const auto returnCode = SilKit_LinController_GoToSleepInternal(_linController);
    ThrowOnError(returnCode);
}

void LinController::Wakeup()
{
    const auto returnCode = SilKit_LinController_Wakeup(_linController);
    ThrowOnError(returnCode);
}

void LinController::WakeupInternal()
{
    const auto returnCode = SilKit_LinController_WakeupInternal(_linController);
    ThrowOnError(returnCode);
}

auto LinController::AddFrameStatusHandler(FrameStatusHandler handler) -> Util::HandlerId
{
    const auto cHandler = [](void *context, SilKit_LinController *controller,
                             const SilKit_LinFrameStatusEvent *frameStatusEvent) {
        SILKIT_UNUSED_ARG(controller);

        SilKit::Services::Lin::LinFrameStatusEvent event{};
        event.timestamp = std::chrono::nanoseconds{frameStatusEvent->timestamp};
        event.frame.id = frameStatusEvent->frame->id;
        event.frame.checksumModel =
            static_cast<SilKit::Services::Lin::LinChecksumModel>(frameStatusEvent->frame->checksumModel);
        event.frame.dataLength = frameStatusEvent->frame->dataLength;
        std::copy_n(frameStatusEvent->frame->data, event.frame.data.size(), event.frame.data.data());
        event.status = static_cast<SilKit::Services::Lin::LinFrameStatus>(frameStatusEvent->status);

        const auto data = static_cast<HandlerData<FrameStatusHandler> *>(context);
        data->handler(data->controller, event);
    };

    SilKit_HandlerId handlerId;

    auto handlerData = std::make_unique<HandlerData<FrameStatusHandler>>();
    handlerData->controller = this;
    handlerData->handler = std::move(handler);

    const auto returnCode =
        SilKit_LinController_AddFrameStatusHandler(_linController, handlerData.get(), cHandler, &handlerId);
    ThrowOnError(returnCode);

    _frameStatusHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

    return static_cast<SilKit::Services::HandlerId>(handlerId);
}

void LinController::RemoveFrameStatusHandler(Util::HandlerId handlerId)
{
    const auto returnCode =
        SilKit_LinController_RemoveFrameStatusHandler(_linController, static_cast<SilKit_HandlerId>(handlerId));
    ThrowOnError(returnCode);

    _frameStatusHandlers.erase(handlerId);
}

auto LinController::AddGoToSleepHandler(GoToSleepHandler handler) -> Util::HandlerId
{
    const auto cHandler = [](void *context, SilKit_LinController *controller,
                             const SilKit_LinGoToSleepEvent *goToSleepEvent) {
        SILKIT_UNUSED_ARG(controller);

        SilKit::Services::Lin::LinGoToSleepEvent event{};
        event.timestamp = std::chrono::nanoseconds{goToSleepEvent->timestamp};

        const auto data = static_cast<HandlerData<GoToSleepHandler> *>(context);
        data->handler(data->controller, event);
    };

    SilKit_HandlerId handlerId;

    auto handlerData = std::make_unique<HandlerData<GoToSleepHandler>>();
    handlerData->controller = this;
    handlerData->handler = std::move(handler);

    const auto returnCode =
        SilKit_LinController_AddGoToSleepHandler(_linController, handlerData.get(), cHandler, &handlerId);
    ThrowOnError(returnCode);

    _goToSleepHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

    return static_cast<SilKit::Services::HandlerId>(handlerId);
}

void LinController::RemoveGoToSleepHandler(Util::HandlerId handlerId)
{
    const auto returnCode =
        SilKit_LinController_RemoveGoToSleepHandler(_linController, static_cast<SilKit_HandlerId>(handlerId));
    ThrowOnError(returnCode);

    _goToSleepHandlers.erase(handlerId);
}

auto LinController::AddWakeupHandler(WakeupHandler handler) -> Util::HandlerId
{
    const auto cHandler = [](void *context, SilKit_LinController *controller,
                             const SilKit_LinWakeupEvent *wakeupEvent) {
        SILKIT_UNUSED_ARG(controller);

        SilKit::Services::Lin::LinWakeupEvent event{};
        event.timestamp = std::chrono::nanoseconds{wakeupEvent->timestamp};
        event.direction = static_cast<SilKit::Services::TransmitDirection>(wakeupEvent->direction);

        const auto data = static_cast<HandlerData<WakeupHandler> *>(context);
        data->handler(data->controller, event);
    };

    SilKit_HandlerId handlerId;

    auto handlerData = std::make_unique<HandlerData<WakeupHandler>>();
    handlerData->controller = this;
    handlerData->handler = std::move(handler);

    const auto returnCode =
        SilKit_LinController_AddWakeupHandler(_linController, handlerData.get(), cHandler, &handlerId);
    ThrowOnError(returnCode);

    _wakeupHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

    return static_cast<SilKit::Services::HandlerId>(handlerId);
}

void LinController::RemoveWakeupHandler(Util::HandlerId handlerId)
{
    const auto returnCode =
        SilKit_LinController_RemoveWakeupHandler(_linController, static_cast<SilKit_HandlerId>(handlerId));
    ThrowOnError(returnCode);

    _wakeupHandlers.erase(handlerId);
}

auto LinController::ExperimentalAddLinSlaveConfigurationHandler(
    SilKit::Experimental::Services::Lin::LinSlaveConfigurationHandler handler) -> SilKit::Util::HandlerId
{
    using SilKit::Experimental::Services::Lin::LinSlaveConfigurationHandler;

    const auto cHandler = [](void *context, SilKit_LinController *controller,
                             const SilKit_Experimental_LinSlaveConfigurationEvent *slaveConfigurationEvent) {
        SILKIT_UNUSED_ARG(controller);

        SilKit::Experimental::Services::Lin::LinSlaveConfigurationEvent event{};
        event.timestamp = std::chrono::nanoseconds{slaveConfigurationEvent->timestamp};

        const auto data = static_cast<HandlerData<LinSlaveConfigurationHandler> *>(context);
        data->handler(data->controller, event);
    };

    SilKit_HandlerId handlerId;

    auto handlerData = std::make_unique<HandlerData<LinSlaveConfigurationHandler>>();
    handlerData->controller = this;
    handlerData->handler = std::move(handler);

    const auto returnCode = SilKit_Experimental_LinController_AddLinSlaveConfigurationHandler(
        _linController, handlerData.get(), cHandler, &handlerId);
    ThrowOnError(returnCode);

    _slaveConfigurationHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

    return static_cast<SilKit::Services::HandlerId>(handlerId);
}

void LinController::ExperimentalRemoveLinSlaveConfigurationHandler(SilKit::Util::HandlerId handlerId)
{
    const auto returnCode =
        SilKit_Experimental_LinController_RemoveLinSlaveConfigurationHandler(_linController, static_cast<SilKit_HandlerId>(handlerId));
    ThrowOnError(returnCode);

    _wakeupHandlers.erase(handlerId);
}

auto LinController::ExperimentalGetSlaveConfiguration() -> SilKit::Experimental::Services::Lin::LinSlaveConfiguration
{
    SilKit_Experimental_LinSlaveConfiguration cSlaveConfiguration;
    SilKit_Struct_Init(SilKit_Experimental_LinSlaveConfiguration, cSlaveConfiguration);

    const auto returnCode = SilKit_Experimental_LinController_GetSlaveConfiguration(_linController, &cSlaveConfiguration);
    ThrowOnError(returnCode);

    constexpr size_t count = sizeof(cSlaveConfiguration.isLinIdResponding) / sizeof(cSlaveConfiguration.isLinIdResponding[0]);

    SilKit::Experimental::Services::Lin::LinSlaveConfiguration cppSlaveConfiguration{};
    for (size_t index = 0; index != count; ++index)
    {
        if (cSlaveConfiguration.isLinIdResponding[index])
        {
            cppSlaveConfiguration.respondingLinIds.emplace_back(static_cast<SilKit::Services::Lin::LinId>(index));
        }
    }

    return cppSlaveConfiguration;
}

void LinController::ExperimentalInitDynamic(const SilKit::Experimental::Services::Lin::LinControllerDynamicConfig& dynamicConfig)
{
    SilKit_Experimental_LinControllerDynamicConfig cConfig;
    SilKit_Struct_Init(SilKit_Experimental_LinControllerDynamicConfig, cConfig);
    cConfig.controllerMode = static_cast<SilKit_LinControllerMode>(dynamicConfig.controllerMode);
    cConfig.baudRate = dynamicConfig.baudRate;

    const auto returnCode = SilKit_Experimental_LinController_InitDynamic(_linController, &cConfig);
    ThrowOnError(returnCode);
}

auto LinController::ExperimentalAddFrameHeaderHandler(SilKit::Experimental::Services::Lin::LinFrameHeaderHandler handler) -> Util::HandlerId
{
    const auto cHandler = [](void *context, SilKit_LinController *controller,
                             const SilKit_Experimental_LinFrameHeaderEvent *headerEvent) {
        SILKIT_UNUSED_ARG(controller);

        SilKit::Experimental::Services::Lin::LinFrameHeaderEvent event{};
        event.timestamp = std::chrono::nanoseconds{headerEvent->timestamp};
        event.id = static_cast<SilKit::Services::Lin::LinId>(headerEvent->id);

        const auto data = static_cast<HandlerData<SilKit::Experimental::Services::Lin::LinFrameHeaderHandler> *>(context);
        data->handler(data->controller, event);
    };

    SilKit_HandlerId handlerId;

    auto handlerData = std::make_unique<HandlerData<SilKit::Experimental::Services::Lin::LinFrameHeaderHandler>>();
    handlerData->controller = this;
    handlerData->handler = std::move(handler);

    const auto returnCode =
        SilKit_Experimental_LinController_AddFrameHeaderHandler(_linController, handlerData.get(), cHandler, &handlerId);
    ThrowOnError(returnCode);

    _frameHeaderHandlers.emplace(static_cast<SilKit::Util::HandlerId>(handlerId), std::move(handlerData));

    return static_cast<SilKit::Services::HandlerId>(handlerId);
}

void LinController::ExperimentalRemoveFrameHeaderHandler(Util::HandlerId handlerId)
{
    const auto returnCode =
        SilKit_Experimental_LinController_RemoveFrameHeaderHandler(_linController, static_cast<SilKit_HandlerId>(handlerId));
    ThrowOnError(returnCode);

    _frameHeaderHandlers.erase(handlerId);
}

void LinController::ExperimentalSendDynamicResponse(const SilKit::Services::Lin::LinFrame& linFrame)
{
    SilKit_LinFrame cLinFrame;
    CxxToC(linFrame, cLinFrame);

    const auto returnCode = SilKit_Experimental_LinController_SendDynamicResponse(_linController, &cLinFrame);
    ThrowOnError(returnCode);
}

namespace {

void CxxToC(const SilKit::Services::Lin::LinFrame &cxxLinFrame, SilKit_LinFrame &cLinFrame)
{
    SilKit_Struct_Init(SilKit_LinFrame, cLinFrame);
    cLinFrame.id = cxxLinFrame.id;
    cLinFrame.checksumModel = static_cast<SilKit_LinChecksumModel>(cxxLinFrame.checksumModel);
    cLinFrame.dataLength = cxxLinFrame.dataLength;
    std::copy_n(cxxLinFrame.data.data(), cxxLinFrame.data.size(), cLinFrame.data);

    static_assert(sizeof(cxxLinFrame.data) == sizeof(cLinFrame.data),
                  "SilKit_LinFrame::data has different size than SilKit::Services::Lin::LinFrame::data");
}

void CxxToC(const SilKit::Services::Lin::LinFrameResponse &cxxLinFrameResponse,
            SilKit_LinFrameResponse &cLinFrameResponse, SilKit_LinFrame &cLinFrame)
{
    CxxToC(cxxLinFrameResponse.frame, cLinFrame);

    SilKit_Struct_Init(SilKit_LinFrameResponse, cLinFrameResponse);
    cLinFrameResponse.frame = &cLinFrame;
    cLinFrameResponse.responseMode = static_cast<SilKit_LinFrameResponseMode>(cxxLinFrameResponse.responseMode);
}

} // namespace

} // namespace Lin
} // namespace Services
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
