// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IIbReceiver.hpp"
#include "IIbSender.hpp"
#include "ib/sim/lin/LinDatatypes.hpp"

namespace ib {
namespace sim {
namespace lin {

/*! \brief IIbToLinControllerFacade interface
 *
 *  Used by the Participant, implemented by the LinControllerFacade
 */
class IIbToLinControllerFacade
    : public mw::IIbReceiver<Transmission, WakeupPulse, ControllerConfig, ControllerStatusUpdate, FrameResponseUpdate>
    , public mw::IIbSender<Transmission, SendFrameRequest, SendFrameHeaderRequest, WakeupPulse, ControllerConfig, ControllerStatusUpdate, FrameResponseUpdate>
{
public:
    virtual ~IIbToLinControllerFacade() noexcept = default;
};

} // namespace lin
} // namespace sim
} // namespace ib
