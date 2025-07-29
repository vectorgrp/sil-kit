// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <unordered_map>

#include "silkit/services/datatypes.hpp"
#include "silkit/services/pubsub/PubSubDatatypes.hpp"
#include "silkit/util/HandlerId.hpp"

#include "WireDataMessages.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

class DataSubscriberInternal;

bool operator==(const DataMessageEvent& lhs, const DataMessageEvent& rhs);

bool operator==(const WireDataMessageEvent& lhs, const WireDataMessageEvent& rhs);

bool MatchMediaType(const std::string& subMediaType, const std::string& pubMediaType);

} // namespace PubSub
} // namespace Services
} // namespace SilKit
