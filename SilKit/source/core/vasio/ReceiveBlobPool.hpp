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

/*! \brief The blob that received messages are linearised into.
 *
 * Deserialized payloads alias the blob, so it is only reused once the pool holds the last reference.
 * Dispatch is synchronous, so by the next message that is the case unless a payload was retained. A
 * retained payload keeps the whole blob, up to MaxBlobSize bytes, and the pool continues with a fresh
 * one. A disabled pool allocates a fresh blob per message, see
 * Config::Experimental::useReceiveBufferPool.
 *
 * Not thread safe. The owning connection only uses it from its io thread.
 */
class ReceiveBlobPool
{
public:
    //! Blobs for larger messages are not pooled. Large messages are rare; do not hoard memory for them.
    static constexpr size_t MaxBlobSize{64 * 1024};

public:
    explicit ReceiveBlobPool(bool enabled);

    //! \brief A blob of at least size bytes with unspecified contents.
    auto Acquire(size_t size) -> std::shared_ptr<std::vector<uint8_t>>;

    auto IsEnabled() const -> bool;

private:
    bool _enabled;
    std::shared_ptr<std::vector<uint8_t>> _blob;
};

} // namespace Core
} // namespace SilKit
