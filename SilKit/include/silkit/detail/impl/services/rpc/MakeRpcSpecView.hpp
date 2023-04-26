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
