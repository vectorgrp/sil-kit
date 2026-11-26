// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "services/orchestration/TimeCoordinator.hpp"

#include <algorithm>

using namespace std::chrono_literals;

namespace SilKit {
namespace Services {
namespace Orchestration {

namespace {
// Time point of a participant that has not reported its next simulation step yet.
constexpr std::chrono::nanoseconds unreportedTimePoint{-1};
} // namespace

auto TimeCoordinator::AddParticipant(const std::string& participantName) -> std::optional<GrantDispatch>
{
    Lock lock{_mx};
    if (_nextTasks.find(participantName) != _nextTasks.end())
    {
        return std::nullopt;
    }

    NextSimTask task;
    task.timePoint = unreportedTimePoint;
    task.duration = 0ns;
    _nextTasks.emplace(participantName, task);

    if (_currentGrant)
    {
        return GrantDispatch{*_currentGrant, {participantName}};
    }
    return std::nullopt;
}

auto TimeCoordinator::RemoveParticipant(const std::string& participantName) -> std::optional<GrantDispatch>
{
    Lock lock{_mx};
    if (_nextTasks.erase(participantName) == 0)
    {
        return std::nullopt;
    }
    return TryAdvance();
}

auto TimeCoordinator::UpdateNextSimTask(const std::string& participantName, const NextSimTask& task)
    -> std::optional<GrantDispatch>
{
    Lock lock{_mx};
    auto it = _nextTasks.find(participantName);
    if (it == _nextTasks.end())
    {
        return std::nullopt;
    }
    it->second = task;

    if (auto dispatch = TryAdvance())
    {
        return dispatch;
    }

    // No advance: the reporter may still be eligible under the current grant (late joiner, or a participant that
    // started following the time coordinator and resent its next step). Send it the current grant again.
    if (_currentGrant && EffectiveStart(task) <= _currentGrant->timePoint)
    {
        return GrantDispatch{*_currentGrant, {participantName}};
    }
    return std::nullopt;
}

auto TimeCoordinator::CurrentGrant() const -> std::optional<NextSimTask>
{
    Lock lock{_mx};
    return _currentGrant;
}

bool TimeCoordinator::HasParticipant(const std::string& participantName) const
{
    Lock lock{_mx};
    return _nextTasks.find(participantName) != _nextTasks.end();
}

auto TimeCoordinator::GetParticipantNames() const -> std::vector<std::string>
{
    Lock lock{_mx};
    std::vector<std::string> names;
    names.reserve(_nextTasks.size());
    for (const auto& entry : _nextTasks)
    {
        names.push_back(entry.first);
    }
    return names;
}

auto TimeCoordinator::Floor() const -> std::chrono::nanoseconds
{
    return _currentGrant ? _currentGrant->timePoint : unreportedTimePoint;
}

auto TimeCoordinator::EffectiveStart(const NextSimTask& task) const -> std::chrono::nanoseconds
{
    return std::max(task.timePoint, Floor());
}

auto TimeCoordinator::TryAdvance() -> std::optional<GrantDispatch>
{
    if (_nextTasks.empty())
    {
        return std::nullopt;
    }

    auto minTimePoint = std::chrono::nanoseconds::max();
    for (const auto& entry : _nextTasks)
    {
        minTimePoint = std::min(minTimePoint, EffectiveStart(entry.second));
    }

    // Nothing new to grant: either someone has not reported yet (initially) or the time did not advance.
    if (minTimePoint <= Floor())
    {
        return std::nullopt;
    }

    // Earliest step boundary (start or end of any participant's next step) after T_min. Participants advancing at
    // T_min use it to align their step when dynamic step sizes are enabled.
    auto boundary = std::chrono::nanoseconds::max();
    GrantDispatch dispatch;
    for (const auto& entry : _nextTasks)
    {
        const auto start = EffectiveStart(entry.second);
        const auto end = entry.second.timePoint + entry.second.duration;
        if (start == minTimePoint)
        {
            dispatch.recipients.push_back(entry.first);
        }
        if (start > minTimePoint)
        {
            boundary = std::min(boundary, start);
        }
        if (end > minTimePoint)
        {
            boundary = std::min(boundary, end);
        }
    }

    dispatch.grant.timePoint = minTimePoint;
    dispatch.grant.duration = (boundary == std::chrono::nanoseconds::max()) ? 0ns : boundary - minTimePoint;
    _currentGrant = dispatch.grant;
    return dispatch;
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
