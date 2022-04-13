// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbReceiver.hpp"
#include "IIbSender.hpp"
#include "ib/sim/lin/LinDatatypes.hpp"

namespace ib {
namespace sim {
namespace lin {

/*! \brief IIbToLinController interface
 *
 *  Used by the Participant, implemented by the LinController
 */
class IIbToLinController
    : public mw::IIbReceiver<Transmission, WakeupPulse, ControllerConfig, ControllerStatusUpdate, FrameResponseUpdate>
    , public mw::IIbSender<Transmission, WakeupPulse, ControllerConfig, ControllerStatusUpdate, FrameResponseUpdate>
{
};

} // namespace lin
} // namespace sim
} // namespace ib
