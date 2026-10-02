// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace SilKit {
namespace Core {

/*! \brief The blobs that received messages are linearised into.
 *
 * Deserialized payloads alias their blob, so a blob is only reused once the pool holds the last
 * reference. A disabled pool allocates a fresh blob per message, see
 * Config::Experimental::useReceiveBufferPool.
 *
 * Not thread safe. The owning connection only uses it from its io thread.
 */
class ReceiveBlobPool
{
public:
    //! Maximum number of blobs held.
    static constexpr size_t MaxEntries{8};
    //! Blobs for larger messages are not pooled. Large messages are rare; do not hoard memory for them.
    static constexpr size_t MaxBlobSize{64 * 1024};
    //! A reused blob exceeds the requested size by at most this many bytes. A payload that is retained
    //! past its handler keeps its whole blob alive, so this bounds the memory a small payload can pin.
    static constexpr size_t MaxSlack{4 * 1024};

public:
    explicit ReceiveBlobPool(bool enabled);

    //! \brief A blob of at least size bytes with unspecified contents.
    auto Acquire(size_t size) -> std::shared_ptr<std::vector<uint8_t>>;

    auto IsEnabled() const -> bool;

private:
    bool _enabled;
    std::vector<std::shared_ptr<std::vector<uint8_t>>> _entries;
};

} // namespace Core
} // namespace SilKit
