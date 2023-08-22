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

#include "VAsioSerdes.hpp"
#include "VAsioPeerInfo.hpp"

#include "Uri.hpp"
#include "InternalSerdes.hpp"
#include "ProtocolVersion.hpp"
#include "VAsioProtocolVersion.hpp" // from_header(ProtcolVersion)

// Backward compatibility:
#include "VAsioSerdes_Protocol30.hpp"

namespace SilKit {
namespace Core {

inline MessageBuffer& operator<<(MessageBuffer& buffer, const RegistryMsgHeader& header)
{
    buffer << header.preamble
           << header.versionHigh
           << header.versionLow;
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, RegistryMsgHeader& header)
{
    buffer >> header.preamble
           >> header.versionHigh
           >> header.versionLow;
    return buffer;
}


inline MessageBuffer& operator<<(MessageBuffer& buffer, const VAsioPeerInfo& peerInfo)
{
    buffer << peerInfo.participantName
           << peerInfo.participantId
           << peerInfo.acceptorUris
           << peerInfo.capabilities
           ;
    return buffer;
}

inline MessageBuffer& operator>>(MessageBuffer& buffer, VAsioPeerInfo& peerInfo)
{
    buffer >> peerInfo.participantName
           >> peerInfo.participantId
           >> peerInfo.acceptorUris
           >> peerInfo.capabilities
        ;
    return buffer;
}

inline MessageBuffer& operator<<(MessageBuffer& buffer, const VAsioMsgSubscriber& subscriber)
{
    buffer << subscriber.receiverIdx
           << subscriber.networkName
           << subscriber.msgTypeName
           << subscriber.version
        ;
    return buffer;
}

inline MessageBuffer& operator>>(MessageBuffer& buffer, VAsioMsgSubscriber& subscriber)
{
    buffer >> subscriber.receiverIdx
           >> subscriber.networkName
           >> subscriber.msgTypeName
           >> subscriber.version
        ;
    return buffer;
}

inline MessageBuffer& operator<<(MessageBuffer& buffer, const SubscriptionAcknowledge& ack)
{
    buffer << ack.status
           << ack.subscriber;
    return buffer;
}

inline MessageBuffer& operator>>(MessageBuffer& buffer, SubscriptionAcknowledge& ack)
{
    buffer >> ack.status
           >> ack.subscriber;
    return buffer;
}

inline MessageBuffer& operator<<(MessageBuffer& buffer, const ParticipantAnnouncement& announcement)
{
    // ParticipantAnnouncement is the first message sent during a handshake.
    // so we need to extract its version information for ser/des here.
    buffer.SetProtocolVersion(ExtractProtocolVersion(announcement.messageHeader));
    if (buffer.GetProtocolVersion() == ProtocolVersion{3,0})
    {
        SerializeV30(buffer, announcement);
    }
    else
    {
        buffer
            << announcement.messageHeader
            << announcement.peerInfo
            ;
    }

    return buffer;
}

inline MessageBuffer& operator>>(MessageBuffer& buffer, ParticipantAnnouncement& announcement)
{
    //Backward compatibility
    if (buffer.GetProtocolVersion() == ProtocolVersion{3,0})
    {
        DeserializeV30(buffer, announcement);
    }
    else
    {
        //  default
        buffer
            >> announcement.messageHeader
            >> announcement.peerInfo
            ;
    }
    return buffer;
}

inline MessageBuffer& operator<<(MessageBuffer& buffer, const ParticipantAnnouncementReply& reply)
{
    //Backward compatibility
    if (buffer.GetProtocolVersion() == ProtocolVersion{3,0})
    {
        SerializeV30(buffer, reply);
    }
    else
    {
        buffer  
            << reply.remoteHeader
            << reply.status
            << reply.subscribers
            // Added in 4.0.8.
            << reply.diagnostic;
    }
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, ParticipantAnnouncementReply& reply)
{
    //Backward compatibility
    if (buffer.GetProtocolVersion() == ProtocolVersion{3,0})
    {
        buffer.SetProtocolVersion({3,0});
        DeserializeV30(buffer, reply);
    }
    else
    {
        buffer 
            >> reply.remoteHeader
            >> reply.status
            >> reply.subscribers;

        // Added in 4.0.8.
        if (buffer.RemainingBytesLeft() > 0)
        {
            buffer >> reply.diagnostic;
        }
    }
    return buffer;
}

inline MessageBuffer& operator<<(MessageBuffer& buffer, const KnownParticipants& participants)
{
    //Backward compatibility with legacy peers
    if (buffer.GetProtocolVersion() == ProtocolVersion{3,0})
    {
        SerializeV30(buffer, participants);
    }
    else
    {
        buffer << participants.messageHeader
            << participants.peerInfos
            ;
    }
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, KnownParticipants& participants)
{
    //Backward compatibility with legacy peers
    if (buffer.GetProtocolVersion() == ProtocolVersion{3,0})
    {
        DeserializeV30(buffer, participants);
    }
    else
    {
        buffer >> participants.messageHeader
            >> participants.peerInfos
            ;
    }
    return buffer;
}

inline MessageBuffer& operator<<(MessageBuffer& buffer, const ProxyMessageHeader& msg)
{
    //Backward compatibility with legacy peers
    if (buffer.GetProtocolVersion() < ProtocolVersion{3,1})
    {
        throw SilKit::ProtocolError{"ProxyMessage is not supported in protocol versions < 3.1"};
    }
    else
    {
        buffer
            << msg.version
            ;
    }
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, ProxyMessageHeader& out)
{
    //Backward compatibility with legacy peers
    if (buffer.GetProtocolVersion() < ProtocolVersion{3,1})
    {
        throw SilKit::ProtocolError{"ProxyMessage is not supported in protocol versions < 3.1"};
    }
    else
    {
        buffer
            >> out.version
            ;
    }
    return buffer;
}

inline MessageBuffer& operator<<(MessageBuffer& buffer, const ProxyMessage& msg)
{
    //Backward compatibility with legacy peers
    if (buffer.GetProtocolVersion() < ProtocolVersion{3,1})
    {
        throw SilKit::ProtocolError{"ProxyMessage is not supported in protocol versions < 3.1"};
    }
    else
    {
        buffer
            << msg.header
            << msg.source
            << msg.destination
            << msg.payload
            ;
    }
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, ProxyMessage& out)
{
    //Backward compatibility with legacy peers
    if (buffer.GetProtocolVersion() < ProtocolVersion{3,1})
    {
        throw SilKit::ProtocolError{"ProxyMessage is not supported in protocol versions < 3.1"};
    }
    else
    {
        buffer
            >> out.header
            >> out.source
            >> out.destination
            >> out.payload
            ;
    }
    return buffer;
}


inline MessageBuffer& operator<<(MessageBuffer& buffer, const RemoteParticipantConnectRequest& msg)
{
	buffer
		<< msg.peerUnableToConnect
		<< msg.connectTargetPeer
		;
    return buffer;
}
inline MessageBuffer& operator>>(MessageBuffer& buffer, RemoteParticipantConnectRequest& out)
{
	buffer
		>> out.peerUnableToConnect
		>> out.connectTargetPeer
		;
    return buffer;
}

//////////////////////////////////////////////////////////////////////
// Public Functions
//////////////////////////////////////////////////////////////////////

// Handshake primitives for wire format
auto ExtractMessageSize(MessageBuffer& buffer) -> uint32_t
{
    uint32_t messageSize{0};
    buffer >> messageSize;
    return messageSize;
}
// Extract the message kind tag (second element in wire format message)
auto ExtractMessageKind(MessageBuffer& buffer) -> VAsioMsgKind
{
    VAsioMsgKind messageKind{};
    buffer >> messageKind;
    return messageKind;
}

auto ExtractRegistryMessageKind(MessageBuffer& buffer) -> RegistryMessageKind
{
    RegistryMessageKind kind;
    buffer >> kind;
    return kind;
}

auto PeekProxyMessageHeader(MessageBuffer& buffer) -> ProxyMessageHeader
{
    MessageBufferPeeker peeker{buffer};

    ProxyMessageHeader header{};
    buffer >> header;
    return header;
}

auto PeekRegistryMessageHeader(MessageBuffer& buffer) -> RegistryMsgHeader
{
    // NB: At the moment using the MessageBufferPeeker here -although correct- leads to an issue in the
    //     Test_ParticipantVersion.cpp. The compatibility code between protocol version 3.0 and 3.1 does not
    //     handle processing the ParticipantAnnouncementReply (3.0) because the protocol version is not communicated
    //     early enough, when creating the SerializedMessage.
    //     This requires some deeper refactoring of the ser./des. code which will be tackled together with some other
    //     improvements.

    // read only the header into a new MessageBuffer
    auto data = buffer.PeekData();
    const auto readPos = buffer.ReadPos();
    std::vector<uint8_t> rawHeader;
    rawHeader.resize(sizeof(RegistryMsgHeader));
    memcpy(rawHeader.data(), (data.data() + readPos), sizeof(RegistryMsgHeader));

    MessageBuffer headerBuffer(std::move(rawHeader));
    RegistryMsgHeader header;
    headerBuffer >> header;
    return header;
}

auto ExtractEndpointId(MessageBuffer& buffer) -> EndpointId
{
    EndpointId endpointId;
    buffer >> endpointId;
    return endpointId;
}
auto ExtractEndpointAddress(MessageBuffer& buffer) -> EndpointAddress
{
    EndpointAddress endpointAddress;
    buffer >> endpointAddress;
    return endpointAddress;
}

void Serialize(MessageBuffer& buffer, const ParticipantAnnouncementReply& msg)
{
    buffer << msg;
}
void Deserialize(MessageBuffer& buffer,ParticipantAnnouncementReply& out)
{
    buffer >> out;
}

void Serialize(MessageBuffer& buffer, const ParticipantAnnouncement& msg)
{
    buffer << msg;
}
void Deserialize(MessageBuffer& buffer, ParticipantAnnouncement& out)
{
    buffer >> out;
}

 void Serialize(MessageBuffer& buffer, const VAsioMsgSubscriber& msg)
{
    buffer << msg;
}

void Deserialize(MessageBuffer& buffer, VAsioMsgSubscriber& out)
{
    buffer >> out;
}

 void Serialize(MessageBuffer& buffer, const SubscriptionAcknowledge& msg)
{
    buffer<< msg;
}
void Deserialize(MessageBuffer& buffer, SubscriptionAcknowledge& out)
{
    buffer >> out;
}

void Serialize(MessageBuffer& buffer, const KnownParticipants& msg)
{
    buffer << msg;
}
void Deserialize(MessageBuffer& buffer,KnownParticipants& out)
{
    buffer >> out;
}

void Serialize(MessageBuffer& buffer, const ProxyMessage& msg)
{
    buffer << msg;
}
void Deserialize(MessageBuffer& buffer, ProxyMessage& out)
{
    buffer >> out;
}


void Serialize(MessageBuffer& buffer, const RemoteParticipantConnectRequest& msg)
{
    buffer << msg;
}
void Deserialize(MessageBuffer& buffer, RemoteParticipantConnectRequest& out)
{
    buffer >> out;
}

} // namespace Core
} // namespace SilKit
