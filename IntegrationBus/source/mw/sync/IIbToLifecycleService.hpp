// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "SyncDatatypes.hpp"
#include "IIbReceiver.hpp"
#include "IIbSender.hpp"

namespace ib {
namespace mw {
namespace sync {

class IIbToLifecycleService
    : public mw::IIbReceiver<ParticipantCommand, SystemCommand>
    , public mw::IIbSender<ParticipantStatus>
{
};

} // namespace sync
} // namespace mw
} // namespace ib
