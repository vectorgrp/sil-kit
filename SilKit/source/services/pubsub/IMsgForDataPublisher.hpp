// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IReceiver.hpp"
#include "ISender.hpp"
#include "WireDataMessages.hpp"

#include "silkit/services/pubsub/fwd_decl.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

//! \brief IMsgForDataSubscriber interface used by the Participant
class IMsgForDataPublisher
    : public Core::IReceiver<>
    , public Core::ISender<WireDataMessageEvent>
{
};

} // namespace PubSub
} // namespace Services
} // namespace SilKit
