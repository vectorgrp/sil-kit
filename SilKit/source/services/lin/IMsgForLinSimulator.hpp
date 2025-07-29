// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IReceiver.hpp"
#include "ISender.hpp"
#include "IServiceEndpoint.hpp"
#include "WireLinMessages.hpp"

#include "silkit/services/lin/LinDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Lin {

/*! \brief IMsgForLinSimulator interface
*
*  Used by the Participant, implemented by the LinSimulator
*/
class IMsgForLinSimulator
    : public Core::IReceiver<LinSendFrameRequest, LinSendFrameHeaderRequest, LinWakeupPulse, WireLinControllerConfig,
                             LinControllerStatusUpdate, LinFrameResponseUpdate>
    , public Core::ISender<LinSendFrameHeaderRequest, LinTransmission, LinWakeupPulse, WireLinControllerConfig>
{
public:
    virtual ~IMsgForLinSimulator() = default;
};

} // namespace Lin
} // namespace Services
} // namespace SilKit
