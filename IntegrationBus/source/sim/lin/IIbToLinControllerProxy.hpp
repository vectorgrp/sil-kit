// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbReceiver.hpp"
#include "IIbSender.hpp"
#include "ib/sim/lin/LinDatatypes.hpp"

namespace ib {
namespace sim {
namespace lin {

/*! \brief IIbToLinControllerProxy interface
 *
 *  Used by the ComAdapter, implemented by the LinControllerProxy
 */
class IIbToLinControllerProxy
    : public mw::IIbReceiver<Transmission, WakeupPulse, ControllerConfig, FrameResponseUpdate>
    , public mw::IIbSender<SendFrameRequest, SendFrameHeaderRequest, WakeupPulse, ControllerConfig, ControllerStatusUpdate, FrameResponseUpdate>
{
};

} // namespace lin
} // namespace sim
} // namespace ib
