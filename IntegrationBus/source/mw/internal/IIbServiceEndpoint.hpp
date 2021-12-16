// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <ostream>
#include <sstream>
#include <vector>
#include <functional>

#include <ib/cfg/Config.hpp>
#include <ib/mw/EndpointAddress.hpp>

namespace ib {
namespace mw {
struct ServiceId
{
    std::string participantName{ "Undefined" }; //!< name of the participant
    std::string linkName{ "Undefined" }; //!< the service's link name
    std::string serviceName{ "Undefined" }; //!< the controller's or the service's name
    std::uint16_t serviceId{ 0 };
    bool isLinkSimulated{ false };
    cfg::Link::Type type{ cfg::Link::Type::Undefined };
    EndpointAddress legacyEpa{}; //!< legacy endpoint will be removed in the future
};

//!< Returns the ServiceId encoded as a string containing the tuple (participantName, linkName, serviceName)
inline std::string to_string(const ServiceId& id);
inline std::ostream& operator<<(std::ostream& out, const ServiceId& id);

inline bool operator==(const ServiceId& lhs, const ServiceId& rhs);
inline bool operator!=(const ServiceId& lhs, const ServiceId& rhs);
inline bool AllowMessageProcessingProxy(const ServiceId& lhs, const ServiceId& rhs);
inline bool AllowMessageProcessing(const ServiceId& lhs, const ServiceId& rhs);


//!< Parses a serialized ServiceId into a struct. This is a lossy operation.
inline auto from_string(const std::string& str)->ServiceId;
// Creates a ServiceId based on an endpoint address - for testing purposes only!
inline auto from_endpointAddress(const EndpointAddress& epa)->ServiceId;

//TODO 
//     remove IIbEndpoint<MsgT> from IIbTo$Service interfaces
class IIbServiceEndpoint
{
public:
    virtual ~IIbServiceEndpoint() = default;
    virtual void SetServiceId(const ServiceId& serviceId) = 0;
    virtual auto GetServiceId() const -> const ServiceId& = 0;
};


// ================================================================================
//  Inline Implementations
// ================================================================================

inline bool operator==(const ServiceId& lhs, const ServiceId& rhs)
{
    return
        lhs.linkName == rhs.linkName
        && lhs.participantName == rhs.participantName
        && lhs.serviceName == rhs.serviceName
        && lhs.isLinkSimulated == rhs.isLinkSimulated
        && lhs.type == rhs.type
        ;
}

inline bool operator!=(const ServiceId& lhs, const ServiceId& rhs)
{
    return !(lhs == rhs);
}

inline bool AllowMessageProcessingProxy(const ServiceId& lhs, const ServiceId& rhs)
{
  return
    lhs.legacyEpa.endpoint == rhs.legacyEpa.endpoint
    && lhs.participantName != rhs.participantName
    ;
}

inline bool AllowMessageProcessing(const ServiceId& lhs, const ServiceId& rhs)
{
    return
        lhs.legacyEpa.endpoint == rhs.legacyEpa.endpoint
        && lhs.participantName == rhs.participantName
        ;
}

inline bool EqualsParticipant(const ServiceId& lhs, const ServiceId& rhs)
{
    return lhs.participantName == rhs.participantName;
}


inline std::string to_string(const ServiceId& id)
{
    // Compute Id
    const std::string separator{ "/" };
    std::stringstream ss;
    ss << id.participantName
        << separator
        << id.linkName
        << separator
        << id.serviceName
        ;
    return ss.str();
}

inline auto from_string(const std::string& str) -> ServiceId
{
    const std::string separator{ "/" };
    auto input = str;
    std::vector<std::string> tokens;

    for (auto i = input.find(separator); i != input.npos; i = input.find(separator))
    {
        tokens.emplace_back(std::move(input.substr(0, i)));
        input = input.substr(i + 1);
    }

    if (!input.empty())
    {
        tokens.emplace_back(std::move(input));
    }

    if (tokens.size() != 3)
    {
        throw std::runtime_error("Cannot parse ServiceId from \"" + str +"\"");
    }

    ServiceId id{};
    id.participantName = tokens.at(0);
    id.linkName = tokens.at(1);
    id.serviceName = tokens.at(2);
    return id;
}

inline auto from_endpointAddress(const EndpointAddress& epa) -> ServiceId
{
    ServiceId endpoint{};
    endpoint.legacyEpa = epa;
    endpoint.participantName = std::to_string(epa.participant);
    endpoint.serviceName = std::to_string(epa.endpoint);
    endpoint.serviceId = epa.endpoint;
    return endpoint;
}

inline std::ostream& operator<<(std::ostream& out, const ServiceId& id)
{
    return out << to_string(id);
}

} // namespace mw
} // namespace ib
