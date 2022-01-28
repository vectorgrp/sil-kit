// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbEndpoint.hpp"
#include "IIbSender.hpp"

#include "ib/sim/data/fwd_decl.hpp"

namespace ib {
namespace sim {
namespace data {

//! \brief IIbToDataSubscriber interface used by the ComAdapter
class IIbToDataPublisher
    : public mw::IIbEndpoint<>
    , public mw::IIbSender<DataMessage, PublisherAnnouncement>
{
};

} // namespace data
} // namespace sim
} // namespace ib

