// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SerializedMessage.hpp"

namespace ib {
namespace mw {

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
        throw std::runtime_error{"SerializedMessage::Serialize: message buffer is too large"};

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

auto SerializedMessage::GetRemoteIndex() const -> EndpointId
{
    if (!IsMwOrSim(_messageKind))
    {
        throw std::runtime_error("SerializedMessage::GetEndpointAddress called on wrong message kind: "
                                 + std::to_string((int)_messageKind));
    }
    return _remoteIndex;
}

auto SerializedMessage::GetEndpointAddress() const -> EndpointAddress
{
    if (!IsMwOrSim(_messageKind))
    {
        throw std::runtime_error("SerializedMessage::GetEndpointAddress called on wrong message kind: "
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

void SerializedMessage::WriteNetworkHeaders()
{
    _buffer << _messageSize; // placeholder for finalization via ReleaseStorage()
    _buffer << _messageKind;
    if (_messageKind == VAsioMsgKind::IbRegistryMessage)
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
    if (_messageKind == VAsioMsgKind::IbRegistryMessage)
    {
        //optional registry kind tag
        _registryKind = ExtractRegistryMessageKind(_buffer);
        switch(_registryKind)
        {
        case RegistryMessageKind::ParticipantAnnouncement:
        case RegistryMessageKind::ParticipantAnnouncementReply:
        case RegistryMessageKind::KnownParticipants:
            // all handshake messages contain a header
            _registryMessageHeader = PeekRegistryMessageHeader(_buffer);
            break;
        case RegistryMessageKind::Invalid:
            throw ProtocolError("SerializedMessage: ReadNetworkHeaders() encountered RegistryMessageKind::Invalid");
        }
    }
    if (IsMwOrSim(_messageKind))
    {
        //optional remoteIndex and endpoint address
        _remoteIndex = ExtractEndpointId(_buffer);
        _endpointAddress = ExtractEndpointAddress(_buffer);
    }
}

} // namespace mw
} // namespace ib
