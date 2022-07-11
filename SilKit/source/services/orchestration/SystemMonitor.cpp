// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <algorithm>
#include <ctime>
#include <iomanip> //std:put_time

#include "silkit/services/logging/ILogger.hpp"
#include "silkit/services/orchestration/string_utils.hpp"

#include "SystemMonitor.hpp"
#include "IServiceDiscovery.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

SystemMonitor::SystemMonitor(Core::IParticipantInternal* participant)
    : _logger{participant->GetLogger()}
{
}

void SystemMonitor::ReceiveMsg(const IServiceEndpoint* /*from*/,
                                     const Orchestration::WorkflowConfiguration& workflowConfiguration)
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
            _participantStatus[name] = Orchestration::ParticipantStatus{};
            _participantStatus[name].state = Orchestration::ParticipantState::Invalid;
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
    if (_systemState != Orchestration::SystemState::Invalid)
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
        if (participantStatus.state == Orchestration::ParticipantState::Invalid)
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

auto SystemMonitor::SystemState() const -> Orchestration::SystemState
{
    return _systemState;
}

auto SystemMonitor::ParticipantStatus(const std::string& participantName) const -> const Orchestration::ParticipantStatus&
{
    auto&& statusIter = _participantStatus.find(participantName);
    if (statusIter == _participantStatus.end())
    {
        throw std::runtime_error{"Unknown participantName"};
    }

    return statusIter->second;
}

void SystemMonitor::ReceiveMsg(const IServiceEndpoint* /*from*/, const Orchestration::ParticipantStatus& newParticipantStatus)
{
    auto participantName = newParticipantStatus.participantName;

    // Validation
    auto oldParticipantState = _participantStatus[participantName].state;
    ValidateParticipantStatusUpdate(newParticipantStatus, oldParticipantState);
    if (oldParticipantState == Orchestration::ParticipantState::Shutdown)
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
    const auto it = _connectedParticipants.find(participantName);
    return it != _connectedParticipants.end();
}

void SystemMonitor::OnParticipantConnected(const ParticipantConnectionInformation& participantConnectionInformation)
{
    // Add the participant name to the map of connected participant names/connections
    _connectedParticipants.emplace(participantConnectionInformation.participantName, participantConnectionInformation);
    // Call the handler if set
    if (_participantConnectedHandler)
    {
        _participantConnectedHandler(participantConnectionInformation);
    }
}

void SystemMonitor::OnParticipantDisconnected(const ParticipantConnectionInformation& participantConnectionInformation)
{
    // Remove the participant name from the map of connected participant names/connections
    auto it = _connectedParticipants.find(participantConnectionInformation.participantName);
    if (it != _connectedParticipants.end())
    {
        _connectedParticipants.erase(it);
    }
    // Call the handler if set
    if (_participantDisconnectedHandler)
    {
        _participantDisconnectedHandler(participantConnectionInformation);
    }
}

bool SystemMonitor::AllRequiredParticipantsInState(std::initializer_list<Orchestration::ParticipantState> acceptedStates) const
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

void SystemMonitor::ValidateParticipantStatusUpdate(const Orchestration::ParticipantStatus& newStatus, Orchestration::ParticipantState oldState)
{
    auto is_any_of = [](Orchestration::ParticipantState state, std::initializer_list<Orchestration::ParticipantState> stateList)
    {
        return std::any_of(begin(stateList), end(stateList), [=](auto candidate) { return candidate == state; });
    };

    switch (newStatus.state)
    {
    case Orchestration::ParticipantState::ServicesCreated:
        if (is_any_of(oldState, {Orchestration::ParticipantState::Invalid, Orchestration::ParticipantState::Reinitializing}))
            return;

    case Orchestration::ParticipantState::CommunicationInitializing:
        if (is_any_of(oldState, {Orchestration::ParticipantState::ServicesCreated}))
            return;

    case Orchestration::ParticipantState::CommunicationInitialized:
        if (oldState == Orchestration::ParticipantState::CommunicationInitializing)
            return;

    case Orchestration::ParticipantState::ReadyToRun:
        if (oldState == Orchestration::ParticipantState::CommunicationInitialized)
            return;

    case Orchestration::ParticipantState::Running:
        if (is_any_of(oldState, {Orchestration::ParticipantState::ReadyToRun, Orchestration::ParticipantState::Paused}))
            return;

    case Orchestration::ParticipantState::Paused:
        if (oldState == Orchestration::ParticipantState::Running)
            return;

    case Orchestration::ParticipantState::Stopping:
        if (is_any_of(oldState, {Orchestration::ParticipantState::Running, Orchestration::ParticipantState::Paused}))
            return;

    case Orchestration::ParticipantState::Stopped:
        if (oldState == Orchestration::ParticipantState::Stopping)
            return;

    case Orchestration::ParticipantState::ShuttingDown:
        if (is_any_of(oldState, {Orchestration::ParticipantState::Error, Orchestration::ParticipantState::Stopped}))
            return;

    case Orchestration::ParticipantState::Shutdown:
        if (oldState == Orchestration::ParticipantState::ShuttingDown)
            return;

    case Orchestration::ParticipantState::Reinitializing:
        if (is_any_of(oldState, {Orchestration::ParticipantState::Error, Orchestration::ParticipantState::Stopped}))
            return;

    case Orchestration::ParticipantState::Error:
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

void SystemMonitor::UpdateSystemState(const Orchestration::ParticipantStatus& newStatus)
{
    auto&& nameIter = std::find(_requiredParticipantNames.begin(), _requiredParticipantNames.end(), newStatus.participantName);
    if (nameIter == _requiredParticipantNames.end())
    {
        return;
    }

    switch (newStatus.state)
    {
    case Orchestration::ParticipantState::ServicesCreated:
        if (AllRequiredParticipantsInState({Orchestration::ParticipantState::ServicesCreated,
                                            Orchestration::ParticipantState::CommunicationInitializing,
                                            Orchestration::ParticipantState::CommunicationInitialized,
                                            Orchestration::ParticipantState::ReadyToRun, 
                                            Orchestration::ParticipantState::Running}))
        {
            SetSystemState(Orchestration::SystemState::ServicesCreated);
        }
        return;

    case Orchestration::ParticipantState::CommunicationInitializing:
        if (AllRequiredParticipantsInState({Orchestration::ParticipantState::CommunicationInitializing,
                                            Orchestration::ParticipantState::CommunicationInitialized,
                                            Orchestration::ParticipantState::ReadyToRun, 
                                            Orchestration::ParticipantState::Running}))
        {
            SetSystemState(Orchestration::SystemState::CommunicationInitializing);
        }
        return;

    case Orchestration::ParticipantState::CommunicationInitialized:
        if (AllRequiredParticipantsInState({Orchestration::ParticipantState::CommunicationInitialized,
                                            Orchestration::ParticipantState::ReadyToRun, 
                                            Orchestration::ParticipantState::Running}))
        {
            SetSystemState(Orchestration::SystemState::CommunicationInitialized);
        }
        return;

    case Orchestration::ParticipantState::ReadyToRun:
        if (AllRequiredParticipantsInState({Orchestration::ParticipantState::ReadyToRun, 
                                            Orchestration::ParticipantState::Running}))
        {
            SetSystemState(Orchestration::SystemState::ReadyToRun);
        }
        return;

    case Orchestration::ParticipantState::Running:
        if (AllRequiredParticipantsInState({Orchestration::ParticipantState::Running}))
        {
            SetSystemState(Orchestration::SystemState::Running);
        }
        return;

    case Orchestration::ParticipantState::Paused:
        if (AllRequiredParticipantsInState({Orchestration::ParticipantState::Paused, 
                                            Orchestration::ParticipantState::Running}))
            SetSystemState(Orchestration::SystemState::Paused);
        return;

    case Orchestration::ParticipantState::Stopping:
        if (AllRequiredParticipantsInState({Orchestration::ParticipantState::Stopping, 
                                            Orchestration::ParticipantState::Stopped,
                                            Orchestration::ParticipantState::Paused, 
                                            Orchestration::ParticipantState::Running}))
            SetSystemState(Orchestration::SystemState::Stopping);
        return;

    case Orchestration::ParticipantState::Stopped:
        if (AllRequiredParticipantsInState({Orchestration::ParticipantState::Stopped}))
            SetSystemState(Orchestration::SystemState::Stopped);
        return;

    case Orchestration::ParticipantState::ShuttingDown:
        if (AllRequiredParticipantsInState({Orchestration::ParticipantState::ShuttingDown, 
                                            Orchestration::ParticipantState::Shutdown,
                                            Orchestration::ParticipantState::Stopped, 
                                            Orchestration::ParticipantState::Error,
                                            Orchestration::ParticipantState::ServicesCreated, 
                                            Orchestration::ParticipantState::ReadyToRun}))
            SetSystemState(Orchestration::SystemState::ShuttingDown);
        return;

    case Orchestration::ParticipantState::Shutdown:
        if (AllRequiredParticipantsInState({Orchestration::ParticipantState::Shutdown}))
            SetSystemState(Orchestration::SystemState::Shutdown);
        return;

    case Orchestration::ParticipantState::Error:
        SetSystemState(Orchestration::SystemState::Error);
        return;

    default:
        return;
    }
}

void SystemMonitor::SetSystemState(Orchestration::SystemState newState)
{
    _systemState = newState;
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
