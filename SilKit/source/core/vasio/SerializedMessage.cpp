// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "SerializedMessage.hpp"

namespace SilKit {
namespace Core {

// Constructor from raw data (reading)
SerializedMessage::SerializedMessage(std::vector<uint8_t>&& blob)
    : _buffer{std::move(blob)}
{
    ReadNetworkHeaders();
}

auto SerializedMessage::ReleaseStorage() -> std::vector<uint8_t>
{
    auto buffer = _buffer.ReleaseStorage();
    if (buffer.size() > std::numeric_limits<uint32_t>::max())
        throw SilKitError{"SerializedMessage::Serialize: message buffer is too large"};

    // emplace the buffer size as the first element in the byte stream
    const auto bufferSize = static_cast<uint32_t>(buffer.size());
    memcpy(buffer.data(), &bufferSize, sizeof(uint32_t));
    return buffer;
}

auto SerializedMessage::GetMessageKind() const -> VAsioMsgKind
{
    return _messageKind;
}

auto SerializedMessage::GetRegistryKind() const -> RegistryMessageKind
{
    return _registryKind;
}

auto SerializedMessage::GetAggregationKind() const -> MessageAggregationKind
{
    return _aggregationKind;
}

auto SerializedMessage::GetRemoteIndex() const -> EndpointId
{
    if (!IsMwOrSim(_messageKind))
    {
        throw SilKitError("SerializedMessage::GetEndpointAddress called on wrong message kind: "
                          + std::to_string((int)_messageKind));
    }
    return _remoteIndex;
}

auto SerializedMessage::GetEndpointAddress() const -> EndpointAddress
{
    if (!IsMwOrSim(_messageKind))
    {
        throw SilKitError("SerializedMessage::GetEndpointAddress called on wrong message kind: "
                          + std::to_string((int)_messageKind));
    }
    return _endpointAddress;
}

void SerializedMessage::SetProtocolVersion(ProtocolVersion version)
{
    _buffer.SetProtocolVersion(version);
}

auto SerializedMessage::GetRegistryMessageHeader() const -> RegistryMsgHeader
{
    return _registryMessageHeader;
}

auto SerializedMessage::GetProxyMessageHeader() const -> ProxyMessageHeader
{
    return _proxyMessageHeader;
}

void SerializedMessage::WriteNetworkHeaders()
{
    _buffer << _messageSize; // placeholder for finalization via ReleaseStorage()
    _buffer << _messageKind;
    if (_messageKind == VAsioMsgKind::SilKitRegistryMessage)
    {
        _buffer << _registryKind;
    }
    if (IsMwOrSim(_messageKind))
    {
        _buffer << _remoteIndex << _endpointAddress;
    }
}

void SerializedMessage::ReadNetworkHeaders()
{
    _messageSize = ExtractMessageSize(_buffer);
    _messageKind = ExtractMessageKind(_buffer);
    if (_messageKind == VAsioMsgKind::SilKitRegistryMessage)
    {
        //optional registry kind tag
        _registryKind = ExtractRegistryMessageKind(_buffer);

        // Protocol Version:
        //   3.1:
        //     All handshake messages start with the RegistryMessageHeader
        //   3.0:
        //     ParticipantAnnouncement and KnownParticipants start with the RegistryMessageHeader
        //     ParticipantAnnouncementReply does _NOT_ start with the RegistryMessageHeader

        switch (_registryKind)
        {
        case RegistryMessageKind::ParticipantAnnouncement:
        case RegistryMessageKind::KnownParticipants:
        case RegistryMessageKind::RemoteParticipantConnectRequest:
            _registryMessageHeader = PeekRegistryMessageHeader(_buffer);
            break;
        case RegistryMessageKind::ParticipantAnnouncementReply:
            if (_buffer.GetProtocolVersion() == ProtocolVersion{3, 1})
            {
                _registryMessageHeader = PeekRegistryMessageHeader(_buffer);
            }
            else if (_buffer.GetProtocolVersion() == ProtocolVersion{3, 0})
            {
                // Since the ParticipantAnnouncementReply
                _registryMessageHeader = RegistryMsgHeader{};
                _registryMessageHeader.versionHigh = 3;
                _registryMessageHeader.versionLow = 0;
            }
            else
            {
                throw ProtocolError("SerializedMessage: Unsupported protocol version encountered during handshake");
            }
            break;
        case RegistryMessageKind::Invalid:
            throw ProtocolError("SerializedMessage: ReadNetworkHeaders() encountered RegistryMessageKind::Invalid");
        }
    }
    if (_messageKind == VAsioMsgKind::SilKitProxyMessage)
    {
        _proxyMessageHeader = PeekProxyMessageHeader(_buffer);
    }
    if (IsMwOrSim(_messageKind))
    {
        //optional remoteIndex and endpoint address
        _remoteIndex = ExtractEndpointId(_buffer);
        _endpointAddress = ExtractEndpointAddress(_buffer);
    }
}

void SerializedMessage::SetAggregationKind(MessageAggregationKind msgAggregationKind)
{
    _aggregationKind = msgAggregationKind;
}

} // namespace Core
} // namespace SilKit
