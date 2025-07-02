// SPDX-FileCopyrightText: 2022-2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "RpcDatatypeUtils.hpp"
#include "silkit/services/datatypes.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

bool MatchMediaType(const std::string& clientMediaType, const std::string& serverMediaType)
{
    return clientMediaType == "" || clientMediaType == serverMediaType;
}

} // namespace Rpc
} // namespace Services
} // namespace SilKit
