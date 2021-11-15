// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbEndpoint.hpp"
#include "IIbSender.hpp"

#include "ib/sim/generic/GenericMessageDatatypes.hpp"

namespace ib {
namespace sim {
namespace generic {

//! \brief IIbToGenericPublisher interface used by the ComAdapter
class IIbToGenericPublisher
    : public mw::IIbEndpoint<>
    , public mw::IIbSender<GenericMessage>
{
};

} // namespace generic
} // namespace sim
} // namespace ib

