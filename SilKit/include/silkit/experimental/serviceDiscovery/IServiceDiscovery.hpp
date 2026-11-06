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

    /*! \brief Set the handler that is called for every user-facing service in the simulation.
     *
     * On the first call the handler is immediately invoked once for every service that this participant
     * already knows, each reported as \ref ServiceDiscoveryEventType::ServiceCreated. These snapshot
     * invocations happen synchronously on the calling thread, before this method returns; a consumer that
     * needs to tell them apart from live events can treat invocations on the calling thread during this
     * call as the snapshot. A service of another participant can become known to this participant only
     * after the handler was first set, even if it existed before, and is then reported as a live event. The handler
     * is subsequently invoked for every user-facing service created or removed. Internal / infrastructure
     * services are not reported.
     *
     * If reporting the snapshot fails, this method throws and the handler is not set; it may already have
     * received part of the snapshot.
     *
     * Calling this method again replaces the handler; the previously set handler is no longer
     * invoked. The replacement does not re-deliver the initial snapshot of already-known services. An
     * invocation already running on another thread may still use the previous handler.
     *
     * \note Threading: the handler may be invoked on an internal SIL Kit worker thread or on an
     *       application thread that creates or destroys a service; the invoking thread is
     *       unspecified. Invocations are serialized (never concurrent). The handler must not block
     *       and must not call back into the participant. Calling this method from within the handler
     *       throws \ref SilKit::StateError. Copy any data that must outlive the call.
     *
     * \param handler The handler to be called on service creation and removal.
     */
    virtual void SetServiceDiscoveryHandler(ServiceDiscoveryHandler handler) = 0;
};

} // namespace ServiceDiscovery
} // namespace Experimental
} // namespace SilKit
