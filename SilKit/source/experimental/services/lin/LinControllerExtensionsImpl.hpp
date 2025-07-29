// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

// ================================================================================
//  ATTENTION: This header must NOT include any SIL Kit header (neither internal,
//             nor public), as it is used to implement the 'legacy' ABI functions.
// ================================================================================

#include <functional>

#include <cstdint>

// Forward Declarations

namespace SilKit {
namespace Services {
namespace Lin {
class ILinController;
struct LinFrame;
} // namespace Lin
} // namespace Services
} // namespace SilKit

namespace SilKit {
namespace Experimental {
namespace Services {
namespace Lin {
struct LinSlaveConfigurationEvent;
struct LinSlaveConfiguration;
struct LinControllerDynamicConfig;
struct LinFrameHeaderEvent;
} // namespace Lin
} // namespace Services
} // namespace Experimental
} // namespace SilKit

namespace SilKit {
namespace Util {
enum struct HandlerId : std::uint64_t;
} // namespace Util
} // namespace SilKit


// Function Declarations

namespace SilKit {
namespace Experimental {
namespace Services {
namespace Lin {

auto AddLinSlaveConfigurationHandlerImpl(
    SilKit::Services::Lin::ILinController* linController,
    std::function<void(SilKit::Services::Lin::ILinController*, const LinSlaveConfigurationEvent& msg)> handler)
    -> SilKit::Util::HandlerId;

void RemoveLinSlaveConfigurationHandlerImpl(SilKit::Services::Lin::ILinController* linController,
                                            SilKit::Util::HandlerId handlerId);

auto GetSlaveConfigurationImpl(SilKit::Services::Lin::ILinController* linController)
    -> SilKit::Experimental::Services::Lin::LinSlaveConfiguration;

void InitDynamicImpl(SilKit::Services::Lin::ILinController* linController,
                     const SilKit::Experimental::Services::Lin::LinControllerDynamicConfig& dynamicConfig);

auto AddFrameHeaderHandlerImpl(SilKit::Services::Lin::ILinController* linController,
                               std::function<void(SilKit::Services::Lin::ILinController*,
                                                  const SilKit::Experimental::Services::Lin::LinFrameHeaderEvent& msg)>
                                   handler) -> SilKit::Util::HandlerId;

void RemoveFrameHeaderHandlerImpl(SilKit::Services::Lin::ILinController* linController,
                                  SilKit::Util::HandlerId handlerId);

void SendDynamicResponseImpl(SilKit::Services::Lin::ILinController* linController,
                             const SilKit::Services::Lin::LinFrame& linFrame);

} // namespace Lin
} // namespace Services
} // namespace Experimental
} // namespace SilKit
