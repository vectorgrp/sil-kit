// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "services/orchestration/DeliveryInbox.hpp"

#include <vector>

namespace SilKit {
namespace Services {
namespace Orchestration {

void DeliveryInbox::Schedule(std::chrono::nanoseconds dueTime, std::string senderParticipant,
                             std::string senderService, std::function<void()> deliver)
{
    {
        std::lock_guard<std::mutex> lock{_mutex};
        if (dueTime >= _releasedBefore)
        {
            _held.emplace(Key{dueTime, std::move(senderParticipant), std::move(senderService), _arrivals++},
                          std::move(deliver));
            return;
        }
        _lateDeliveries++;
    }
    deliver();
}

void DeliveryInbox::ReleaseBefore(std::chrono::nanoseconds until)
{
    std::vector<std::function<void()>> due;
    {
        std::lock_guard<std::mutex> lock{_mutex};
        const auto end = _held.lower_bound(Key{until, {}, {}, 0});
        for (auto it = _held.begin(); it != end; ++it)
        {
            due.push_back(std::move(it->second));
        }
        _held.erase(_held.begin(), end);
        _releasedBefore = std::max(_releasedBefore, until);
    }
    // Outside the lock: deliveries run user handlers
    for (auto& deliver : due)
    {
        deliver();
    }
}

auto DeliveryInbox::Held() const -> size_t
{
    std::lock_guard<std::mutex> lock{_mutex};
    return _held.size();
}

auto DeliveryInbox::LateDeliveries() const -> uint64_t
{
    std::lock_guard<std::mutex> lock{_mutex};
    return _lateDeliveries;
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
