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
    : public mw::IIbReceiver<LinTransmission, LinWakeupPulse, LinControllerConfig, LinControllerStatusUpdate, LinFrameResponseUpdate>
    , public mw::IIbSender<LinTransmission, LinWakeupPulse, LinControllerConfig, LinControllerStatusUpdate, LinFrameResponseUpdate>
{
};

} // namespace lin
} // namespace sim
} // namespace ib
