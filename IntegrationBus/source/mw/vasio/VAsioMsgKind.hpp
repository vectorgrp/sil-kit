// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include <cstdint>

namespace SilKit {
namespace Core {

enum class VAsioMsgKind: uint8_t
{
    Invalid = 0,
    SubscriptionAnnouncement = 1,
    SubscriptionAcknowledge = 2,
    SilKitMwMsg = 3, //Deprecated? and nowhere used as of 3.99.22
    SilKitSimMsg = 4,
    SilKitRegistryMessage = 5,
};

} // namespace Core
} // namespace SilKit
