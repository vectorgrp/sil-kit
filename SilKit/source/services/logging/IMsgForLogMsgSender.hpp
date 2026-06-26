// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "core/internal/LoggingDatatypesInternal.hpp"
#include "core/internal/IReceiver.hpp"
#include "core/internal/ISender.hpp"

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
