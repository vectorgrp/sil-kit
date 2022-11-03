#pragma once

#include "silkit/capi/DataPubSub.h"

#include "silkit/services/pubsub/IDataPublisher.hpp"

#include "silkit/hourglass/impl/CheckReturnCode.hpp"

#include "MakePubSubSpecView.hpp"

namespace SilKit {
namespace Hourglass {
namespace Impl {
namespace Services {
namespace PubSub {

class DataPublisher : public SilKit::Services::PubSub::IDataPublisher
{
public:
    DataPublisher(SilKit_Participant* participant, const std::string& canonicalName,
                  const SilKit::Services::PubSub::PubSubSpec& dataSpec, uint8_t history)
    {
        auto labels = MakePubSubSpecView(dataSpec);

        SilKit_DataSpec cDataSpec;
        SilKit_Struct_Init(SilKit_DataSpec, cDataSpec);
        cDataSpec.topic = dataSpec.Topic().c_str();
        cDataSpec.mediaType = dataSpec.MediaType().c_str();
        cDataSpec.labelList.numLabels = labels.size();
        cDataSpec.labelList.labels = labels.data();

        const auto returnCode =
            SilKit_DataPublisher_Create(&_dataPublisher, participant, canonicalName.c_str(), &cDataSpec, history);
        ThrowOnError(returnCode);
    }

    ~DataPublisher() override = default;

    void Publish(Util::Span<const uint8_t> data) override
    {
        const auto byteVector = ToSilKitByteVector(data);
        const auto returnCode = SilKit_DataPublisher_Publish(_dataPublisher, &byteVector);
        ThrowOnError(returnCode);
    }

private:
    SilKit_DataPublisher* _dataPublisher{nullptr};
};

} // namespace PubSub
} // namespace Services
} // namespace Impl
} // namespace Hourglass
} // namespace SilKit
