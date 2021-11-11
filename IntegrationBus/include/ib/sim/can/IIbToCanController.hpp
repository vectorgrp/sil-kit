// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/sim/can/CanDatatypes.hpp"

#include "ib/mw/IIbEndpoint.hpp"
#include "ib/mw/IIbSender.hpp"

namespace ib {
namespace sim {
namespace can {

/*! \brief IIbToCanController interface
 *
 *  Used by the ComAdapter, implemented by the CanController
 */
class IIbToCanController
    : public mw::IIbEndpoint<CanMessage>
    , public mw::IIbSender<CanMessage>
{
};

} // namespace can
} // namespace sim
} // namespace ib
