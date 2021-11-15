// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/sync/SyncDatatypes.hpp"
#include "IIbEndpoint.hpp"
#include "IIbSender.hpp"

namespace ib {
namespace mw {
namespace sync {

class IIbToParticipantController
    : public mw::IIbEndpoint<ParticipantCommand, SystemCommand, Tick, QuantumGrant, NextSimTask>
    , public mw::IIbSender<ParticipantStatus, TickDone, QuantumRequest, NextSimTask>
{
};

} // namespace sync
} // namespace mw
} // namespace ib
