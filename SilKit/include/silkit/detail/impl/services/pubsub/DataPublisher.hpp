// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include "silkit/capi/DataPubSub.h"

#include "silkit/services/pubsub/IDataPublisher.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Services {
namespace PubSub {

class DataPublisher : public SilKit::Services::PubSub::IDataPublisher
{
public:
    inline DataPublisher(SilKit_Participant* participant, const std::string& canonicalName,
                         const SilKit::Services::PubSub::PubSubSpec& dataSpec, uint8_t history);

    inline ~DataPublisher() override = default;

    inline void Publish(Util::Span<const uint8_t> data) override;

private:
    SilKit_DataPublisher* _dataPublisher{nullptr};
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

DataPublisher::DataPublisher(SilKit_Participant* participant, const std::string& canonicalName,
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

void DataPublisher::Publish(Util::Span<const uint8_t> data)
{
    const auto byteVector = ToSilKitByteVector(data);
    const auto returnCode = SilKit_DataPublisher_Publish(_dataPublisher, &byteVector);
    ThrowOnError(returnCode);
}

} // namespace PubSub
} // namespace Services
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
