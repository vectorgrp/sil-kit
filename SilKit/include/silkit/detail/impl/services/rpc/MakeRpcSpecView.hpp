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
namespace Rpc {

inline auto MakeRpcSpecView(const SilKit::Services::Rpc::RpcSpec& rpcSpec) -> std::vector<SilKit_Label>;

} // namespace Rpc
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
namespace Rpc {

inline auto MakeRpcSpecView(const SilKit::Services::Rpc::RpcSpec& rpcSpec) -> std::vector<SilKit_Label>
{
    std::vector<SilKit_Label> labels;
    std::transform(rpcSpec.Labels().begin(), rpcSpec.Labels().end(), std::back_inserter(labels),
                   [](const SilKit::Services::MatchingLabel& matchingLabel) -> SilKit_Label {
        return {
            matchingLabel.key.c_str(),
            matchingLabel.value.c_str(),
            static_cast<SilKit_LabelKind>(matchingLabel.kind),
        };
    });
    return labels;
}

} // namespace Rpc
} // namespace Services
} // namespace Impl
DETAIL_SILKIT_DETAIL_VN_NAMESPACE_CLOSE
} // namespace SilKit
