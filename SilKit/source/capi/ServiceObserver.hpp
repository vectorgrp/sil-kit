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
//    RpcServerInternal endpoint) is surfaced as a PubSubMatch / RpcMatch while both endpoints are
//    known: created after both endpoints were reported, removed when the internal endpoint goes away
//    or right before the ServiceRemoved of either endpoint, whichever comes first.
//  - A network-simulator link is surfaced as a NetworkSimulatorLink (serviceName = bus type) on both
//    ServiceCreated and ServiceRemoved.
//  - A participant's lifecycle and time sync services are forwarded as their own service kind, with
//    the operation mode / synchronization state as primaryIdentifier.
//  - Infrastructure / internal services are suppressed.
class ServiceObserver
{
public:
    ServiceObserver(SilKit_Experimental_ServiceDiscoveryHandler_t handler, void* context);

    // Handle a single internal discovery event, emitting zero or more public events through the
    // handler. Invocations are expected to be serialized (never concurrent) but may originate from
    // different threads; the internal state is guarded by a mutex and the handler is always invoked
    // outside that lock. isSnapshot marks the replay of already known services on registration.
    void HandleEvent(SilKit::Core::Discovery::ServiceDiscoveryEvent::Type type,
                     const SilKit::Core::ServiceDescriptor& descriptor, bool isSnapshot = false);

private:
    // Identifies a service on a participant: (participant name, service id).
    using ServiceKey = std::pair<std::string, SilKit::Core::EndpointId>;

    // Identifies a match: (parent key, peer UUID).
    using MatchKey = std::pair<ServiceKey, std::string>;

    // An internal-match endpoint whose parent (subscriber/server) or peer (publisher/client) is not
    // yet known. Resolved into a PubSubMatch / RpcMatch once both descriptors have been discovered.
    struct PendingMatch
    {
        ServiceKey parentKey;
        std::string peerUuid;
        SilKit_Experimental_ServiceKind kind; // PubSubMatch or RpcMatch
    };

    // A match to be emitted (holds the backing storage for the emitted struct). The parent descriptor
    // is the receiving side (DataSubscriber / RpcServer); the connected... fields name the peer
    // (DataPublisher / RpcClient).
    struct MatchEmission
    {
        SilKit::Core::ServiceDescriptor parentDescriptor;
        std::string connectedParticipantName;
        std::string connectedServiceName;
        SilKit::Core::EndpointId connectedServiceId;
        SilKit_Experimental_ServiceKind kind;
    };

    void HandleInternalMatch(SilKit::Core::Discovery::ServiceDiscoveryEvent::Type type,
                             const SilKit::Core::ServiceDescriptor& descriptor, const std::string& parentIdKey,
                             const std::string& peerUuid, SilKit_Experimental_ServiceKind kind);
    void HandlePeer(SilKit::Core::Discovery::ServiceDiscoveryEvent::Type type,
                    const SilKit::Core::ServiceDescriptor& descriptor);
    void HandleParent(SilKit::Core::Discovery::ServiceDiscoveryEvent::Type type,
                      const SilKit::Core::ServiceDescriptor& descriptor);

    // Records a match whose parent and peer are both known (once) and appends it to
    // `created`. Returns false if parent or peer is still unknown. Called with _mutex held.
    auto TryResolve(const PendingMatch& match, std::vector<MatchEmission>& created) -> bool;
    // Resolves every pending match whose parent and peer are now both known. Called with _mutex held.
    void DrainResolvablePending(std::vector<MatchEmission>& created);
    // Moves the emitted matches whose MatchKey satisfies the predicate to `removed`. Called with _mutex held.
    template <typename Predicate>
    void TakeEmitted(Predicate predicate, std::vector<MatchEmission>& removed);

    void EmitService(SilKit_Experimental_ServiceDiscoveryEvent_Type type,
                     const SilKit::Core::ServiceDescriptor& descriptor);
    void EmitMatch(SilKit_Experimental_ServiceDiscoveryEvent_Type type, const MatchEmission& emission);
    void EmitAll(SilKit_Experimental_ServiceDiscoveryEvent_Type type, const std::vector<MatchEmission>& emissions);
    void Invoke(SilKit_Experimental_ServiceDiscoveryEvent_Type type,
                const SilKit_Experimental_ServiceDescriptor& descriptor);

    static MatchEmission MakeMatch(const SilKit::Core::ServiceDescriptor& parent,
                                 const SilKit::Core::ServiceDescriptor& peer, SilKit_Experimental_ServiceKind kind);

    SilKit_Experimental_ServiceDiscoveryHandler_t _handler{};
    void* _context{nullptr};
    // Snapshot flag of the event currently handled (HandleEvent calls are serialized).
    bool _isSnapshot{false};

    std::mutex _mutex;
    // DataPublisher / RpcClient UUID (= their networkName) -> their descriptor. The UUID is globally
    // unique, so publishers and clients share this map without collision.
    std::map<std::string, SilKit::Core::ServiceDescriptor> _peersByUuid;
    // (participant, serviceId) -> DataSubscriber / RpcServer descriptor (the receiving side).
    std::map<ServiceKey, SilKit::Core::ServiceDescriptor> _parents;
    // Internal-match endpoints awaiting resolution of their parent and/or peer.
    std::vector<PendingMatch> _pending;
    // Matches reported as created and not yet as removed.
    std::map<MatchKey, MatchEmission> _emitted;
};

} // namespace VSilKit
