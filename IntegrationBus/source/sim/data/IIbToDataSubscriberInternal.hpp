// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbEndpoint.hpp"
#include "IIbSender.hpp"

#include "ib/sim/data/DataMessageDatatypes.hpp"

namespace ib {
namespace sim {
namespace data {

//! \brief IIbToDataSubscriber interface used by the ComAdapter
class IIbToDataSubscriberInternal
    : public mw::IIbEndpoint<DataMessage>
    , public mw::IIbSender<>
{
};

} // namespace data
} // namespace sim
} // namespace ib

