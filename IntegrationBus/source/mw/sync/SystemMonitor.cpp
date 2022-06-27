// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SystemMonitor.hpp"
#include "IServiceDiscovery.hpp"
#include "LifecycleService.hpp"
#include "TimeSyncService.hpp"

#include <algorithm>
#include <ctime>
#include <iomanip> //std:put_time

#include "ib/mw/logging/ILogger.hpp"
#include "ib/mw/sync/string_utils.hpp"

namespace ib {
namespace mw {
namespace sync {

SystemMonitor::SystemMonitor(IParticipantInternal* participant)
    : _logger{participant->GetLogger()}
{
}

void SystemMonitor::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/,
                                     const sync::WorkflowConfiguration& workflowConfiguration)
{
    UpdateRequiredParticipantNames(workflowConfiguration.requiredParticipantNames);
}

void SystemMonitor::UpdateRequiredParticipantNames(const std::vector<std::string>& requiredParticipantNames)
{
    // Prevent calling this method more than once
    if (!_requiredParticipantNames.empty())
    {
        throw std::runtime_error{"Expected participant names are already set."};
    }

    _requiredParticipantNames = requiredParticipantNames;

    bool allRequiredParticipantsKnown = true;
    // Init new participants in status map
    for (auto&& name : _requiredParticipantNames)
    {
        auto&& statusIter = _participantStatus.find(name);
        if (statusIter == _participantStatus.end())
        {
            _participantStatus[name] = sync::ParticipantStatus{};
            _participantStatus[name].state = sync::ParticipantState::Invalid;
            allRequiredParticipantsKnown = false;
        }
    }

    // Update / propagate the system state in case status updated for all required participants have been received already
    if (allRequiredParticipantsKnown)
    {
        auto oldSystemState = _systemState;
        for (auto&& name : _requiredParticipantNames)
        {
            UpdateSystemState(_participantStatus[name]);
        }
        if (oldSystemState != _systemState)
        {
            _systemStateHandlers.InvokeAll(_systemState);
        }
    }
}
auto SystemMonitor::AddSystemStateHandler(SystemStateHandlerT handler) -> HandlerId
{
    if (_systemState != sync::SystemState::Invalid)
    {
        handler(_systemState);
    }

    return _systemStateHandlers.Add(std::move(handler));
}

void SystemMonitor::RemoveSystemStateHandler(HandlerId handlerId)
{
    if (!_systemStateHandlers.Remove(handlerId))
    {
        _logger->Warn("RemoveSystemStateHandler failed: Unknown HandlerId.");
    }

}

auto SystemMonitor::AddParticipantStatusHandler(ParticipantStatusHandlerT handler) -> HandlerId
{
    for (auto&& kv : _participantStatus)
    {
        auto&& participantStatus = kv.second;
        if (participantStatus.state == sync::ParticipantState::Invalid)
            continue;

        handler(participantStatus);
    }

    return _participantStatusHandlers.Add(std::move(handler));
}

void SystemMonitor::RemoveParticipantStatusHandler(HandlerId handlerId)
{
    if (!_participantStatusHandlers.Remove(handlerId))
    {
        _logger->Warn("RemoveParticipantStatusHandler failed: Unknown HandlerId.");
    }
}

auto SystemMonitor::SystemState() const -> sync::SystemState
{
    return _systemState;
}

auto SystemMonitor::ParticipantStatus(const std::string& participantName) const -> const sync::ParticipantStatus&
{
    auto&& statusIter = _participantStatus.find(participantName);
    if (statusIter == _participantStatus.end())
    {
        throw std::runtime_error{"Unknown participantName"};
    }

    return statusIter->second;
}

void SystemMonitor::ReceiveIbMessage(const IIbServiceEndpoint* /*from*/, const sync::ParticipantStatus& newParticipantStatus)
{
    auto participantName = newParticipantStatus.participantName;

    // Validation
    auto oldParticipantState = _participantStatus[participantName].state;
    ValidateParticipantStatusUpdate(newParticipantStatus, oldParticipantState);
    if (oldParticipantState == sync::ParticipantState::Shutdown)
    {
        _logger->Debug("Ignoring ParticipantState update from participant {} to ParticipantState::{} because "
                       "participant is already in terminal state ParticipantState::Shutdown.",
                       newParticipantStatus.participantName, newParticipantStatus.state);
        return;
    }

    // Update status map
    _participantStatus[participantName] = newParticipantStatus;

    // On new participant state
    if (oldParticipantState != newParticipantStatus.state)
    {
        // Fire status / state handlers
        _participantStatusHandlers.InvokeAll(newParticipantStatus);

        auto oldSystemState = _systemState;
        // Update the system state for required participants, others are ignored
        UpdateSystemState(newParticipantStatus);
        if (oldSystemState != _systemState)
        {
            _systemStateHandlers.InvokeAll(_systemState);
        }
    }
}

void SystemMonitor::SetParticipantConnectedHandler(ParticipantConnectedHandler handler)
{
    _participantConnectedHandler = std::move(handler);
}

void SystemMonitor::SetParticipantDisconnectedHandler(ParticipantDisconnectedHandler handler)
{
    _participantDisconnectedHandler = std::move(handler);
}

auto SystemMonitor::IsParticipantConnected(const std::string& participantName) const -> bool
{
    const auto it = _connectedParticipantNames.find(participantName);
    return it != _connectedParticipantNames.end();
}

void SystemMonitor::OnParticipantConnected(const std::string& participantName)
{
    // Add the participant name to the set of connected participant names
    _connectedParticipantNames.emplace(participantName);
    // Call the handler if set
    if (_participantConnectedHandler)
    {
        _participantConnectedHandler(participantName);
    }
}

void SystemMonitor::OnParticipantDisconnected(const std::string& participantName)
{
    // Remove the participant name from the set of connected participant names
    auto it = _connectedParticipantNames.find(participantName);
    if (it != _connectedParticipantNames.end())
    {
        _connectedParticipantNames.erase(it);
    }
    // Call the handler if set
    if (_participantDisconnectedHandler)
    {
        _participantDisconnectedHandler(participantName);
    }
}

bool SystemMonitor::AllRequiredParticipantsInState(std::initializer_list<sync::ParticipantState> acceptedStates) const
{
    for (auto&& name : _requiredParticipantNames)
    {
        bool isAcceptedState = std::any_of(begin(acceptedStates), end(acceptedStates), [participantState = _participantStatus.at(name).state](auto acceptedState) {
            return participantState == acceptedState;
        });
        if (!isAcceptedState)
            return false;
    }
    return true;
}

void SystemMonitor::ValidateParticipantStatusUpdate(const sync::ParticipantStatus& newStatus, sync::ParticipantState oldState)
{
    auto is_any_of = [](sync::ParticipantState state, std::initializer_list<sync::ParticipantState> stateList)
    {
        return std::any_of(begin(stateList), end(stateList), [=](auto candidate) { return candidate == state; });
    };

    switch (newStatus.state)
    {
    case sync::ParticipantState::ServicesCreated:
        if (is_any_of(oldState, {sync::ParticipantState::Invalid, sync::ParticipantState::Reinitializing}))
            return;

    case sync::ParticipantState::CommunicationInitializing:
        if (is_any_of(oldState, {sync::ParticipantState::ServicesCreated}))
            return;

    case sync::ParticipantState::CommunicationInitialized:
        if (oldState == sync::ParticipantState::CommunicationInitializing)
            return;

    case sync::ParticipantState::ReadyToRun:
        if (oldState == sync::ParticipantState::CommunicationInitialized)
            return;

    case sync::ParticipantState::Running:
        if (is_any_of(oldState, {sync::ParticipantState::ReadyToRun, sync::ParticipantState::Paused}))
            return;

    case sync::ParticipantState::Paused:
        if (oldState == sync::ParticipantState::Running)
            return;

    case sync::ParticipantState::Stopping:
        if (is_any_of(oldState, {sync::ParticipantState::Running, sync::ParticipantState::Paused}))
            return;

    case sync::ParticipantState::Stopped:
        if (oldState == sync::ParticipantState::Stopping)
            return;

    case sync::ParticipantState::ShuttingDown:
        if (is_any_of(oldState, {sync::ParticipantState::Error, sync::ParticipantState::Stopped}))
            return;

    case sync::ParticipantState::Shutdown:
        if (oldState == sync::ParticipantState::ShuttingDown)
            return;

    case sync::ParticipantState::Reinitializing:
        if (is_any_of(oldState, {sync::ParticipantState::Error, sync::ParticipantState::Stopped}))
            return;

    case sync::ParticipantState::Error:
        return;

    default:
        _logger->Error("SystemMonitor::ValidateParticipantStatusUpdate() Unhandled ParticipantState::{}", newStatus.state);
    }

    std::time_t enterTime = std::chrono::system_clock::to_time_t(newStatus.enterTime);
    std::tm tmBuffer;
#if defined(_WIN32)
    localtime_s(&tmBuffer, &enterTime);
#else
    localtime_r(&enterTime, &tmBuffer);
#endif
    std::stringstream timeBuf;
    timeBuf <<  std::put_time(&tmBuffer, "%FT%T");

    _logger->Error(
        "SystemMonitor detected invalid ParticipantState transition from {} to {} EnterTime={}, EnterReason=\"{}\"",
        oldState,
        newStatus.state,
        timeBuf.str(),
        newStatus.enterReason);

    _invalidTransitionCount++;
}

void SystemMonitor::UpdateSystemState(const sync::ParticipantStatus& newStatus)
{
    auto&& nameIter = std::find(_requiredParticipantNames.begin(), _requiredParticipantNames.end(), newStatus.participantName);
    if (nameIter == _requiredParticipantNames.end())
    {
        return;
    }

    switch (newStatus.state)
    {
    case sync::ParticipantState::ServicesCreated:
        if (AllRequiredParticipantsInState({sync::ParticipantState::ServicesCreated,
                                            sync::ParticipantState::CommunicationInitializing,
                                            sync::ParticipantState::CommunicationInitialized,
                                            sync::ParticipantState::ReadyToRun, 
                                            sync::ParticipantState::Running}))
        {
            SetSystemState(sync::SystemState::ServicesCreated);
        }
        return;

    case sync::ParticipantState::CommunicationInitializing:
        if (AllRequiredParticipantsInState({sync::ParticipantState::CommunicationInitializing,
                                            sync::ParticipantState::CommunicationInitialized,
                                            sync::ParticipantState::ReadyToRun, 
                                            sync::ParticipantState::Running}))
        {
            SetSystemState(sync::SystemState::CommunicationInitializing);
        }
        return;

    case sync::ParticipantState::CommunicationInitialized:
        if (AllRequiredParticipantsInState({sync::ParticipantState::CommunicationInitialized,
                                            sync::ParticipantState::ReadyToRun, 
                                            sync::ParticipantState::Running}))
        {
            SetSystemState(sync::SystemState::CommunicationInitialized);
        }
        return;

    case sync::ParticipantState::ReadyToRun:
        if (AllRequiredParticipantsInState({sync::ParticipantState::ReadyToRun, 
                                            sync::ParticipantState::Running}))
        {
            SetSystemState(sync::SystemState::ReadyToRun);
        }
        return;

    case sync::ParticipantState::Running:
        if (AllRequiredParticipantsInState({sync::ParticipantState::Running}))
        {
            SetSystemState(sync::SystemState::Running);
        }
        return;

    case sync::ParticipantState::Paused:
        if (AllRequiredParticipantsInState({sync::ParticipantState::Paused, 
                                            sync::ParticipantState::Running}))
            SetSystemState(sync::SystemState::Paused);
        return;

    case sync::ParticipantState::Stopping:
        if (AllRequiredParticipantsInState({sync::ParticipantState::Stopping, 
                                            sync::ParticipantState::Stopped,
                                            sync::ParticipantState::Paused, 
                                            sync::ParticipantState::Running}))
            SetSystemState(sync::SystemState::Stopping);
        return;

    case sync::ParticipantState::Stopped:
        if (AllRequiredParticipantsInState({sync::ParticipantState::Stopped}))
            SetSystemState(sync::SystemState::Stopped);
        return;

    case sync::ParticipantState::ShuttingDown:
        if (AllRequiredParticipantsInState({sync::ParticipantState::ShuttingDown, 
                                            sync::ParticipantState::Shutdown,
                                            sync::ParticipantState::Stopped, 
                                            sync::ParticipantState::Error,
                                            sync::ParticipantState::ServicesCreated, 
                                            sync::ParticipantState::ReadyToRun}))
            SetSystemState(sync::SystemState::ShuttingDown);
        return;

    case sync::ParticipantState::Shutdown:
        if (AllRequiredParticipantsInState({sync::ParticipantState::Shutdown}))
            SetSystemState(sync::SystemState::Shutdown);
        return;

    case sync::ParticipantState::Error:
        SetSystemState(sync::SystemState::Error);
        return;

    default:
        return;
    }
}

void SystemMonitor::SetSystemState(sync::SystemState newState)
{
    _systemState = newState;
}


} // namespace sync
} // namespace mw
} // namespace ib
