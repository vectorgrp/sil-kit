// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "silkit/capi/Experimental.h"
#include "silkit/experimental/netsim/NetworkSimulatorDatatypes.hpp"
#include "silkit/services/datatypes.hpp"
#include "silkit/services/orchestration/OrchestrationDatatypes.hpp"

namespace SilKit {
namespace Experimental {
namespace ServiceDiscovery {

//! \brief The kind of change reported for a discovered service.
enum class ServiceDiscoveryEventType : SilKit_Experimental_ServiceDiscoveryEvent_Type
{
    //! An invalid / unknown service discovery event.
    Invalid = SilKit_Experimental_ServiceDiscoveryEvent_Type_Invalid,
    //! A service has been created (or was already present on registration, see
    //! \ref ServiceDescriptor::isSnapshot).
    ServiceCreated = SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated,
    //! A service has been removed.
    ServiceRemoved = SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved,
};

//! \brief The kind of a discovered service. Only user-facing services are reported.
enum class ServiceKind : SilKit_Experimental_ServiceKind
{
    Undefined = SilKit_Experimental_ServiceKind_Undefined,
    CanController = SilKit_Experimental_ServiceKind_CanController,
    EthernetController = SilKit_Experimental_ServiceKind_EthernetController,
    FlexrayController = SilKit_Experimental_ServiceKind_FlexrayController,
    LinController = SilKit_Experimental_ServiceKind_LinController,
    DataPublisher = SilKit_Experimental_ServiceKind_DataPublisher,
    DataSubscriber = SilKit_Experimental_ServiceKind_DataSubscriber,
    RpcClient = SilKit_Experimental_ServiceKind_RpcClient,
    RpcServer = SilKit_Experimental_ServiceKind_RpcServer,
    //! A network simulator simulates a network. The participant is the simulator, the primary
    //! identifier the network name, and the network type its bus type.
    NetworkSimulatorLink = SilKit_Experimental_ServiceKind_NetworkSimulatorLink,
    //! A DataPublisher (the connected side) is matched with a DataSubscriber; the primary identifier
    //! is the topic.
    PubSubMatch = SilKit_Experimental_ServiceKind_PubSubMatch,
    //! An RpcClient (the connected side) is matched with an RpcServer; the primary identifier is the
    //! function name.
    RpcMatch = SilKit_Experimental_ServiceKind_RpcMatch,
    //! The lifecycle of a participant; see \ref ServiceDescriptor::operationMode.
    LifecycleService = SilKit_Experimental_ServiceKind_LifecycleService,
    //! The time synchronization of a participant; see \ref ServiceDescriptor::timeSyncActive.
    TimeSyncService = SilKit_Experimental_ServiceKind_TimeSyncService,
};

//! \brief Describes a single discovered service, passed to a \ref ServiceDiscoveryHandler.
//!
//! A service is identified by \p participantName and \p serviceId. Fields that do not apply to a kind
//! keep their default values.
//!
//! Network-simulator links and pub/sub and RPC matches have kinds of their own:
//! \ref ServiceKind::NetworkSimulatorLink, \ref ServiceKind::PubSubMatch and \ref ServiceKind::RpcMatch.
//! Like all services, they are reported when they are created and when they are removed. A pub/sub or
//! RPC match is reported only while both of its endpoints are known: it is created after both
//! endpoints, and removed before the first of them.
struct ServiceDescriptor
{
    //! Name of the participant providing the service. For a pub/sub or RPC match the receiving side
    //! (subscriber/server), for a network-simulator link the simulating participant.
    std::string participantName;
    //! Name of the service (the controller / publisher / subscriber / client / server name).
    std::string serviceName;
    //! Identifier of the service, unique within its participant.
    uint64_t serviceId{0};
    //! The kind of service.
    ServiceKind serviceKind{ServiceKind::Undefined};
    //! The primary, user-facing identifier of the service: the network name for bus controllers and
    //! network-simulator links, the topic for pub/sub, and the function name for RPC; empty for
    //! lifecycle and time sync services.
    std::string primaryIdentifier;
    //! Bus type of bus controllers and network-simulator links; Undefined otherwise.
    SilKit::Experimental::NetworkSimulation::SimulatedNetworkType networkType{
        SilKit::Experimental::NetworkSimulation::SimulatedNetworkType::Undefined};
    //! Media type for pub/sub and RPC services; empty string when not applicable.
    std::string mediaType;
    //! Decoded matching labels for pub/sub and RPC services; empty for bus controllers.
    std::vector<SilKit::Services::MatchingLabel> labels;
    //! Operation mode of a \ref ServiceKind::LifecycleService; Invalid otherwise.
    SilKit::Services::Orchestration::OperationMode operationMode{
        SilKit::Services::Orchestration::OperationMode::Invalid};
    //! For a \ref ServiceKind::TimeSyncService: whether the participant takes part in the virtual time
    //! synchronization.
    bool timeSyncActive{false};
    //! Name of the peer participant (publisher/client); populated only for pub/sub and RPC matches.
    std::string connectedParticipantName;
    //! Name of the peer service (publisher/client); populated only for pub/sub and RPC matches.
    std::string connectedServiceName;
    //! Identifier of the peer service within its participant; populated only for pub/sub and RPC matches.
    uint64_t connectedServiceId{0};
    //! True if the service already existed when the handler was registered.
    bool isSnapshot{false};
};

/*! \brief Handler invoked when a user-facing service is created or removed in the simulation.
 *
 * \param eventType Whether the service was created or removed.
 * \param serviceDescriptor The affected service.
 */
using ServiceDiscoveryHandler =
    std::function<void(ServiceDiscoveryEventType eventType, const ServiceDescriptor& serviceDescriptor)>;

} // namespace ServiceDiscovery
} // namespace Experimental
} // namespace SilKit
