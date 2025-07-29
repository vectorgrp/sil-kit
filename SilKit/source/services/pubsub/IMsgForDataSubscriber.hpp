// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IReceiver.hpp"
#include "ISender.hpp"

#include "silkit/services/pubsub/fwd_decl.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

//! \brief IMsgForDataPubSubHandshake interface used by the Participant
class IMsgForDataSubscriber
    : public Core::IReceiver<>
    , public Core::ISender<>
{
};

} // namespace PubSub
} // namespace Services
} // namespace SilKit
