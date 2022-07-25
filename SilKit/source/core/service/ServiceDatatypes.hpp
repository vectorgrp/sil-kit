/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include <vector>
#include <map>
#include <sstream>

#include "IServiceEndpoint.hpp"

namespace SilKit {
namespace Core {
namespace Discovery {

struct ServiceDiscoveryEvent
{
    enum class Type
    {
        Invalid,
        ServiceCreated,
        ServiceRemoved,
    };
    Type type{ Type::Invalid };
    ServiceDescriptor serviceDescriptor;
};

struct ParticipantDiscoveryEvent //requires history >= 1
{
    std::string participantName;
    uint64_t version{1}; //!< version indicator is manually set and changed when announcements break compatibility
    std::vector<ServiceDescriptor> services; //!< list of services provided by the participant
};

////////////////////////////////////////////////////////////////////////////////
// Inline operators
////////////////////////////////////////////////////////////////////////////////
inline bool operator==(const ServiceDiscoveryEvent& lhs, const ServiceDiscoveryEvent& rhs)
{
    return lhs.type == rhs.type
        && lhs.serviceDescriptor == rhs.serviceDescriptor
        ;
}
inline bool operator==(const ParticipantDiscoveryEvent& lhs, const ParticipantDiscoveryEvent& rhs)
{
    return lhs.participantName == rhs.participantName
        && lhs.services == rhs.services
        ;
}
inline bool operator!=(const ParticipantDiscoveryEvent& lhs, const ParticipantDiscoveryEvent& rhs)
{
    return !(lhs == rhs);
}
////////////////////////////////////////////////////////////////////////////////
// Inline string utils
////////////////////////////////////////////////////////////////////////////////

inline std::ostream& operator<<(std::ostream& out, const ServiceDiscoveryEvent::Type& t)
{
    switch (t)
    {
    case ServiceDiscoveryEvent::Type::Invalid: out << "Invalid"; break;
    case ServiceDiscoveryEvent::Type::ServiceCreated: out << "ServiceCreated"; break;
    case ServiceDiscoveryEvent::Type::ServiceRemoved: out << "ServiceRemoved"; break;
    default:
        out << "Unknown ServiceDiscoveryEvent::Type{"
            << static_cast<std::underlying_type_t<ServiceDiscoveryEvent::Type>>(t);
    }
    return out;
}
inline std::ostream& operator<<(std::ostream& out, const ServiceDiscoveryEvent& event)
{
    out << "ServiceDiscoveryEvent{" << event.type << ", " << event.serviceDescriptor << "}";
    return out;
}

inline std::ostream& operator<<(std::ostream& out, const ParticipantDiscoveryEvent& serviceAnnouncement)
{
    out << "ParticipantDiscoveryEvent{\"" << serviceAnnouncement.participantName
        << "\", services=["
        ;
    for (auto&& service : serviceAnnouncement.services)
    {
        out << service
            << ", "
            ;
    }
    out << "] }";
    return out;
}
inline std::string to_string(const ParticipantDiscoveryEvent& serviceAnnouncement)
{
    std::stringstream str;
    str << serviceAnnouncement;
    return str.str();
}

} // namespace Discovery
} // namespace Core
} // namespace SilKit
