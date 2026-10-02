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

    // NB: prefer the smallest free blob that holds the message within the slack, so that a mix of
    //     message sizes settles into blobs of matching sizes instead of reallocating back and forth.
    std::shared_ptr<std::vector<uint8_t>>* fitting{nullptr};
    std::shared_ptr<std::vector<uint8_t>>* tooSmall{nullptr};
    std::shared_ptr<std::vector<uint8_t>>* tooLarge{nullptr};

    for (auto& entry : _entries)
    {
        if (entry.use_count() != 1)
        {
            continue;
        }

        const auto entrySize = entry->size();
        if (entrySize < size)
        {
            if (tooSmall == nullptr || entrySize > (*tooSmall)->size())
            {
                tooSmall = &entry;
            }
        }
        else if (entrySize - size > MaxSlack)
        {
            tooLarge = &entry;
        }
        else if (fitting == nullptr || entrySize < (*fitting)->size())
        {
            fitting = &entry;
        }
    }

    auto* reused = (fitting != nullptr) ? fitting : tooSmall;
    if (reused != nullptr)
    {
        // NB: use_count() is a relaxed load. The fence orders the writes into the reused blob after
        //     the reads of whichever thread dropped the second-to-last reference to it.
        std::atomic_thread_fence(std::memory_order_acquire);

        // NB: only grow. Shrinking would value initialize bytes that the caller overwrites anyway,
        //     so the caller views only the part it fills.
        if ((*reused)->size() < size)
        {
            (*reused)->resize(size);
        }
        return *reused;
    }

    auto blob = std::make_shared<std::vector<uint8_t>>(size);
    if (_entries.size() < MaxEntries)
    {
        _entries.push_back(blob);
    }
    else if (tooLarge != nullptr)
    {
        // Drop an oversized blob in favour of one that matches the current traffic.
        *tooLarge = blob;
    }

    return blob;
}

} // namespace Core
} // namespace SilKit
