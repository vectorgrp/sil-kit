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

#include "silkit/capi/DataPubSub.h"

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
    inline static void TheDataMessageHandler(void* context, SilKit_DataSubscriber* subscriber,
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
