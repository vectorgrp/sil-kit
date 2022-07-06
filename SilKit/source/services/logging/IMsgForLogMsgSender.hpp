// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "silkit/services/logging/LoggingDatatypes.hpp"

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
