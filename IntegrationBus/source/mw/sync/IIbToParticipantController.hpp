// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "SyncDatatypes.hpp"
#include "IIbEndpoint.hpp"
#include "IIbSender.hpp"

namespace ib {
namespace mw {
namespace sync {

class IIbToParticipantController
    : public mw::IIbEndpoint<ParticipantCommand, SystemCommand, NextSimTask>
    , public mw::IIbSender<ParticipantStatus, NextSimTask>
{
};

} // namespace sync
} // namespace mw
} // namespace ib
