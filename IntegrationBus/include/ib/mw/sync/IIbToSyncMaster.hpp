// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/IIbEndpoint.hpp"
#include "ib/mw/IIbSender.hpp"

#include "SyncDatatypes.hpp"

namespace ib {
namespace mw {
namespace sync {

class IIbToSyncMaster
    : public mw::IIbEndpoint<TickDone, QuantumRequest>
    , public mw::IIbSender<Tick, QuantumGrant>
{
};

} // namespace sync
} // namespace mw
} // namespace ib
