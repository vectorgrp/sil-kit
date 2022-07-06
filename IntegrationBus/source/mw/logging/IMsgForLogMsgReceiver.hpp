// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "silkit/core/logging/LoggingDatatypes.hpp"

#include "IReceiver.hpp"
#include "ISender.hpp"

namespace SilKit {
namespace Core {
namespace Logging {

class IMsgForLogMsgReceiver
    : public Core::IReceiver<LogMsg>
    , public Core::ISender<>
{
};

} // namespace Logging
} // namespace Core
} // namespace SilKit
