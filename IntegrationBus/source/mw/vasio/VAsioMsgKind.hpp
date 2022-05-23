// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include <cstdint>

namespace ib {
namespace mw {

enum class VAsioMsgKind: uint8_t
{
    Invalid = 0,
    SubscriptionAnnouncement = 1,
    SubscriptionAcknowledge = 2,
    IbMwMsg = 3, //Deprecated? and nowhere used as of 3.99.22
    IbSimMsg = 4,
    IbRegistryMessage = 5,
};


} // mw
} // namespace ib
