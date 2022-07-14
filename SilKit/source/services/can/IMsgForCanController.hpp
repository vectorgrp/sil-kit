// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "silkit/services/can/CanDatatypes.hpp"

#include "IReceiver.hpp"
#include "ISender.hpp"
#include "WireCanMessages.hpp"

namespace SilKit {
namespace Services {
namespace Can {

/*! \brief IMsgForCanController interface
 *
 *  Used by the Participant, implemented by the CanController
 */
class IMsgForCanController
    : public Core::IReceiver<WireCanFrameEvent, CanFrameTransmitEvent, CanControllerStatus>
    , public Core::ISender<WireCanFrameEvent, CanConfigureBaudrate, CanSetControllerMode>
{
};

} // namespace Can
} // namespace Services
} // namespace SilKit
