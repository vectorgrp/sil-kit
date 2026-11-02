// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
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
 * Receivers differ only in the remote index of the network header, so each peer sends its own
 * patched copy of the header followed by the shared body. Relies on simulation message serdes
 * being independent of the protocol version, see README-network-compatibility.md.
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

    //! The whole serialized message, network header included.
    auto Blob() const -> Util::Span<const uint8_t>;

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

inline auto SharedSerializedMessage::Blob() const -> Util::Span<const uint8_t>
{
    return _blob.AsSpan();
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
