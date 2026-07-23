// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/capi/Experimental.h"

#include "core/internal/ServiceDescriptor.hpp"
#include "core/service/ServiceDatatypes.hpp"

#include <map>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace VSilKit {

// Translates the internal service-discovery events of a participant into the events of the
// experimental public service-discovery C API. It is driven from a single internal service-discovery
// handler (see CapiExperimental.cpp) and owns the small amount of state needed to correlate the raw
// internal announcements into fully-named public events.
//
// Reported events:
//  - Bus controllers, publishers/subscribers and RPC clients/servers are forwarded as their own
//    service kind on ServiceCreated / ServiceRemoved.
//  - A confirmed pub/sub or RPC match (announced internally as a DataSubscriberInternal /
//    RpcServerInternal endpoint) is surfaced as a SilKit_Experimental_ServiceKind_Link ServiceCreated
//    event once both endpoints are known; no removal event is emitted for it (its teardown is implied
//    by the ServiceRemoved of one of its endpoints).
//  - A network-simulator link is surfaced as a SilKit_Experimental_ServiceKind_Link on both
//    ServiceCreated and ServiceRemoved.
//  - Infrastructure / internal services are suppressed.
class ServiceObserver
{
public:
    ServiceObserver(SilKit_Experimental_ServiceDiscoveryHandler_t handler, void* context);

    // Handle a single internal discovery event, emitting zero or more public events through the
    // handler. Invocations are expected to be serialized (never concurrent) but may originate from
    // different threads; the internal state is guarded by a mutex and the handler is always invoked
    // outside that lock.
    void HandleEvent(SilKit::Core::Discovery::ServiceDiscoveryEvent::Type type,
                     const SilKit::Core::ServiceDescriptor& descriptor);

private:
    // Identifies a service on a participant: (participant name, service id).
    using ServiceKey = std::pair<std::string, SilKit::Core::EndpointId>;

    // An internal-match endpoint whose parent (subscriber/server) or peer (publisher/client) is not
    // yet known. Resolved into a Link event once both descriptors have been discovered.
    struct PendingMatch
    {
        ServiceKey parentKey;
        std::string peerUuid;
    };

    // A resolved link ready to be emitted (holds the backing storage for the emitted struct). The
    // parent descriptor is the receiving side (DataSubscriber / RpcServer); the connected... fields
    // name the peer (DataPublisher / RpcClient).
    struct LinkEmission
    {
        SilKit::Core::ServiceDescriptor parentDescriptor;
        std::string connectedParticipantName;
        std::string connectedServiceName;
    };

    void HandleInternalMatch(SilKit::Core::Discovery::ServiceDiscoveryEvent::Type type,
                             const SilKit::Core::ServiceDescriptor& descriptor, const std::string& parentIdKey,
                             const std::string& peerUuid);
    void HandlePeer(SilKit::Core::Discovery::ServiceDiscoveryEvent::Type type,
                    const SilKit::Core::ServiceDescriptor& descriptor);
    void HandleParent(SilKit::Core::Discovery::ServiceDiscoveryEvent::Type type,
                      const SilKit::Core::ServiceDescriptor& descriptor);

    // Emits any pending match whose parent and peer are now both known. Called with _mutex held.
    void DrainResolvablePending(std::vector<LinkEmission>& emissions);

    void EmitService(SilKit_Experimental_ServiceDiscoveryEvent_Type type,
                     const SilKit::Core::ServiceDescriptor& descriptor);
    void EmitLink(const LinkEmission& emission);

    static LinkEmission MakeLink(const SilKit::Core::ServiceDescriptor& parent,
                                 const SilKit::Core::ServiceDescriptor& peer);

    SilKit_Experimental_ServiceDiscoveryHandler_t _handler{};
    void* _context{nullptr};

    std::mutex _mutex;
    // DataPublisher / RpcClient UUID (= their networkName) -> their descriptor. The UUID is globally
    // unique, so publishers and clients share this map without collision.
    std::map<std::string, SilKit::Core::ServiceDescriptor> _peersByUuid;
    // (participant, serviceId) -> DataSubscriber / RpcServer descriptor (the receiving side).
    std::map<ServiceKey, SilKit::Core::ServiceDescriptor> _parents;
    // Internal-match endpoints awaiting resolution of their parent and/or peer.
    std::vector<PendingMatch> _pending;
};

} // namespace VSilKit
