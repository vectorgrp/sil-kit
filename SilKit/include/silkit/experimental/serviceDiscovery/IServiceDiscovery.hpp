// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/experimental/serviceDiscovery/ServiceDiscoveryDatatypes.hpp"

namespace SilKit {
namespace Experimental {
namespace ServiceDiscovery {

/*! \brief Passively observes the user-facing services (bus controllers, publishers/subscribers,
 *         RPC clients/servers, network-simulator links, pub/sub and RPC matches, lifecycle and time
 *         sync services) created by all participants in the simulation.
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
     * invoked for every user-facing service created or removed. Internal / infrastructure
     * services are not reported.
     *
     * Calling this method again replaces the handler; the previously set handler is no longer
     * invoked. The replacement does not re-deliver the initial snapshot of already-known services.
     * Unlike the C API, which adds a handler per call, this method keeps a single handler.
     *
     * \note Threading: the handler may be invoked on an internal SIL Kit worker thread or on an
     *       application thread that creates or destroys a service; the invoking thread is
     *       unspecified. Invocations are serialized (never concurrent). The handler must not block
     *       and must not call back into the participant. Registering a new handler from within a
     *       handler throws \ref SilKit::StateError. Copy any data that must outlive the call.
     *
     * \param handler The handler to be called on service creation and removal.
     */
    virtual void SetServiceDiscoveryHandler(ServiceDiscoveryHandler handler) = 0;
};

} // namespace ServiceDiscovery
} // namespace Experimental
} // namespace SilKit
