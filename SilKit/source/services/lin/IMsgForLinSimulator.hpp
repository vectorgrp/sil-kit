// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "core/internal/IReceiver.hpp"
#include "core/internal/ISender.hpp"
#include "core/internal/IServiceEndpoint.hpp"
#include "wire/lin/WireLinMessages.hpp"

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
