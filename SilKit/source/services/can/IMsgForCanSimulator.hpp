// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IReceiver.hpp"
#include "ISender.hpp"
#include "IServiceEndpoint.hpp"
#include "WireCanMessages.hpp"

#include "silkit/services/can/CanDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Can {

/*! \brief IMsgForCanSimulator interface
 *
 *  Used by the Participant
 */
class IMsgForCanSimulator
    : public Core::IReceiver<WireCanFrameEvent, CanConfigureBaudrate, CanSetControllerMode>
    , public Core::ISender<WireCanFrameEvent, CanFrameTransmitEvent, CanControllerStatus>
{
public:
    ~IMsgForCanSimulator() = default;
};

} // namespace Can
} // namespace Services
} // namespace SilKit
