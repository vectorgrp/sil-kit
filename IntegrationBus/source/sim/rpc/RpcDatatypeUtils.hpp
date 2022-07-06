// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <ostream>
#include <map>

#include "silkit/services/rpc/RpcDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

bool MatchMediaType(const std::string& clientMediaType, const std::string& serverMediaType);

bool MatchLabels(const std::map<std::string, std::string>& clientLabels,
                 const std::map<std::string, std::string>& serverLabels);


} // namespace Rpc
} // namespace Services
} // namespace SilKit
