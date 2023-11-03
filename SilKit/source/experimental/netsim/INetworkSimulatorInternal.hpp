// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/experimental/netsim/NetworkSimulatorDatatypes.hpp"

namespace SilKit {
namespace Experimental {
namespace NetworkSimulation {

class INetworkSimulatorInternal 
{
public:

    // Returns the corresponding serviceDescriptor string to a given controllerDescriptor.
    // Returns an empty string and logs a warning if the controllerDescriptor is unknown.
    virtual auto GetServiceDescriptorString(ControllerDescriptor controllerDescriptor) -> std::string const = 0;
};

} // namespace NetworkSimulation
} // namespace Experimental
} // namespace SilKit

