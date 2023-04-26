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
