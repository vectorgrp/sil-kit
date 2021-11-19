// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <ostream>

#include <ib/cfg/Config.hpp>
#include <ib/mw/EndpointAddress.hpp>

namespace ib {
namespace mw {

struct ServiceId
{
    std::string id{ "Undefined" }; //!< Unique ID
    std::string linkName{ "Undefined" }; //!< the service's link name
    std::string participantName{ "Undefined" }; //!< name of the participant
    std::string serviceName{ "Undefined" }; //!< the controller's or the service's name
    bool isLinkSimulated{ false };
    cfg::Link::Type type{ cfg::Link::Type::Undefined };
    EndpointAddress legacyEpa{}; //!< legacy endpoint will be removed in the future
};

inline bool operator==(const ServiceId& lhs, const ServiceId& rhs);
inline std::string to_string(const ServiceId& id);
inline std::ostream& operator<<(std::ostream& out, const ServiceId& id);

//TODO rename IIbEndpoint
//     remove IIbEndpoint<MsgT> from IIbTo$Service interfaces
class IServiceId
{
public:
    virtual void SetServiceId(const ServiceId& serviceId) = 0;
    virtual auto GetServiceId() const -> const ServiceId& = 0;
};


// ================================================================================
//  Inline Implementations
// ================================================================================

inline bool operator==(const ServiceId& lhs, const ServiceId& rhs)
{
    return lhs.id == rhs.id
        && lhs.linkName == rhs.linkName
        && lhs.participantName == rhs.participantName
        && lhs.serviceName == rhs.serviceName
        && lhs.isLinkSimulated == rhs.isLinkSimulated
        && lhs.type == rhs.type
        && lhs.legacyEpa == rhs.legacyEpa
        ;
}

inline std::string to_string(const ServiceId& id)
{
    return id.id;
}
inline std::ostream& operator<<(std::ostream& out, const ServiceId& id)
{
    return out << to_string(id);
}

} // namespace mw
} // namespace ib
