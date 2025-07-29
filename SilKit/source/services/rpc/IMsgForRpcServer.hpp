// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IReceiver.hpp"
#include "ISender.hpp"

#include "silkit/services/rpc/fwd_decl.hpp"

namespace SilKit {
namespace Services {
namespace Rpc {

//! \brief IMsgForRpcServer interface used by the Participant
class IMsgForRpcServer
    : public Core::IReceiver<>
    , public Core::ISender<>
{
public:
    virtual ~IMsgForRpcServer() noexcept = default;
};

} // namespace Rpc
} // namespace Services
} // namespace SilKit
