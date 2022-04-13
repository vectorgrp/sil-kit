// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbReceiver.hpp"
#include "IIbSender.hpp"

#include "ib/sim/data/fwd_decl.hpp"

namespace ib {
namespace sim {
namespace data {

//! \brief IIbToDataPubSubHandshake interface used by the Participant
class IIbToDataSubscriber
    : public mw::IIbReceiver<>
    , public mw::IIbSender<>
{
};

} // namespace data
} // namespace sim
} // namespace ib

