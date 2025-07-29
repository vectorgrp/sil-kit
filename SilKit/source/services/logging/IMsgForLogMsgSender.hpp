// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "LoggingDatatypesInternal.hpp"
#include "IReceiver.hpp"
#include "ISender.hpp"

namespace SilKit {
namespace Services {
namespace Logging {

class IMsgForLogMsgSender
    : public Core::IReceiver<>
    , public Core::ISender<LogMsg>
{
};

} // namespace Logging
} // namespace Services
} // namespace SilKit
