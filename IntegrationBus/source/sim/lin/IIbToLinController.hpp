// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbEndpoint.hpp"
#include "IIbSender.hpp"
#include "ib/sim/lin/LinDatatypes.hpp"

namespace ib {
namespace sim {
namespace lin {

/*! \brief IIbToLinController interface
 *
 *  Used by the ComAdapter, implemented by the LinController
 */
class IIbToLinController
    : public mw::IIbEndpoint<Transmission, WakeupPulse, ControllerConfig, ControllerStatusUpdate, FrameResponseUpdate>
    , public mw::IIbSender<Transmission, WakeupPulse, ControllerConfig, ControllerStatusUpdate, FrameResponseUpdate>
    // : public mw::IIbEndpoint<LinMessage, WakeupRequest, ControllerConfig, SlaveConfiguration, SlaveResponse>
    // , public mw::IIbSender<LinMessage, WakeupRequest, ControllerConfig, SlaveConfiguration, SlaveResponse>
{
};

} // namespace lin
} // namespace sim
} // namespace ib
