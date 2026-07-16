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
    NetworkLink = SilKit_Experimental_ServiceKind_NetworkLink,
};

//! \brief Describes a single discovered service, passed to a \ref ServiceDiscoveryHandler.
struct ServiceDescriptor
{
    //! Name of the participant providing the service.
    std::string participantName;
    //! Name of the service (the controller / publisher / subscriber / client / server name).
    std::string serviceName;
    //! The kind of service.
    ServiceKind serviceKind{ServiceKind::Undefined};
    //! The primary, user-facing identifier of the service: the network name for bus controllers
    //! (e.g. "CAN1") and network links, the topic for pub/sub, and the function name for RPC.
    std::string primaryIdentifier;
    //! Media type for pub/sub and RPC services; empty string when not applicable.
    std::string mediaType;
    //! Decoded matching labels for pub/sub and RPC services; empty for bus controllers and links.
    std::vector<SilKit::Services::MatchingLabel> labels;
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
