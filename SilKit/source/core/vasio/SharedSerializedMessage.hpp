// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstddef>
#include <utility>

#include "core/vasio/SerializedMessage.hpp"
#include "util/SharedSpan.hpp"

namespace SilKit {
namespace Core {

/*! \brief A simulation message serialized once for delivery to any number of peers.
 *
 * The serialized form of a simulation message differs between its receivers only in the remote
 * index inside the network header. This type therefore serializes the message a single time and
 * exposes the header and the body separately, so that each peer can send its own small header
 * followed by the shared body instead of re-serializing the whole message per peer.
 *
 * The body is shared through a reference count, so it stays alive until the slowest peer has
 * finished writing it.
 *
 * NB: this relies on the serialized body being independent of the peer. That holds because
 *     simulation message serialization does not consult MessageBuffer::GetProtocolVersion();
 *     only the handshake and registry messages are protocol version dependent. If a simulation
 *     serdes ever becomes version dependent, the remote receivers have to be grouped by peer
 *     protocol version and serialized once per distinct version. See also
 *     README-network-compatibility.md.
 */
class SharedSerializedMessage
{
public:
    template <typename MessageT>
    SharedSerializedMessage(const MessageT& message, EndpointAddress endpointAddress);

    //! \brief The network header, carrying a placeholder remote index that the sender patches.
    auto Header() const -> Util::Span<const uint8_t>;
    //! \brief Everything after the network header, shared between all peers.
    auto Body() const -> const Util::SharedSpan<uint8_t>&;

    auto HeaderSize() const -> size_t;
    auto RemoteIndexOffset() const -> size_t;
    auto TotalSize() const -> size_t;
    auto GetAggregationKind() const -> MessageAggregationKind;

private:
    Util::SharedSpan<uint8_t> _blob;
    Util::SharedSpan<uint8_t> _body;
    size_t _headerSize{0};
    size_t _remoteIndexOffset{0};
    MessageAggregationKind _aggregationKind{MessageAggregationKind::Other};
};

// ================================================================================
//  Inline Implementations
// ================================================================================

template <typename MessageT>
SharedSerializedMessage::SharedSerializedMessage(const MessageT& message, EndpointAddress endpointAddress)
{
    // The remote index is patched per peer, so serialize with a placeholder.
    SerializedMessage serialized{message, endpointAddress, EndpointId{0}};

    _headerSize = serialized.GetHeaderSize();
    _remoteIndexOffset = serialized.GetRemoteIndexOffset();
    _aggregationKind = serialized.GetAggregationKind();

    auto blob = serialized.ReleaseStorage();
    const auto blobSize = blob.size();

    _blob = Util::SharedSpan<uint8_t>{std::move(blob)};
    _body = _blob.Subspan(_headerSize, blobSize - _headerSize);
}

inline auto SharedSerializedMessage::Header() const -> Util::Span<const uint8_t>
{
    return Util::Span<const uint8_t>{_blob.AsSpan().data(), _headerSize};
}

inline auto SharedSerializedMessage::Body() const -> const Util::SharedSpan<uint8_t>&
{
    return _body;
}

inline auto SharedSerializedMessage::HeaderSize() const -> size_t
{
    return _headerSize;
}

inline auto SharedSerializedMessage::RemoteIndexOffset() const -> size_t
{
    return _remoteIndexOffset;
}

inline auto SharedSerializedMessage::TotalSize() const -> size_t
{
    return _blob.size();
}

inline auto SharedSerializedMessage::GetAggregationKind() const -> MessageAggregationKind
{
    return _aggregationKind;
}

} // namespace Core
} // namespace SilKit
