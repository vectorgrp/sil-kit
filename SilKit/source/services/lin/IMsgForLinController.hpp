// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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
    : public Core::IReceiver<LinTransmission, LinWakeupPulse, WireLinControllerConfig, LinControllerStatusUpdate,
                             LinSendFrameHeaderRequest, LinFrameResponseUpdate>
    , public Core::ISender<LinTransmission, LinSendFrameRequest, LinSendFrameHeaderRequest, LinWakeupPulse,
                           WireLinControllerConfig, LinControllerStatusUpdate, LinFrameResponseUpdate>
{
};

} // namespace Lin
} // namespace Services
} // namespace SilKit
