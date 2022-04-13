// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbReceiver.hpp"
#include "IIbSender.hpp"

#include "ib/sim/data/fwd_decl.hpp"

namespace ib {
namespace sim {
namespace data {

//! \brief IIbToDataSubscriber interface used by the Participant
class IIbToDataPublisher
    : public mw::IIbReceiver<>
    , public mw::IIbSender<DataMessage>
{
};

} // namespace data
} // namespace sim
} // namespace ib

