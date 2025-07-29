// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/services/logging/LoggingDatatypes.hpp"

#include "IReceiver.hpp"
#include "ISender.hpp"

namespace SilKit {
namespace Services {
namespace Logging {

class IMsgForLogMsgReceiver
    : public Core::IReceiver<LogMsg>
    , public Core::ISender<>
{
};

} // namespace Logging
} // namespace Services
} // namespace SilKit
