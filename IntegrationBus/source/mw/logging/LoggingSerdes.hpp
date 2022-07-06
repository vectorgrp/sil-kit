// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"

#include "silkit/core/logging/LoggingDatatypes.hpp"

namespace SilKit {
namespace Core {
namespace Logging {

void Serialize(SilKit::Core::MessageBuffer& buffer,const LogMsg& msg);
void Deserialize(SilKit::Core::MessageBuffer& buffer, LogMsg& out);

} // namespace Logging
} // namespace Core
} // namespace SilKit
