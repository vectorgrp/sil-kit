// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ServiceSerdes.hpp"
#include "InternalSerdes.hpp"
#include "ServiceDescriptor.hpp"

namespace ib {
namespace mw {

// ServiceDescriptor encoding is here, because it pulls in IbConfiguration 
inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer,
    const ib::mw::ServiceDescriptor& msg)
{
    buffer
        << msg._participantName
        << msg._serviceType
        << msg._networkName
        << msg._networkType
        << msg._serviceName
        << msg._serviceId
        << msg._supplementalData
        << msg._participantId
        ;
    return buffer;
}

inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer,
    ib::mw::ServiceDescriptor& updatedMsg)
{
    buffer
        >> updatedMsg._participantName
        >> updatedMsg._serviceType
        >> updatedMsg._networkName
        >> updatedMsg._networkType
        >> updatedMsg._serviceName
        >> updatedMsg._serviceId
        >> updatedMsg._supplementalData
        >> updatedMsg._participantId
        ;
    return buffer;
}
namespace service {

// ParticipantDiscoveryEvent
inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer,
    const ParticipantDiscoveryEvent& msg)
{
    buffer << msg.participantName
        << msg.version
        << msg.services
        ;
    return buffer;
}

inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer,
    ParticipantDiscoveryEvent& updatedMsg)
{
    buffer >> updatedMsg.participantName
        >> updatedMsg.version
        >> updatedMsg.services
        ;
    return buffer;
}

// ServiceDiscoveryEvent
inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer,
    const ServiceDiscoveryEvent& msg)
{
    buffer << msg.type
        << msg.serviceDescriptor
        ;
    return buffer;
}

inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer,
    ServiceDiscoveryEvent& updatedMsg)
{
    buffer >> updatedMsg.type
        >> updatedMsg.serviceDescriptor
        ;
    return buffer;
}

void Serialize(MessageBuffer& buffer, const ParticipantDiscoveryEvent& msg)
{
    buffer << msg;
    return;
}
void Serialize(MessageBuffer& buffer, const ServiceDiscoveryEvent& msg)
{
    buffer << msg;
    return;
}

void Deserialize(MessageBuffer& buffer, ParticipantDiscoveryEvent& out)
{
    buffer >> out;
}
void Deserialize(MessageBuffer& buffer, ServiceDiscoveryEvent& out)
{
    buffer >> out;
}

} // namespace service
} // namespace mw
} // namespace ib
