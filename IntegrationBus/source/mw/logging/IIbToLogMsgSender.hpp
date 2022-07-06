// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "silkit/core/logging/LoggingDatatypes.hpp"

#include "IReceiver.hpp"
#include "ISender.hpp"

namespace SilKit {
namespace Core {
namespace Logging {

class IMsgForLogMsgSender
    : public Core::IReceiver<>
    , public Core::ISender<LogMsg>
{
};

} // namespace Logging
} // namespace Core
} // namespace SilKit
