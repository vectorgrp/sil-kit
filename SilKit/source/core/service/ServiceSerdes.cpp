// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ServiceSerdes.hpp"
#include "InternalSerdes.hpp"
#include "ServiceDescriptor.hpp"

namespace SilKit {
namespace Core {

// ServiceDescriptor encoding is here, because it pulls in O_SilKit_Config
inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer,
                                               const SilKit::Core::ServiceDescriptor& msg)
{
    buffer << msg._participantName << msg._serviceType << msg._networkName << msg._networkType << msg._serviceName
           << msg._serviceId << msg._supplementalData << msg._participantId;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer,
                                               SilKit::Core::ServiceDescriptor& updatedMsg)
{
    buffer >> updatedMsg._participantName >> updatedMsg._serviceType >> updatedMsg._networkName
        >> updatedMsg._networkType >> updatedMsg._serviceName >> updatedMsg._serviceId >> updatedMsg._supplementalData
        >> updatedMsg._participantId;
    return buffer;
}
namespace Discovery {

// ParticipantDiscoveryEvent
inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer,
                                               const ParticipantDiscoveryEvent& msg)
{
    buffer << msg.participantName << msg.version << msg.services;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer,
                                               ParticipantDiscoveryEvent& updatedMsg)
{
    buffer >> updatedMsg.participantName >> updatedMsg.version >> updatedMsg.services;
    return buffer;
}

// ServiceDiscoveryEvent
inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const ServiceDiscoveryEvent& msg)
{
    buffer << msg.type << msg.serviceDescriptor;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, ServiceDiscoveryEvent& updatedMsg)
{
    buffer >> updatedMsg.type >> updatedMsg.serviceDescriptor;
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

} // namespace Discovery
} // namespace Core
} // namespace SilKit
