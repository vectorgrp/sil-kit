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
    : public mw::IIbReceiver<LinTransmission, LinWakeupPulse, LinControllerConfig, LinControllerStatusUpdate, LinFrameResponseUpdate>
    , public mw::IIbSender<LinTransmission, LinSendFrameRequest, LinSendFrameHeaderRequest, LinWakeupPulse, LinControllerConfig, LinControllerStatusUpdate, LinFrameResponseUpdate>
{
public:
    virtual ~IIbToLinControllerFacade() noexcept = default;
};

} // namespace lin
} // namespace sim
} // namespace ib
