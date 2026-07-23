// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>
#include <string>
#include <vector>

#include "silkit/capi/Experimental.h"
#include "silkit/services/datatypes.hpp"

namespace SilKit {
namespace Experimental {
namespace ServiceDiscovery {

//! \brief The kind of change reported for a discovered service.
enum class ServiceDiscoveryEventType : SilKit_Experimental_ServiceDiscoveryEvent_Type
{
    //! An invalid / unknown service discovery event.
    Invalid = SilKit_Experimental_ServiceDiscoveryEvent_Type_Invalid,
    //! A service has been created (or was already present on registration).
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
    //! A link between two services: a pub/sub or RPC match, or a network-simulator link.
    Link = SilKit_Experimental_ServiceKind_Link,
};

//! \brief Describes a single discovered service, passed to a \ref ServiceDiscoveryHandler.
//!
//! A \ref ServiceKind::Link describes a link between two services: a pub/sub or RPC match, or a
//! network-simulator link. For a pub/sub or RPC match \p participantName / \p serviceName name the
//! receiving side (the DataSubscriber or RpcServer) and \p connectedParticipantName /
//! \p connectedServiceName name the peer (the DataPublisher or RpcClient). For a network-simulator
//! link \p participantName is the simulating participant, \p primaryIdentifier is the simulated
//! network name (matching the affected bus controllers' \p primaryIdentifier), and the
//! \p connected... fields are empty.
struct ServiceDescriptor
{
    //! Name of the participant providing the service.
    std::string participantName;
    //! Name of the service (the controller / publisher / subscriber / client / server name).
    std::string serviceName;
    //! The kind of service.
    ServiceKind serviceKind{ServiceKind::Undefined};
    //! The primary, user-facing identifier of the service: the network name for bus controllers and
    //! network-simulator links, the topic for pub/sub, and the function name for RPC.
    std::string primaryIdentifier;
    //! Media type for pub/sub and RPC services; empty string when not applicable.
    std::string mediaType;
    //! Decoded matching labels for pub/sub and RPC services; empty for bus controllers.
    std::vector<SilKit::Services::MatchingLabel> labels;
    //! The simulation name this service belongs to. Empty string when not available.
    std::string simulationName;
    //! Name of the peer participant; populated only in pub/sub and RPC Link events.
    std::string connectedParticipantName;
    //! Name of the peer service; populated only in pub/sub and RPC Link events.
    std::string connectedServiceName;
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
