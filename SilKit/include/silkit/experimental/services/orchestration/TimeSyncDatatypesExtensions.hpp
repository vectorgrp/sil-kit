// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/services/orchestration/ITimeSyncService.hpp"

namespace SilKit {
namespace Experimental {
namespace Services {
namespace Orchestration {

/*! Callback type to indicate external simulation step coupling.
 *  Cf., \ref AddExternalCouplingHandler(ITimeSyncService*,TimeSyncExternalCouplingHandler);
 */
using TimeSyncExternalCouplingHandler = std::function<void()>;

} // namespace Orchestration
} // namespace Services
} // namespace Experimental
} // namespace SilKit
