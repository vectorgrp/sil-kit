// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbEndpoint.hpp"
#include "IIbSender.hpp"

#include "ib/sim/generic/GenericMessageDatatypes.hpp"

namespace ib {
namespace sim {
namespace generic {

//! \brief IIbToGenericSubscriber interface used by the ComAdapter
class IIbToGenericSubscriber
    : public mw::IIbEndpoint<GenericMessage>
    , public mw::IIbSender<>
{
};

} // namespace generic
} // namespace sim
} // namespace ib

