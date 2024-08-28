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
    SerializedSize(const T& message)
    {
        MessageBuffer buffer;
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
    explicit SerializedMessage(const MessageT& message);
    // Sim messages have additional parameters:
    template <typename MessageT>
    explicit SerializedMessage(const MessageT& message, EndpointAddress endpointAddress, EndpointId remoteIndex);
    template <typename MessageT>
    explicit SerializedMessage(ProtocolVersion version, const MessageT& message);

    auto ReleaseStorage() -> std::vector<uint8_t>;

public: // Receiving a SerializedMessage: from binary blob to SilKitMessage<T>
    explicit SerializedMessage(std::vector<uint8_t>&& blob);

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
SerializedMessage::SerializedMessage(const MessageT& message)
{
    static SerializedSize<MessageT> messageSize{message};
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
SerializedMessage::SerializedMessage(ProtocolVersion version, const MessageT& message)
{
    static SerializedSize<MessageT> messageSize{message};
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
SerializedMessage::SerializedMessage(const MessageT& message, EndpointAddress endpointAddress, EndpointId remoteIndex)
{
    static SerializedSize<MessageT> messageSize{message};
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
