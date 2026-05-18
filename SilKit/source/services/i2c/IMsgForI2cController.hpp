// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/services/i2c/I2cDatatypes.hpp"

#include "IReceiver.hpp"
#include "ISender.hpp"
#include "WireI2cMessages.hpp"

namespace SilKit {
namespace Services {
namespace I2c {

/*! \brief IMsgForI2cController interface
 *
 *  Used by the Participant, implemented by the I2cController
 */
class IMsgForI2cController
    : public Core::IReceiver<WireI2cFrameEvent, I2cAcknowledge, WireI2cControllerConfig>
    , public Core::ISender<WireI2cFrameEvent, I2cAcknowledge, WireI2cControllerConfig>
{
};

} // namespace I2c
} // namespace Services
} // namespace SilKit
