// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "IReceiver.hpp"
#include "ISender.hpp"

#include "silkit/services/lin/LinDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Lin {

/*! \brief IMsgForLinController interface
 *
 *  Used by the Participant, implemented by the LinControllerProxy
 */
class IMsgForLinController
    : public Core::IReceiver<LinTransmission, LinWakeupPulse, LinControllerConfig, LinControllerStatusUpdate,
                             LinFrameResponseUpdate>
    , public Core::ISender<LinTransmission, LinSendFrameRequest, LinSendFrameHeaderRequest, LinWakeupPulse,
                           LinControllerConfig, LinControllerStatusUpdate, LinFrameResponseUpdate>
{
};

} // namespace Lin
} // namespace Services
} // namespace SilKit
