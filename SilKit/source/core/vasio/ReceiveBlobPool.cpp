// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "core/vasio/ReceiveBlobPool.hpp"

#include <atomic>

namespace SilKit {
namespace Core {

ReceiveBlobPool::ReceiveBlobPool(bool enabled)
    : _enabled{enabled}
{
}

auto ReceiveBlobPool::IsEnabled() const -> bool
{
    return _enabled;
}

auto ReceiveBlobPool::Acquire(size_t size) -> std::shared_ptr<std::vector<uint8_t>>
{
    if (!_enabled || size > MaxBlobSize)
    {
        return std::make_shared<std::vector<uint8_t>>(size);
    }

    const bool isRetained = (_blob != nullptr) && (_blob.use_count() != 1);
    if (_blob == nullptr || isRetained)
    {
        _blob = std::make_shared<std::vector<uint8_t>>(size);
        return _blob;
    }

    // NB: use_count() is a relaxed load. The fence orders the writes into the reused blob after
    //     the reads of whichever thread dropped the second-to-last reference to it.
    std::atomic_thread_fence(std::memory_order_acquire);

    // NB: only grow. Shrinking would value initialize bytes that the caller overwrites anyway,
    //     so the caller views only the part it fills.
    if (_blob->size() < size)
    {
        _blob->resize(size);
    }
    return _blob;
}

} // namespace Core
} // namespace SilKit
