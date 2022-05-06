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
 *  Used by the Participant, implemented by the LinControllerProxy
 */
class IIbToLinControllerProxy
    : public mw::IIbReceiver<LinTransmission, LinWakeupPulse, LinControllerConfig, LinFrameResponseUpdate>
    , public mw::IIbSender<LinSendFrameRequest, LinSendFrameHeaderRequest, LinWakeupPulse, LinControllerConfig, LinControllerStatusUpdate, LinFrameResponseUpdate>
{
};

} // namespace lin
} // namespace sim
} // namespace ib
