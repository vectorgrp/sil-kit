// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <ostream>
#include <sstream>
#include <string>

#include "silkit/experimental/serviceDiscovery/ServiceDiscoveryDatatypes.hpp"

namespace SilKit {
namespace Experimental {
namespace ServiceDiscovery {

inline std::string to_string(ServiceKind serviceKind);
inline std::string to_string(ServiceDiscoveryEventType eventType);

inline std::ostream& operator<<(std::ostream& out, ServiceKind serviceKind);
inline std::ostream& operator<<(std::ostream& out, ServiceDiscoveryEventType eventType);

// ================================================================================
//  Inline Implementations
// ================================================================================

std::string to_string(ServiceKind serviceKind)
{
    switch (serviceKind)
    {
    case ServiceKind::Undefined:
        return "Undefined";
    case ServiceKind::CanController:
        return "CanController";
    case ServiceKind::EthernetController:
        return "EthernetController";
    case ServiceKind::FlexrayController:
        return "FlexrayController";
    case ServiceKind::LinController:
        return "LinController";
    case ServiceKind::DataPublisher:
        return "DataPublisher";
    case ServiceKind::DataSubscriber:
        return "DataSubscriber";
    case ServiceKind::RpcClient:
        return "RpcClient";
    case ServiceKind::RpcServer:
        return "RpcServer";
    case ServiceKind::NetworkLink:
        return "NetworkLink";
    }

    std::stringstream out;
    out << "ServiceKind(" << static_cast<uint32_t>(serviceKind) << ")";
    return out.str();
}

std::string to_string(ServiceDiscoveryEventType eventType)
{
    switch (eventType)
    {
    case ServiceDiscoveryEventType::Invalid:
        return "Invalid";
    case ServiceDiscoveryEventType::ServiceCreated:
        return "ServiceCreated";
    case ServiceDiscoveryEventType::ServiceRemoved:
        return "ServiceRemoved";
    }

    std::stringstream out;
    out << "ServiceDiscoveryEventType(" << static_cast<uint32_t>(eventType) << ")";
    return out.str();
}

std::ostream& operator<<(std::ostream& out, ServiceKind serviceKind)
{
    return out << to_string(serviceKind);
}

std::ostream& operator<<(std::ostream& out, ServiceDiscoveryEventType eventType)
{
    return out << to_string(eventType);
}

} // namespace ServiceDiscovery
} // namespace Experimental
} // namespace SilKit
