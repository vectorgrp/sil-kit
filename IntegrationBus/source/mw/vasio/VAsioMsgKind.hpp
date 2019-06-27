// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

namespace ib {
namespace mw {

enum class VAsioMsgKind
{
    Invalid = 0,
    AnnounceSubscription = 1,
    IbMwMsg = 2,
    IbSimMsg = 3,
    IbRegistryMessage = 4,
};


} // mw
} // namespace ib
