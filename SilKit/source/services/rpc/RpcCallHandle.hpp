// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/services/rpc/IRpcCallHandle.hpp"
#include "silkit/services/rpc/RpcDatatypes.hpp"

#include "WireRpcMessages.hpp"
#include "Uuid.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

class IRpcCallHandle
{
public:
    virtual ~IRpcCallHandle() = default;
};

class RpcCallHandle : public IRpcCallHandle
{
public:
    RpcCallHandle(Util::Uuid callUuid)
        : _callUuid{callUuid}
    {
    }

    auto GetCallUuid() const -> const Util::Uuid&
    {
        return _callUuid;
    }

private:
    Util::Uuid _callUuid{};
};

} // namespace Rpc
} // namespace Services
} // namespace SilKit
