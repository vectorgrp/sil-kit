// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

namespace ib {
namespace mw {

enum class VAsioMsgKind
{
    Invalid = 0,
    SubscriptionAnnouncement = 1,
    SubscriptionAcknowledge = 2,
    IbMwMsg = 3,
    IbSimMsg = 4,
    IbRegistryMessage = 5,
};


} // mw
} // namespace ib
