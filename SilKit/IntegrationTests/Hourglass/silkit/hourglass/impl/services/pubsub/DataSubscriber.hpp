#pragma once

#include "silkit/capi/DataPubSub.h"

#include "silkit/services/pubsub/IDataSubscriber.hpp"

#include "silkit/hourglass/impl/CheckReturnCode.hpp"

#include "MakePubSubSpecView.hpp"

namespace SilKit {
namespace Hourglass {
namespace Impl {
namespace Services {
namespace PubSub {

class DataSubscriber : public SilKit::Services::PubSub::IDataSubscriber
{
public:
    DataSubscriber(SilKit_Participant* participant, const std::string& canonicalName,
                   const SilKit::Services::PubSub::PubSubSpec& dataSpec,
                   SilKit::Services::PubSub::DataMessageHandler dataMessageHandler)
        : _dataMessageHandler{std::move(dataMessageHandler)}
    {
        auto labels = MakePubSubSpecView(dataSpec);

        SilKit_DataSpec cDataSpec;
        SilKit_Struct_Init(SilKit_DataSpec, cDataSpec);
        cDataSpec.topic = dataSpec.Topic().c_str();
        cDataSpec.mediaType = dataSpec.MediaType().c_str();
        cDataSpec.labelList.numLabels = labels.size();
        cDataSpec.labelList.labels = labels.data();

        const auto returnCode = SilKit_DataSubscriber_Create(&_dataSubscriber, participant, canonicalName.c_str(),
                                                             &cDataSpec, this, &TheDataMessageHandler);
        ThrowOnError(returnCode);
    }

    ~DataSubscriber() override = default;

    void SetDataMessageHandler(SilKit::Services::PubSub::DataMessageHandler handler) override
    {
        _dataMessageHandler = std::move(handler);

        const auto returnCode =
            SilKit_DataSubscriber_SetDataMessageHandler(_dataSubscriber, this, &TheDataMessageHandler);
        ThrowOnError(returnCode);
    }

private:
    static void TheDataMessageHandler(void* context, SilKit_DataSubscriber* subscriber,
                                      const SilKit_DataMessageEvent* dataMessageEvent)
    {
        SILKIT_UNUSED_ARG(subscriber);

        SilKit::Services::PubSub::DataMessageEvent event{};
        event.timestamp = std::chrono::nanoseconds{dataMessageEvent->timestamp};
        event.data = SilKit::Util::ToSpan(dataMessageEvent->data);

        const auto dataSubscriber = static_cast<DataSubscriber*>(context);
        dataSubscriber->_dataMessageHandler(dataSubscriber, event);
    }

private:
    SilKit_DataSubscriber* _dataSubscriber{nullptr};

    SilKit::Services::PubSub::DataMessageHandler _dataMessageHandler;
};

} // namespace PubSub
} // namespace Services
} // namespace Impl
} // namespace Hourglass
} // namespace SilKit
