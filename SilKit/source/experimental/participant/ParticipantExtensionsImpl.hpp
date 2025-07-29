// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

// ================================================================================
//  ATTENTION: This header must NOT include any SIL Kit header (neither internal,
//             nor public), as it is used to implement the 'legacy' ABI functions.
// ================================================================================


// Forward Declarations

namespace SilKit {
class IParticipant;
} // namespace SilKit

namespace SilKit {
namespace Experimental {
namespace Services {
namespace Orchestration {
class ISystemController;
} // namespace Orchestration
} // namespace Services
} // namespace Experimental
} // namespace SilKit

namespace SilKit {
namespace Experimental {
namespace NetworkSimulation {
class INetworkSimulator;
} // namespace NetworkSimulation
} // namespace Experimental
} // namespace SilKit

// Function Declarations

namespace SilKit {
namespace Experimental {
namespace Participant {

auto CreateSystemControllerImpl(IParticipant* participant)
    -> SilKit::Experimental::Services::Orchestration::ISystemController*;

auto CreateNetworkSimulatorImpl(IParticipant* participant)
    -> SilKit::Experimental::NetworkSimulation::INetworkSimulator*;

} // namespace Participant
} // namespace Experimental
} // namespace SilKit
