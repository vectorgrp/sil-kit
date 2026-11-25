// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "core/internal/OrchestrationDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

//! \brief Central time coordination: collects the NextSimTask of every synchronized participant and grants time
//!        advances.
//!
//! The grant is a NextSimTask with timePoint = T_min (the minimum next time point of all participants) and
//! duration = (earliest step start or end strictly after T_min) - T_min. A participant may advance iff its next time
//! point equals T_min, which is the same rule as the distributed algorithm (own next <= all others' next). The
//! duration makes the dynamic step size alignment work unchanged on the receiving side.
//!
//! Grants are only issued when T_min increases, and only to the participants that become eligible. This makes the
//! traffic O(N) per simulation step instead of O(N^2).
//!
//! Late joiners: the last granted time point is a floor. Entries below the floor (a participant that has not yet
//! reported, or one that reports its initial time 0 while the simulation already advanced) are clamped to the floor,
//! so the virtual time never goes backwards and the joiner holds the others until it reports its real next step.
//!
//! This class is pure logic (thread-safe, no I/O). The caller sends the returned grants.
class TimeCoordinator
{
public:
    //! A grant and the participants it must be sent to.
    struct GrantDispatch
    {
        NextSimTask grant;
        std::vector<std::string> recipients;
    };

public:
    //! Adds a participant. Until it reports, it blocks the time at the current floor.
    //! Returns the current grant addressed to the new participant if the time has already been granted.
    auto AddParticipant(const std::string& participantName) -> std::optional<GrantDispatch>;

    //! Removes a participant. Returns a new grant if the removal let the time advance.
    auto RemoveParticipant(const std::string& participantName) -> std::optional<GrantDispatch>;

    //! Updates the next simulation step of a known participant (unknown participants are ignored).
    //! Returns a new grant if the time advanced, or the current grant addressed to the reporter only if the reporter
    //! is already eligible under it (e.g., a late joiner or a duplicate report).
    auto UpdateNextSimTask(const std::string& participantName, const NextSimTask& task)
        -> std::optional<GrantDispatch>;

    //! The last issued grant, if any.
    auto CurrentGrant() const -> std::optional<NextSimTask>;

    bool HasParticipant(const std::string& participantName) const;
    auto GetParticipantNames() const -> std::vector<std::string>;

private:
    using Lock = std::unique_lock<std::mutex>;

    auto Floor() const -> std::chrono::nanoseconds;
    auto EffectiveStart(const NextSimTask& task) const -> std::chrono::nanoseconds;
    //! Computes a new grant if T_min advanced beyond the floor. Must be called with _mx held.
    auto TryAdvance() -> std::optional<GrantDispatch>;

private:
    mutable std::mutex _mx;
    std::map<std::string, NextSimTask> _nextTasks;
    std::optional<NextSimTask> _currentGrant;
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
