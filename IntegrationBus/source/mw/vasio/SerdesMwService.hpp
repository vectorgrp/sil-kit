// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"
#include "SerdesMw.hpp"
#include "ServiceDescriptor.hpp"

#include "ServiceDatatypes.hpp"

namespace ib {
namespace mw {

//Special case for std::map<std::string, std::string> as used in ServiceDescription::supplementalData
// not generic enough to add to MessageBuffer.hpp.
// We encode it as follows:
// NUM_ELEMENTS N
// STRING key1 STRING value1
// ...
// STRING keyN STRING valueN
// 
// for a generic approach we would have to encode the key type and the element type somehow

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer,
    const std::map<std::string, std::string>& msg)
{
    buffer << static_cast<uint32_t>(msg.size());
    for (auto&& kv : msg)
    {
        buffer << kv.first
            << kv.second
            ;
    }
    return buffer;
}

inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer,
    std::map<std::string,std::string>& updatedMsg)
{
    std::map<std::string, std::string> tmp;// do not modify updatedMsg until we validated the input
    uint32_t numElements{ 0 };
    buffer >> numElements;

    for (auto i = 0u; i < numElements; i++)
    {
        std::string key;
        std::string value;
        buffer >> key >> value;
        tmp[key] = std::move(value);
    }
    if (numElements != tmp.size())
    {
        throw std::runtime_error("MessageBuffer unable to deserialize std::map<std::string, std::string>");
    }
    updatedMsg = std::move(tmp);
    return buffer;
}

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
// ServiceAnnouncement
inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer,
    const ServiceAnnouncement& msg)
{
    buffer << msg.participantName
        << msg.version
        << msg.services
        ;
    return buffer;
}

inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer,
    ServiceAnnouncement& updatedMsg)
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
        << msg.service
        ;
    return buffer;
}

inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer,
    ServiceDiscoveryEvent& updatedMsg)
{
    buffer >> updatedMsg.type
        >> updatedMsg.service
        ;
    return buffer;
}
} // namespace service
} // namespace mw
} // namespace ib
