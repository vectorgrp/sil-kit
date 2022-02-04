// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <ostream>

#include "ib/sim/rpc/RpcDatatypes.hpp"

namespace ib {
namespace sim {
namespace rpc {

bool Match(const RpcExchangeFormat& clientRxf, const RpcExchangeFormat& serverRxf);

bool MatchLabels(const std::map<std::string, std::string>& clientLabels,
                 const std::map<std::string, std::string>& serverLabels);


} // namespace rpc
} // namespace sim
} // namespace ib
