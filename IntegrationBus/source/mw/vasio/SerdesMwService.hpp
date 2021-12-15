// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "MessageBuffer.hpp"
#include "SerdesMw.hpp"

#include "ServiceDatatypes.hpp"

namespace ib {
namespace mw {

// ServiceId
inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer,
    const ib::mw::ServiceId& msg)
{
    buffer
        << msg.participantName
        << msg.linkName
        << msg.serviceName
        << msg.type
        << msg.legacyEpa
        << msg.isLinkSimulated
        ;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer,
    ib::mw::ServiceId& updatedMsg)
{
    buffer
        >> updatedMsg.participantName
        >> updatedMsg.linkName
        >> updatedMsg.serviceName
        >> updatedMsg.type
        >> updatedMsg.legacyEpa
        >> updatedMsg.isLinkSimulated
        ;
    return buffer;
}
//Special case for std::map<std::string, std::string> ServiceDescription::supplementalData
// not generic enough to add to MessageBuffer.hpp.
// We encode it as follows:
// NUM_ELEMENTS
// VECTOR keys
// VECTOR elems
// 
// for a generic approach we would have to encode the key type and the element type somehow

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer,
    const std::map<std::string, std::string>& msg)
{
    std::vector<std::string> keys;
    std::vector<std::string> values;
    for (auto&& kv : msg)
    {
        keys.push_back(kv.first);
        values.push_back(kv.second);
    }

    buffer << msg.size()
        << keys
        << values
        ;
    return buffer;
}

inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer,
    std::map<std::string,std::string>& updatedMsg)
{
    std::vector<std::string> keys;
    std::vector<std::string> values;
    size_t numElements;
    buffer >> numElements
        >> keys
        >> values
        ;
    if (numElements != keys.size() || numElements != values.size())
    {
        throw std::runtime_error("MessageBuffer unable to deserialize std::map<std::string, std::string>");
    }
    for (auto i = 0u; i < keys.size(); i++)
    {
        updatedMsg[keys[i]] = values[i];
    }
    return buffer;
}

namespace service {

// ServiceDescription
inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer,
    const ServiceDescription& msg)
{
    buffer <<  msg.serviceId
        << msg.supplementalData
        ;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer,
    ServiceDescription& updatedMsg)
{
    buffer
        >> updatedMsg.serviceId
        >> updatedMsg.supplementalData
        ;
    return buffer;
}

// ServiceAnnouncement
inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer,
    const ServiceAnnouncement& msg)
{
    buffer << msg.participantName
        << msg.services
        ;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer,
    ServiceAnnouncement& updatedMsg)
{
    buffer >> updatedMsg.participantName
        >> updatedMsg.services
        ;
    return buffer;
}

} // namespace service
} // namespace mw
} // namespace ib
