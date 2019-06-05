// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "SyncDatatypes.hpp"
#include "ib/mw/IIbEndpoint.hpp"
#include "ib/mw/IIbSender.hpp"

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
