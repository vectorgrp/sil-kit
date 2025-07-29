// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>

#include "silkit/capi/DataPubSub.h"

#include "silkit/services/pubsub/PubSubSpec.hpp"


namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Services {
namespace PubSub {

inline auto MakePubSubSpecView(const SilKit::Services::PubSub::PubSubSpec& pubSubSpec) -> std::vector<SilKit_Label>;

} // namespace PubSub
} // namespace Services
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit


// ================================================================================
//  Inline Implementations
// ================================================================================

#include <algorithm>

namespace SilKit {
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_BEGIN
namespace Impl {
namespace Services {
namespace PubSub {

auto MakePubSubSpecView(const SilKit::Services::PubSub::PubSubSpec& pubSubSpec) -> std::vector<SilKit_Label>
{
    std::vector<SilKit_Label> labels;
    std::transform(pubSubSpec.Labels().begin(), pubSubSpec.Labels().end(), std::back_inserter(labels),
                   [](const SilKit::Services::MatchingLabel& matchingLabel) -> SilKit_Label {
        return {
            matchingLabel.key.c_str(),
            matchingLabel.value.c_str(),
            static_cast<SilKit_LabelKind>(matchingLabel.kind),
        };
    });
    return labels;
}

} // namespace PubSub
} // namespace Services
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
