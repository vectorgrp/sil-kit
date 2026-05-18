// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IReceiver.hpp"
#include "ISender.hpp"
#include "IServiceEndpoint.hpp"
#include "WireI2cMessages.hpp"

#include "silkit/services/i2c/I2cDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace I2c {

/*! \brief IMsgForI2cSimulator interface
 *
 *  Used by the Participant
 */
class IMsgForI2cSimulator
    : public Core::IReceiver<WireI2cFrameEvent, WireI2cControllerConfig>
    , public Core::ISender<WireI2cFrameEvent, I2cAcknowledge>
{
public:
    ~IMsgForI2cSimulator() = default;
};

} // namespace I2c
} // namespace Services
} // namespace SilKit
