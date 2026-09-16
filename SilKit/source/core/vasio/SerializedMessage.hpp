// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include "core/vasio/VAsioMsgKind.hpp"
#include "core/vasio/VAsioDatatypes.hpp"
#include "core/vasio/SerializedMessageTraits.hpp"
#include "core/vasio/AggregationMessageTraits.hpp"
#include "core/vasio/SerializedSizeHint.hpp"
#include "core/internal/MessageBuffer.hpp"

// Component specific Serialize/Deserialize functions
#include "core/vasio/VAsioSerdes.hpp"
#include "services/can/CanSerdes.hpp"
#include "services/lin/LinSerdes.hpp"
#include "services/ethernet/EthernetSerdes.hpp"
#include "services/flexray/FlexraySerdes.hpp"
#include "services/rpc/RpcSerdes.hpp"
#include "core/internal/InternalSerdes.hpp"
#include "services/orchestration/SyncSerdes.hpp"
#include "core/service/ServiceSerdes.hpp"
#include "core/requests/RequestReplySerdes.hpp"
#include "services/logging/LoggingSerdes.hpp"
#include "services/pubsub/DataSerdes.hpp"
#include "services/metrics/MetricsSerdes.hpp"

namespace SilKit {
namespace Core {

// Helper to allow calling Deserialize(MessageBuffer&, T&) inside of template method SerializedMessage::Deserialize<T>
template <typename... Args>
auto AdlDeserialize(Args&&... args) -> decltype(auto)
{
    return Deserialize(std::forward<Args>(args)...);
}

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
    //! \brief Read from a shared blob. Deserialized byte payloads then alias it rather than
    //!        being copied out of it, and keep it alive for as long as they are referenced.
    explicit SerializedMessage(Util::SharedSpan<uint8_t> blob);

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

    auto GetHeaderSize() const -> size_t
    {
        return _headerSize;
    }

    auto GetRemoteIndexOffset() const -> size_t
    {
        return _remoteIndexOffset;
    }

private:
    void WriteNetworkHeaders();
    void ReadNetworkHeaders();
    // Size of the network headers and the offset of _remoteIndex within them, both recorded by
    // WriteNetworkHeaders(). Derived from the actual write positions so they cannot drift from the
    // layout. Only meaningful for messages that carry a remote index (see IsMwOrSim).
    size_t _headerSize{0};
    size_t _remoteIndexOffset{0};
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
    _buffer.IncreaseCapacity(SerializedSizeHint<MessageT>::Of(message));

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
    _buffer.IncreaseCapacity(SerializedSizeHint<MessageT>::Of(message));

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
    _buffer.IncreaseCapacity(SerializedSizeHint<MessageT>::Of(message));

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
