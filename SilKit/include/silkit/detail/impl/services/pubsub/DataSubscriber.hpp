// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/capi/DataPubSub.h"
#include "silkit/capi/SilKitMacros.h"

#include "silkit/services/pubsub/IDataSubscriber.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Services {
namespace PubSub {

class DataSubscriber : public SilKit::Services::PubSub::IDataSubscriber
{
    using DataMessageHandler = SilKit::Services::PubSub::DataMessageHandler;

public:
    inline DataSubscriber(SilKit_Participant* participant, const std::string& canonicalName,
                          const SilKit::Services::PubSub::PubSubSpec& dataSpec,
                          SilKit::Services::PubSub::DataMessageHandler dataMessageHandler);

    inline ~DataSubscriber() override = default;

    inline void SetDataMessageHandler(SilKit::Services::PubSub::DataMessageHandler handler) override;

private:
    inline static void SilKitCALL TheDataMessageHandler(void* context, SilKit_DataSubscriber* subscriber,
                                                        const SilKit_DataMessageEvent* dataMessageEvent);

private:
    template <typename HandlerFunction>
    struct HandlerData
    {
        SilKit::Services::PubSub::IDataSubscriber* controller{nullptr};
        HandlerFunction handler{};
    };

private:
    SilKit_DataSubscriber* _dataSubscriber{nullptr};

    std::unique_ptr<HandlerData<DataMessageHandler>> _dataMessageHandler;
};

} // namespace PubSub
} // namespace Services
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


// ================================================================================
//  Inline Implementations
// ================================================================================

#include "silkit/detail/impl/ThrowOnError.hpp"

#include "MakePubSubSpecView.hpp"

namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Services {
namespace PubSub {

DataSubscriber::DataSubscriber(SilKit_Participant* participant, const std::string& canonicalName,
                               const SilKit::Services::PubSub::PubSubSpec& dataSpec,
                               SilKit::Services::PubSub::DataMessageHandler dataMessageHandler)
    : _dataMessageHandler{std::make_unique<HandlerData<DataMessageHandler>>()}
{
    _dataMessageHandler->controller = this;
    _dataMessageHandler->handler = std::move(dataMessageHandler);

    auto labels = MakePubSubSpecView(dataSpec);

    SilKit_DataSpec cDataSpec;
    SilKit_Struct_Init(SilKit_DataSpec, cDataSpec);
    cDataSpec.topic = dataSpec.Topic().c_str();
    cDataSpec.mediaType = dataSpec.MediaType().c_str();
    cDataSpec.labelList.numLabels = labels.size();
    cDataSpec.labelList.labels = labels.data();

    const auto returnCode = SilKit_DataSubscriber_Create(&_dataSubscriber, participant, canonicalName.c_str(),
                                                         &cDataSpec, _dataMessageHandler.get(), &TheDataMessageHandler);
    ThrowOnError(returnCode);
}

void DataSubscriber::SetDataMessageHandler(SilKit::Services::PubSub::DataMessageHandler handler)
{
    auto handlerData = std::make_unique<HandlerData<DataMessageHandler>>();
    handlerData->controller = this;
    handlerData->handler = std::move(handler);

    const auto returnCode =
        SilKit_DataSubscriber_SetDataMessageHandler(_dataSubscriber, handlerData.get(), &TheDataMessageHandler);
    ThrowOnError(returnCode);

    _dataMessageHandler = std::move(handlerData);
}

void DataSubscriber::TheDataMessageHandler(void* context, SilKit_DataSubscriber* subscriber,
                                           const SilKit_DataMessageEvent* dataMessageEvent)
{
    SILKIT_UNUSED_ARG(subscriber);

    SilKit::Services::PubSub::DataMessageEvent event{};
    event.timestamp = std::chrono::nanoseconds{dataMessageEvent->timestamp};
    event.data = SilKit::Util::ToSpan(dataMessageEvent->data);

    const auto handlerData = static_cast<HandlerData<DataMessageHandler>*>(context);
    handlerData->handler(handlerData->controller, event);
}

} // namespace PubSub
} // namespace Services
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
