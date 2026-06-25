// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "core/internal/IReceiver.hpp"
#include "core/internal/ISender.hpp"
#include "wire/rpc/WireRpcMessages.hpp"

#include "silkit/services/rpc/fwd_decl.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

//! \brief IMsgForRpcClient interface used by the Participant
class IMsgForRpcClient
    : public Core::IReceiver<FunctionCallResponse>
    , public Core::ISender<FunctionCall>
{
public:
    virtual ~IMsgForRpcClient() noexcept = default;
};

} // namespace Rpc
} // namespace Services
} // namespace SilKit
