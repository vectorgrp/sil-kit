#include "SerdesCompat.hpp"
#include "ib/mw/ParticipantId.hpp"
#include "SerdesMwVAsio.hpp"

namespace version_v3_0
{
//these two structures were merged for v3.1
struct VAsioPeerInfo
{
    std::string participantName;
    ib::mw::ParticipantId participantId;
    std::string acceptorHost;
    uint16_t acceptorPort;
};

struct VAsioPeerUri
{
    std::string participantName;
    ib::mw::ParticipantId participantId;
    std::vector<std::string> acceptorUris;
};

struct ParticipantAnnouncement
{
    ib::mw::RegistryMsgHeader messageHeader; //not changed
    VAsioPeerInfo peerInfo;
    //!< additional field as of VIB >3.4.1, will be ignored by older participants
    VAsioPeerUri peerUri;
};

// Actual Serdes Code
inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const VAsioPeerInfo& peerInfo)
{
    buffer << peerInfo.participantName
           << peerInfo.participantId
           << peerInfo.acceptorHost
           << peerInfo.acceptorPort;
    return buffer;
}

inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, VAsioPeerInfo& peerInfo)
{
    buffer >> peerInfo.participantName
           >> peerInfo.participantId
           >> peerInfo.acceptorHost
           >> peerInfo.acceptorPort;
    return buffer;
}

//SerDes v3.1
inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const VAsioPeerUri& peerUri)
{
    buffer << peerUri.participantName
           << peerUri.participantId
           << peerUri.acceptorUris
           ;
    return buffer;
}

inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, VAsioPeerUri& peerUri)
{
    buffer >> peerUri.participantName
           >> peerUri.participantId
           >> peerUri.acceptorUris
        ;
    return buffer;
}


inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const ParticipantAnnouncement& announcement)
{
    buffer << announcement.messageHeader
        << announcement.peerInfo
        << announcement.peerUri
        ;

    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, ParticipantAnnouncement& announcement)
{
    buffer >> announcement.messageHeader
        >> announcement.peerInfo
        >> announcement.peerUri
        ;
    return buffer;
}
} //version_v3_0

namespace ib {
namespace mw {
void DeserializeCompat(ib::mw::MessageBuffer& buffer, ParticipantAnnouncement& announcement)
{
    if (buffer.GetFormatVersion() == ProtocolVersion{3,0})
    {
        //need legacy support here, convert old format to current one
        version_v3_0::ParticipantAnnouncement oldAnnouncement;
        buffer >> oldAnnouncement;
        announcement.messageHeader = oldAnnouncement.messageHeader;
        auto& info = announcement.peerInfo;
        auto& oldUri = oldAnnouncement.peerUri;
        info.participantName = oldUri.participantName;
        info.participantId = oldUri.participantId;
        info.acceptorUris = oldUri.acceptorUris;
    }
}
} //mw
} //ib
