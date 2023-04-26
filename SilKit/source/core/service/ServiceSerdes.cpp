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

#include "ServiceSerdes.hpp"
#include "InternalSerdes.hpp"
#include "ServiceDescriptor.hpp"

namespace SilKit {
namespace Core {

// ServiceDescriptor encoding is here, because it pulls in O_SilKit_Config
inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer,
    const SilKit::Core::ServiceDescriptor& msg)
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

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer,
    SilKit::Core::ServiceDescriptor& updatedMsg)
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
namespace Discovery {

// ParticipantDiscoveryEvent
inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer,
    const ParticipantDiscoveryEvent& msg)
{
    buffer << msg.participantName
        << msg.version
        << msg.services
        ;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer,
    ParticipantDiscoveryEvent& updatedMsg)
{
    buffer >> updatedMsg.participantName
        >> updatedMsg.version
        >> updatedMsg.services
        ;
    return buffer;
}

// ServiceDiscoveryEvent
inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer,
    const ServiceDiscoveryEvent& msg)
{
    buffer << msg.type
        << msg.serviceDescriptor
        ;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer,
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

} // namespace Discovery
} // namespace Core
} // namespace SilKit
