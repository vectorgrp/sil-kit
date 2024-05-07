// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "SystemStateTracker.hpp"

#include "silkit/participant/exception.hpp"
#include "silkit/services/orchestration/string_utils.hpp"

#include "ILogger.hpp"

#include <fmt/format.h>


namespace Log = SilKit::Services::Logging;


namespace {

using SilKit::Services::Orchestration::ParticipantState;
using SilKit::Services::Orchestration::SystemState;

auto ValidateParticipantStateUpdate(ParticipantState oldParticipantState, ParticipantState newParticipantState) -> bool
{
    auto OldParticipantStateWas = [oldParticipantState](std::initializer_list<ParticipantState> stateList) {
        return std::any_of(begin(stateList), end(stateList), [oldParticipantState](auto candidate) {
            return candidate == oldParticipantState;
        });
    };

    switch (newParticipantState)
    {
    case ParticipantState::ServicesCreated:
        return OldParticipantStateWas({ParticipantState::Invalid});

    case ParticipantState::CommunicationInitializing:
        return OldParticipantStateWas({ParticipantState::ServicesCreated});

    case ParticipantState::CommunicationInitialized:
        return OldParticipantStateWas({ParticipantState::CommunicationInitializing});

    case ParticipantState::ReadyToRun:
        return OldParticipantStateWas({ParticipantState::CommunicationInitialized});

    case ParticipantState::Running:
        return OldParticipantStateWas({ParticipantState::ReadyToRun, ParticipantState::Paused});

    case ParticipantState::Paused:
        return OldParticipantStateWas({ParticipantState::Running});

    case ParticipantState::Stopping:
        return OldParticipantStateWas({ParticipantState::Running, ParticipantState::Paused});

    case ParticipantState::Stopped:
        return OldParticipantStateWas({ParticipantState::Stopping});

    case ParticipantState::ShuttingDown:
        return OldParticipantStateWas({ParticipantState::Error, ParticipantState::Stopped});

    case ParticipantState::Shutdown:
        return OldParticipantStateWas({ParticipantState::ShuttingDown, ParticipantState::Aborting});

    case ParticipantState::Aborting:
    case ParticipantState::Error:
        return true;

    default:
        throw SilKit::AssertionError(fmt::format(
            "SystemStateTracker::ValidateParticipantStateUpdate: unhandled ParticipantState::{}", newParticipantState));
    }
}

auto FormatTimePoint(std::chrono::system_clock::time_point timePoint) -> std::string
{
    std::time_t enterTime = std::chrono::system_clock::to_time_t(timePoint);
    std::tm tmBuffer;

#if defined(_WIN32)
    localtime_s(&tmBuffer, &enterTime);
#else
    localtime_r(&enterTime, &tmBuffer);
#endif

    std::stringstream timeBuf;
    timeBuf << std::put_time(&tmBuffer, "%FT%T");

    return timeBuf.str();
}

} // namespace


namespace VSilKit {


void SystemStateTracker::SetLogger(SilKit::Services::Logging::ILogger* logger)
{
    _logger = logger;
}

auto SystemStateTracker::IsEmpty() const -> bool
{
    std::lock_guard<decltype(_mutex)> lock{_mutex};
    return _participantStatusCache.empty();
}

auto SystemStateTracker::UpdateRequiredParticipants(SilKit::Util::Span<const std::string> requiredParticipantNames)
    -> UpdateRequiredParticipantsResult
{
    std::lock_guard<decltype(_mutex)> lock{_mutex};

    // Update the stored list of required participants

    _requiredParticipants.clear();
    _requiredParticipants.insert(requiredParticipantNames.begin(), requiredParticipantNames.end());

    // recompute the system state

    UpdateRequiredParticipantsResult result;

    const auto oldSystemState{_systemState};
    const auto newSystemState{ComputeSystemState(GetAnyRequiredParticipantState())};

    if (oldSystemState != newSystemState)
    {
        _systemState = newSystemState;
        result.systemStateChanged = true;
    }

    return result;
}

auto SystemStateTracker::UpdateParticipantStatus(const ParticipantStatus& newParticipantStatus)
    -> UpdateParticipantStatusResult
{
    std::lock_guard<decltype(_mutex)> lock{_mutex};

    const auto& participantName{newParticipantStatus.participantName};
    const auto& participantStatus{GetOrCreateParticipantStatus(participantName)};

    const auto oldParticipantState{participantStatus.state};
    const auto newParticipantState{newParticipantStatus.state};

    Log::Debug(_logger, "Updating participant status for {} from {} to {}", participantName, oldParticipantState,
               newParticipantState);

    // Check if transition from the old to the new participant state is valid

    if (!ValidateParticipantStateUpdate(oldParticipantState, newParticipantState))
    {
        const auto logLevel = IsRequiredParticipant(participantName) ? Log::Level::Warn : Log::Level::Debug;

        Log::Log(
            _logger, logLevel,
            "SystemMonitor detected invalid ParticipantState transition for {} from {} to {} EnterTime={}, EnterReason=\"{}\"",
            participantName, oldParticipantState, newParticipantState, FormatTimePoint(newParticipantStatus.enterTime),
            newParticipantStatus.enterReason);

        // NB: Failing validation doesn't actually stop the participants state from being changed, it just logs the
        //     invalid transition
    }

    // Ignores transition if ParticipantState is Shutdown already

    if (oldParticipantState == ParticipantState::Shutdown)
    {
        return UpdateParticipantStatusResult{};
    }

    // Update the stored participant status and recompute the system state if required

    SetParticipantStatus(participantName, newParticipantStatus);

    UpdateParticipantStatusResult result;

    if (oldParticipantState != newParticipantState)
    {
        result.participantStateChanged = true;

        Log::Debug(_logger, "The participant state has changed for {}", participantName);

        if (IsRequiredParticipant(participantName))
        {
            const auto oldSystemState{_systemState};
            const auto newSystemState{ComputeSystemState(newParticipantState)};

            Log::Debug(_logger, "Computed new system state update from {} to {}", oldSystemState, newSystemState);

            if (oldSystemState != newSystemState)
            {
                Log::Debug(_logger, "The system state has changed from {} to {}", oldSystemState, newSystemState);

                _systemState = newSystemState;
                result.systemStateChanged = true;
            }
        }
    }

    return result;
}

auto SystemStateTracker::RemoveParticipant(const std::string& participantName) -> RemoveParticipantResult
{
    std::lock_guard<decltype(_mutex)> lock{_mutex};

    RemoveParticipantResult result;

    const auto participantStatusIt{_participantStatusCache.find(participantName)};

    if (participantStatusIt != _participantStatusCache.end())
    {
        _participantStatusCache.erase(participantStatusIt);

        const auto oldSystemState{_systemState};
        const auto newSystemState{ComputeSystemState(GetAnyRequiredParticipantState())};

        if (oldSystemState != newSystemState)
        {
            _systemState = newSystemState;
            result.systemStateChanged = true;
        }
    }

    return result;
}

auto SystemStateTracker::IsRequiredParticipant(const std::string& participantName) const -> bool
{
    std::lock_guard<decltype(_mutex)> lock{_mutex};
    return _requiredParticipants.find(participantName) != _requiredParticipants.end();
}

auto SystemStateTracker::GetParticipantStatus(const std::string& participantName) const -> const ParticipantStatus*
{
    std::lock_guard<decltype(_mutex)> lock{_mutex};

    const auto it{_participantStatusCache.find(participantName)};
    if (it == _participantStatusCache.end())
    {
        return nullptr;
    }

    return &it->second;
}

auto SystemStateTracker::GetSystemState() const -> SystemState
{
    std::lock_guard<decltype(_mutex)> lock{_mutex};
    return _systemState;
}

auto SystemStateTracker::GetOrCreateParticipantStatus(const std::string& participantName) -> const ParticipantStatus&
{
    std::lock_guard<decltype(_mutex)> lock{_mutex};

    auto it{_participantStatusCache.find(participantName)};

    if (it == _participantStatusCache.end())
    {
        SilKit::Services::Orchestration::ParticipantStatus participantStatus{};
        participantStatus.participantName = participantName;
        participantStatus.state = SilKit::Services::Orchestration::ParticipantState::Invalid;

        it = _participantStatusCache.emplace(participantName, std::move(participantStatus)).first;
    }

    return it->second;
}

void SystemStateTracker::SetParticipantStatus(const std::string& participantName,
                                              const ParticipantStatus& participantStatus)
{
    std::lock_guard<decltype(_mutex)> lock{_mutex};
    _participantStatusCache[participantName] = participantStatus;
}

auto SystemStateTracker::GetAnyRequiredParticipantState() const -> ParticipantState
{
    if (!_requiredParticipants.empty())
    {
        const auto& requiredParticipantName{*_requiredParticipants.begin()};
        const auto* const requiredParticipantStatus{GetParticipantStatus(requiredParticipantName)};
        if (requiredParticipantStatus != nullptr)
        {
            return requiredParticipantStatus->state;
        }
    }

    return ParticipantState::Invalid;
}

auto SystemStateTracker::ComputeSystemState(ParticipantState newParticipantState) const -> SystemState
{
    std::lock_guard<decltype(_mutex)> lock{_mutex};

    const auto oldSystemState = _systemState;
    SystemState newSystemState{oldSystemState};

    auto ChangeToIfAllIn = [this, &newSystemState](SystemState systemState,
                                                   std::initializer_list<ParticipantState> stateList) {
        for (const auto& requiredParticipantName : _requiredParticipants)
        {
            const auto* const requiredParticipantStatus{GetParticipantStatus(requiredParticipantName)};
            if (requiredParticipantStatus == nullptr)
            {
                return false;
            }

            const auto requiredParticipantState{requiredParticipantStatus->state};

            const bool accepted =
                std::any_of(begin(stateList), end(stateList), [requiredParticipantState](auto candidate) {
                    return candidate == requiredParticipantState;
                });

            if (!accepted)
            {
                return false;
            }
        }

        newSystemState = systemState;
        return true;
    };

    if (IsEmpty())
    {
        switch (oldSystemState)
        {
        case SystemState::Error:
        case SystemState::Shutdown:
            // The old system state will be kept
            break;
        case SystemState::ShuttingDown:
            // If all participants have disconnected, and the system state was still ShuttingDown, not all
            // participants have reported their Shutdown state.
            // TODO: find out if this is fine
            // TODO: this case exists, because otherwise the registry would have to implement the
            //       request-reply services 'barrier', which I don't want to implement atm
            newSystemState = SystemState::Shutdown;
            break;
        default:
            // If the system state was anything else, the system state is now invalid (since a participant
            // has disconnected unexpectedly).
            newSystemState = SystemState::Invalid;
            break;
        }
    }

    switch (newParticipantState)
    {
        using PS = ParticipantState;
        using SS = SystemState;
    case PS::Invalid:
        break;
    case PS::ServicesCreated:
        ChangeToIfAllIn(SS::ServicesCreated, {PS::ServicesCreated, PS::CommunicationInitializing,
                                              PS::CommunicationInitialized, PS::ReadyToRun, PS::Running});
        break;
    case PS::CommunicationInitializing:
        ChangeToIfAllIn(SS::CommunicationInitializing,
                        {PS::CommunicationInitializing, PS::CommunicationInitialized, PS::ReadyToRun, PS::Running});
        break;
    case PS::CommunicationInitialized:
        ChangeToIfAllIn(SS::CommunicationInitialized, {PS::CommunicationInitialized, PS::ReadyToRun, PS::Running});
        break;
    case PS::ReadyToRun:
        ChangeToIfAllIn(SS::ReadyToRun, {PS::ReadyToRun, PS::Running});
        break;
    case PS::Running:
        if (ChangeToIfAllIn(SS::Running, {PS::Running}))
            break;
        if (ChangeToIfAllIn(SS::Stopping, {PS::Running, PS::Stopping}))
            break;
        if (ChangeToIfAllIn(SS::Stopped, {PS::Running, PS::Stopped}))
            break;
        if (ChangeToIfAllIn(SS::ShuttingDown, {PS::Running, PS::Stopped, PS::ShuttingDown}))
            break;
        if (ChangeToIfAllIn(SS::Shutdown, {PS::Running, PS::Stopped, PS::ShuttingDown, PS::Shutdown}))
            break;
        break;
    case PS::Paused:
        if (ChangeToIfAllIn(SS::Paused, {PS::Running, PS::Paused}))
            break;
        if (ChangeToIfAllIn(SS::Stopping, {PS::Running, PS::Paused, PS::Stopping}))
            break;
        if (ChangeToIfAllIn(SS::Stopped, {PS::Running, PS::Paused, PS::Stopped}))
            break;
        if (ChangeToIfAllIn(SS::ShuttingDown, {PS::Running, PS::Paused, PS::Stopped, PS::ShuttingDown}))
            break;
        if (ChangeToIfAllIn(SS::Shutdown, {PS::Running, PS::Paused, PS::Stopped, PS::ShuttingDown, PS::Shutdown}))
            break;
        break;
    case PS::Stopping:
        ChangeToIfAllIn(SS::Stopping,
                        {PS::Running, PS::Paused, PS::Stopping, PS::Stopped, PS::ShuttingDown, PS::Shutdown});
        break;
    case PS::Stopped:
        ChangeToIfAllIn(SS::Stopped, {PS::Stopped, PS::ShuttingDown, PS::Shutdown});
        break;
    case PS::ShuttingDown:
        // TODO: figure out, why the direct transition from ServicesCreated and ReadyToRun is fine, but
        //       CommunicationInitializing / CommunicationInitialized is not
        // TODO: figure out, if ShuttingDown is a dominant state or not (since it immediately transitions while
        //       Stopped participants are present)
        ChangeToIfAllIn(SS::ShuttingDown,
                        {PS::Stopped, PS::ShuttingDown, PS::Shutdown, PS::Error, PS::ServicesCreated, PS::ReadyToRun});
        break;
    case PS::Shutdown:
        ChangeToIfAllIn(SS::Shutdown, {PS::Shutdown});
        break;
    case PS::Aborting:
        newSystemState = SS::Aborting;
        break;
    case PS::Error:
        newSystemState = SS::Error;
        break;
    default:
        Log::Debug(_logger, "Unhandled participant state {}", newParticipantState);
        break;
    }

    return newSystemState;
}


} // namespace VSilKit
