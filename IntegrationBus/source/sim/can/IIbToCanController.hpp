// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "silkit/services/can/CanDatatypes.hpp"

#include "IReceiver.hpp"
#include "ISender.hpp"

namespace SilKit {
namespace Services {
namespace Can {

/*! \brief IMsgForCanController interface
 *
 *  Used by the Participant, implemented by the CanController
 */
class IMsgForCanController
    : public Core::IReceiver<CanFrameEvent, CanFrameTransmitEvent, CanControllerStatus>
    , public Core::ISender<CanFrameEvent, CanConfigureBaudrate, CanSetControllerMode>
{
};

} // namespace Can
} // namespace Services
} // namespace SilKit
