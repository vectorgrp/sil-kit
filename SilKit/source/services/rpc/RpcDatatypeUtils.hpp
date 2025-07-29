// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <ostream>
#include <map>

#include "silkit/services/rpc/RpcDatatypes.hpp"
#include "silkit/services/datatypes.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

bool MatchMediaType(const std::string& clientMediaType, const std::string& serverMediaType);

bool MatchLabels(const std::vector<SilKit::Services::MatchingLabel>& subscriberLabels,
                 const std::vector<SilKit::Services::MatchingLabel>& publisherLabels);


} // namespace Rpc
} // namespace Services
} // namespace SilKit
