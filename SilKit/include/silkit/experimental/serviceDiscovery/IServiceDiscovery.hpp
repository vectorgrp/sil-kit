// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/experimental/serviceDiscovery/ServiceDiscoveryDatatypes.hpp"

namespace SilKit {
namespace Experimental {
namespace ServiceDiscovery {

/*! \brief Passively observes the user-facing services (bus controllers, publishers/subscribers,
 *         RPC clients/servers, network links) created by all participants in the simulation.
 *
 * \warning This interface is experimental and not part of the stable API and ABI of the SIL Kit. It
 *          may be removed or changed at any time without prior notice.
 */
class IServiceDiscovery
{
public:
    virtual ~IServiceDiscovery() = default;

    /*! \brief Register the handler that is called for every user-facing service in the simulation.
     *
     * Upon registration the handler is immediately invoked once for every service that is already
     * known, each reported as \ref ServiceDiscoveryEventType::ServiceCreated. It is subsequently
     * invoked for every user-facing service created or removed. Internal / infrastructure services
     * are not reported.
     *
     * \param handler The handler to be called on service creation and removal.
     */
    virtual void SetServiceDiscoveryHandler(ServiceDiscoveryHandler handler) = 0;
};

} // namespace ServiceDiscovery
} // namespace Experimental
} // namespace SilKit
