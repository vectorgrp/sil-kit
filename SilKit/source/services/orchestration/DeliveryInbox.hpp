// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <tuple>

namespace SilKit {
namespace Services {
namespace Orchestration {

//! Holds messages until the simulation step that contains their due time starts, and then delivers them in a
//! deterministic order: by due time, then sender participant, then sender service, then arrival (messages of one
//! sender service arrive in the order they were sent).
class DeliveryInbox
{
public:
    void Schedule(std::chrono::nanoseconds dueTime, std::string senderParticipant, std::string senderService,
                  std::function<void()> deliver);

    //! Delivers everything due before `until`. Deliveries scheduled afterwards that are due before `until` arrived
    //! too late for their step; they are delivered right away and counted.
    void ReleaseBefore(std::chrono::nanoseconds until);

    auto Held() const -> size_t;
    auto LateDeliveries() const -> uint64_t;

private:
    struct Key
    {
        std::chrono::nanoseconds dueTime;
        std::string senderParticipant;
        std::string senderService;
        uint64_t arrival;

        bool operator<(const Key& other) const
        {
            return std::tie(dueTime, senderParticipant, senderService, arrival)
                   < std::tie(other.dueTime, other.senderParticipant, other.senderService, other.arrival);
        }
    };

    mutable std::mutex _mutex;
    std::map<Key, std::function<void()>> _held;
    uint64_t _arrivals{0};
    uint64_t _lateDeliveries{0};
    std::chrono::nanoseconds _releasedBefore{std::chrono::nanoseconds::min()};
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
