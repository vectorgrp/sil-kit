// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbEndpoint.hpp"
#include "IIbSender.hpp"

#include "ib/sim/data/DataMessageDatatypes.hpp"

namespace ib {
namespace sim {
namespace data {

//! \brief IIbToDataPubSubHandshake interface used by the ComAdapter
class IIbToDataSubscriber
    : public mw::IIbEndpoint<PublisherAnnouncement>
    , public mw::IIbSender<>
{
};

} // namespace data
} // namespace sim
} // namespace ib

