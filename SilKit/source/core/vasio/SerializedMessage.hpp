// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include "VAsioMsgKind.hpp"
#include "VAsioDatatypes.hpp"
#include "SerializedMessageTraits.hpp"
#include "AggregationMessageTraits.hpp"
#include "MessageBuffer.hpp"

// Component specific Serialize/Deserialize functions
#include "VAsioSerdes.hpp"
#include "CanSerdes.hpp"
#include "LinSerdes.hpp"
#include "EthernetSerdes.hpp"
#include "FlexraySerdes.hpp"
#include "RpcSerdes.hpp"
#include "InternalSerdes.hpp"
#include "SyncSerdes.hpp"
#include "ServiceSerdes.hpp"
#include "RequestReplySerdes.hpp"
#include "LoggingSerdes.hpp"
#include "DataSerdes.hpp"
#include "MetricsSerdes.hpp"

namespace SilKit {
namespace Core {

// Helper to allow calling Deserialize(MessageBuffer&, T&) inside of template method SerializedMessage::Deserialize<T>
template <typename... Args>
auto AdlDeserialize(Args&&... args) -> decltype(auto)
{
    return Deserialize(std::forward<Args>(args)...);
}

template <typename T>
struct SerializedSize
{
    size_t _size{};
    SerializedSize(const T& message, std::pmr::memory_resource* memoryResource)
    {
        MessageBuffer buffer{memoryResource};
        Serialize(buffer, message);
        _size = buffer.ReleaseStorage().size();
    }
    size_t Size() const
    {
        return _size;
    }
};

// A serialized message used as binary wire format for the VAsio transport.
class SerializedMessage
{
public: //defaulted CTors
    SerializedMessage(SerializedMessage&&) = default;
    SerializedMessage& operator=(SerializedMessage&&) = default;
    SerializedMessage(const SerializedMessage&) = default;
    SerializedMessage& operator=(const SerializedMessage&) = default;

public: // Sending a SerializedMessage: from T to binary blob
    template <typename MessageT>
    explicit SerializedMessage(const MessageT& message, std::pmr::memory_resource* memoryResource);
    // Sim messages have additional parameters:
    template <typename MessageT>
    explicit SerializedMessage(const MessageT& message, EndpointAddress endpointAddress, EndpointId remoteIndex, std::pmr::memory_resource* memoryResource);
    template <typename MessageT>
    explicit SerializedMessage(ProtocolVersion version, const MessageT& message, std::pmr::memory_resource* memoryResource);

    auto ReleaseStorage() -> std::pmr::vector<uint8_t>;

public: // Receiving a SerializedMessage: from binary blob to SilKitMessage<T>
    explicit SerializedMessage(std::pmr::vector<uint8_t>&& blob, std::pmr::memory_resource* memoryResource);

    template <typename ApiMessageT>
    auto Deserialize() -> ApiMessageT;
    template <typename ApiMessageT>
    auto Deserialize() const -> ApiMessageT;

    auto GetMessageKind() const -> VAsioMsgKind;
    auto GetRegistryKind() const -> RegistryMessageKind;
    auto GetAggregationKind() const -> MessageAggregationKind;
    auto GetRemoteIndex() const -> EndpointId;
    auto GetEndpointAddress() const -> EndpointAddress;
    void SetProtocolVersion(ProtocolVersion version);
    auto GetProxyMessageHeader() const -> ProxyMessageHeader;
    auto GetRegistryMessageHeader() const -> RegistryMsgHeader;

    void SetAggregationKind(MessageAggregationKind msgAggregationKind);

    auto GetStorageSize() const -> size_t
    {
        return _buffer.PeekData().size();
    }

private:
    void WriteNetworkHeaders();
    void ReadNetworkHeaders();
    // network headers, some members are optional depending on messageKind
    uint32_t _messageSize{0};
    VAsioMsgKind _messageKind{VAsioMsgKind::Invalid};
    RegistryMessageKind _registryKind{RegistryMessageKind::Invalid};
    MessageAggregationKind _aggregationKind{MessageAggregationKind::Other};
    // For simMsg
    EndpointAddress _endpointAddress{};
    EndpointId _remoteIndex{0};
    // For registry messages
    RegistryMsgHeader _registryMessageHeader;
    // For proxy messages
    ProxyMessageHeader _proxyMessageHeader;

    MessageBuffer _buffer;
};

//////////////////////////////////////////////////////////////////////
// Inline Implementations
//////////////////////////////////////////////////////////////////////
template <typename MessageT>
SerializedMessage::SerializedMessage(const MessageT& message, std::pmr::memory_resource* memoryResource)
    :_buffer{memoryResource}
{
    static SerializedSize<MessageT> messageSize{message, memoryResource};
    _buffer.IncreaseCapacity(messageSize.Size());

    _messageKind = messageKind<MessageT>();
    _registryKind = registryMessageKind<MessageT>();
    _aggregationKind = aggregationKind<MessageT>();
    WriteNetworkHeaders();
    Serialize(_buffer, message);
    //Ensure we can directly Deserialize in unit tests by reading the header in again
    ReadNetworkHeaders();
}

template <typename MessageT>
SerializedMessage::SerializedMessage(ProtocolVersion version, const MessageT& message,
                                     std::pmr::memory_resource* memoryResource)
    : _buffer{memoryResource}
{
    static SerializedSize<MessageT> messageSize{message, memoryResource};
    _buffer.IncreaseCapacity(messageSize.Size());

    _messageKind = messageKind<MessageT>();
    _registryKind = registryMessageKind<MessageT>();
    _aggregationKind = aggregationKind<MessageT>();
    _buffer.SetProtocolVersion(version);
    WriteNetworkHeaders();
    Serialize(_buffer, message);
    //Ensure we can directly Deserialize in unit tests by reading the header in again
    ReadNetworkHeaders();
}

template <typename MessageT>
SerializedMessage::SerializedMessage(const MessageT& message, EndpointAddress endpointAddress, EndpointId remoteIndex,
                                     std::pmr::memory_resource* memoryResource)
    : _buffer{memoryResource}
{
    static SerializedSize<MessageT> messageSize{message, memoryResource};
    _buffer.IncreaseCapacity(messageSize.Size());

    _remoteIndex = remoteIndex;
    _endpointAddress = endpointAddress;
    _messageKind = messageKind<MessageT>();
    _registryKind = registryMessageKind<MessageT>();
    _aggregationKind = aggregationKind<MessageT>();
    WriteNetworkHeaders();
    Serialize(_buffer, message);
    //Ensure we can directly Deserialize in unit tests by reading the header in again
    ReadNetworkHeaders();
}

template <typename ApiMessageT>
auto SerializedMessage::Deserialize() -> ApiMessageT
{
    ApiMessageT value{};
    AdlDeserialize(_buffer, value);
    return value;
}

template <typename ApiMessageT>
auto SerializedMessage::Deserialize() const -> ApiMessageT
{
    auto bufferCopy = _buffer;
    ApiMessageT value{};
    AdlDeserialize(bufferCopy, value);
    return value;
}

} // namespace Core
} // namespace SilKit
