// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

// ================================================================================
//  ATTENTION: This header must NOT include any SIL Kit header (neither internal,
//             nor public), as it is used to implement the 'legacy' ABI functions.
// ================================================================================

#include <memory>
#include <string>


// Forward Declarations

namespace SilKit {
namespace Config {
class IParticipantConfiguration;
} // namespace Config
} // namespace SilKit

namespace SilKit {
namespace Vendor {
namespace Vector {
class ISilKitRegistry;
} // namespace Vector
} // namespace Vendor
} // namespace SilKit


// Function Declarations

namespace SilKit {
namespace Vendor {
namespace Vector {

auto CreateSilKitRegistryImpl(std::shared_ptr<SilKit::Config::IParticipantConfiguration> config)
    -> std::unique_ptr<ISilKitRegistry>;

} // namespace Vector
} // namespace Vendor
} // namespace SilKit
