// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "core/internal/MessageBuffer.hpp"
#include "core/internal/LoggingDatatypesInternal.hpp"

namespace SilKit {
namespace Services {
namespace Logging {

void Serialize(SilKit::Core::MessageBuffer& buffer, const LogMsg& msg);
void Deserialize(SilKit::Core::MessageBuffer& buffer, LogMsg& out);

} // namespace Logging
} // namespace Services
} // namespace SilKit
