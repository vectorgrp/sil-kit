// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <ostream>
#include <sstream>
#include <vector>
#include <functional>

#include "EndpointAddress.hpp"
#include "ServiceDescriptor.hpp"
#include "ServiceConfigKeys.hpp"

namespace SilKit {
namespace Core {
inline std::ostream& operator<<(std::ostream& out, const ServiceDescriptor& id);

inline EndpointAddress to_endpointAddress(const ServiceDescriptor& descriptor);
// Creates a ServiceDescriptor based on an endpoint address - for testing purposes only!
inline auto from_endpointAddress(const EndpointAddress& epa) -> ServiceDescriptor;

inline bool AllowMessageProcessing(const ServiceDescriptor& lhs, const ServiceDescriptor& rhs);


class IServiceEndpoint
{
public:
    virtual ~IServiceEndpoint() = default;
    virtual void SetServiceDescriptor(const ServiceDescriptor& serviceDescriptor) = 0;
    virtual auto GetServiceDescriptor() const -> const ServiceDescriptor& = 0;
};

inline bool AllowMessageProcessing(const ServiceDescriptor& lhs, const ServiceDescriptor& rhs)
{
    return lhs.GetServiceId() == rhs.GetServiceId() && lhs.GetParticipantName() == rhs.GetParticipantName();
}

inline EndpointAddress to_endpointAddress(const ServiceDescriptor& descriptor)
{
    return descriptor.to_endpointAddress();
}

inline auto from_endpointAddress(const EndpointAddress& epa) -> ServiceDescriptor
{
    ServiceDescriptor endpoint{};
    endpoint.SetParticipantName(std::to_string(epa.participant));
    endpoint.SetServiceName(std::to_string((unsigned int)epa.endpoint));
    endpoint.SetServiceId(epa.endpoint);
    endpoint.SetNetworkName("generated");
    return endpoint;
}

inline std::ostream& operator<<(std::ostream& out, const ServiceDescriptor& descriptor)
{
    return out << descriptor.to_string();
}

} // namespace Core
} // namespace SilKit
