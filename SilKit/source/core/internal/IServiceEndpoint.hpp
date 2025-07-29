// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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
inline std::ostream& operator<<(std::ostream& out, const ServiceDescriptor& descriptor);

inline EndpointAddress to_endpointAddress(const ServiceDescriptor& descriptor);

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

inline std::ostream& operator<<(std::ostream& out, const ServiceDescriptor& descriptor)
{
    return out << descriptor.to_string();
}

} // namespace Core
} // namespace SilKit
