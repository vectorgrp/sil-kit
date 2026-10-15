// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <functional>
#include <string>

namespace SilKit {
namespace Services {
namespace Orchestration {

//! Latency-based delivery ("exact mode") of the messages a participant receives from other synchronized
//! participants: a message sent at t on a link with latency L is due at t + L and is delivered at the start of the
//! receiver's simulation step that contains t + L.
class IDeliveryScheduler
{
public:
    virtual ~IDeliveryScheduler() = default;

    //! Latency of the messages received on the link, or zero to deliver them on receipt.
    virtual auto LatencyOf(const std::string& linkName) const -> std::chrono::nanoseconds = 0;

    //! Holds a delivery until the receiver's step that contains dueTime starts. Deliveries are ordered by
    //! (dueTime, sender participant, sender service, arrival). sendTime tells the scheduler that the sender will
    //! not send anything stamped earlier.
    virtual void Schedule(std::chrono::nanoseconds sendTime, std::chrono::nanoseconds dueTime,
                          const std::string& senderParticipant, const std::string& senderService,
                          std::function<void()> deliver) = 0;
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
