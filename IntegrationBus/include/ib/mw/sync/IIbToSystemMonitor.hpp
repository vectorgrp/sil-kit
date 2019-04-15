// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "SyncDatatypes.hpp"
#include "ib/mw/IIbEndpoint.hpp"
#include "ib/mw/IIbSender.hpp"

namespace ib {
namespace mw {
namespace sync {

class IIbToSystemMonitor
    : public mw::IIbEndpoint<ParticipantStatus>
    , public mw::IIbSender<>
{
};

} // namespace sync
} // namespace mw
} // namespace ib
