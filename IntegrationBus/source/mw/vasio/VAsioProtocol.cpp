// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include "VAsioProtocol.hpp"

namespace ib {
namespace mw {
//////////////////////////////////////////////////////////////////////
//     Serialize
//////////////////////////////////////////////////////////////////////
auto Serialize(const ParticipantAnnouncementReply& reply) -> MessageBuffer
{
    MessageBuffer buffer;
    uint32_t msgSizePlaceholder{0u};

    buffer << msgSizePlaceholder
           << VAsioMsgKind::IbRegistryMessage
           << RegistryMessageKind::ParticipantAnnouncementReply
           << reply;
    return buffer;
}

auto Serialize(const ParticipantAnnouncement& announcement) -> MessageBuffer
{
    MessageBuffer buffer;
    uint32_t msgSizePlaceholder{0u};

    buffer << msgSizePlaceholder
           << VAsioMsgKind::IbRegistryMessage
           << RegistryMessageKind::ParticipantAnnouncement
           << announcement;
    return buffer;
}


 auto Serialize(const VAsioMsgSubscriber& subscriber) -> MessageBuffer
{
    ib::mw::MessageBuffer buffer;
    uint32_t rawMsgSize{0};
    buffer
        << rawMsgSize
        << VAsioMsgKind::SubscriptionAnnouncement
        << subscriber;

    return buffer;
}

// Subscription of services have a negotiated "connection"-version
 auto Serialize(uint32_t protocolVersion, const SubscriptionAcknowledge& ack) ->  MessageBuffer
{
    MessageBuffer ackBuffer;
    ackBuffer.SetFormatVersion(protocolVersion);

    uint32_t msgSizePlaceholder{0u};
    ackBuffer
        << msgSizePlaceholder
        << VAsioMsgKind::SubscriptionAcknowledge
        << ack;
    return ackBuffer;
}

//////////////////////////////////////////////////////////////////////
//  Protocol Versioning
//////////////////////////////////////////////////////////////////////
 bool ProtocolVersionSupported(const ParticipantAnnouncement& /*announcement*/)
{
    return true;
}

 auto ProtocolVersionToString(const ib::mw::RegistryMsgHeader& registryMsgHeader) -> std::string
{
    if (registryMsgHeader.versionHigh == 1)
    {
        return {"< v2.0.0"};
    }
    else if (registryMsgHeader.versionHigh == 2 && registryMsgHeader.versionLow == 0)
    {
        return {"v2.0.0 - v3.4.0"};
    }
    else if (registryMsgHeader.versionHigh == 2 && registryMsgHeader.versionLow == 1)
    {
        return {"v3.4.1 - v3.99.21"};
    }
    else if (registryMsgHeader.versionHigh == 3 && registryMsgHeader.versionLow == 0)
    {
        return {"v3.99.22 - current"};
    }

    return {"Unknown version range"};
}

}//mw
}//ib
