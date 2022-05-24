// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"
#include "InternalSerdes.hpp"
#include "ServiceDescriptor.hpp"

#include "ServiceDatatypes.hpp"

namespace ib {
namespace mw {

// ServiceDescriptor
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

//ServiceDiscoveryEvent

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
} // namespace service
} // namespace mw
} // namespace ib
