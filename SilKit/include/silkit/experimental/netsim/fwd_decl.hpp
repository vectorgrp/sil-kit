// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

namespace SilKit {
namespace Experimental {
namespace NetworkSimulation {

class INetworkSimulator;
class ISimulatedNetwork;
class ISimulatedController;
namespace Can {
class ISimulatedCanController;
}
namespace Flexray {
class ISimulatedFlexRayController;
}
namespace Ethernet {
class ISimulatedEthernetController;
}
namespace Lin {
class ISimulatedLinController;
}

} // namespace NetworkSimulation
} // namespace Experimental
} // namespace SilKit
