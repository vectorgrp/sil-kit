// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/sync/SyncDatatypes.hpp"
#include "IIbReceiver.hpp"
#include "IIbSender.hpp"

namespace ib {
namespace mw {
namespace sync {

class IIbToSystemMonitor
    : public mw::IIbReceiver<ParticipantStatus, WorkflowConfiguration>
    , public mw::IIbSender<>
{
};

} // namespace sync
} // namespace mw
} // namespace ib
