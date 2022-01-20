// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbEndpoint.hpp"
#include "IIbSender.hpp"
#include "ib/sim/lin/LinDatatypes.hpp"

namespace ib {
namespace sim {
namespace lin {

/*! \brief IIbToLinControllerFacade interface
 *
 *  Used by the ComAdapter, implemented by the LinControllerFacade
 */
class IIbToLinControllerFacade
    : public mw::IIbEndpoint<Transmission, WakeupPulse, ControllerConfig, ControllerStatusUpdate, FrameResponseUpdate>
    , public mw::IIbSender<Transmission, SendFrameRequest, SendFrameHeaderRequest, WakeupPulse, ControllerConfig, ControllerStatusUpdate, FrameResponseUpdate>
{
};

} // namespace lin
} // namespace sim
} // namespace ib
