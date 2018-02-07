// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "SystemMonitor.hpp"

#include <algorithm>

#include "ib/mw/sync/string_utils.hpp"

namespace ib {
namespace mw {
namespace sync {

SystemMonitor::SystemMonitor(IComAdapter* comAdapter, cfg::SimulationSetup simulationSetup)
    : _comAdapter{comAdapter}
    , _simulationSetup{std::move(simulationSetup)}
{
    for (auto&& participant : _simulationSetup.participants)
    {
        if (participant.syncType == cfg::SyncType::Unsynchronized)
            continue;

        _participantStatus[participant.id] = sync::ParticipantStatus{};
        _participantStatus[participant.id].state = sync::ParticipantState::Invalid;
    }
}

void SystemMonitor::RegisterSystemStateHandler(SystemStateHandlerT handler)
{
    _systemStateHandlers.emplace_back(std::move(handler));
}

void SystemMonitor::RegisterParticipantStateHandler(ParticipantStateHandlerT handler)
{
    _participantStateHandlers.emplace_back(std::move(handler));
}

void SystemMonitor::RegisterParticipantStatusHandler(ParticipantStatusHandlerT handler)
{
    _participantStatusHandlers.emplace_back(std::move(handler));
}

auto SystemMonitor::SystemState() const -> sync::SystemState
{
    return _systemState;
}

auto SystemMonitor::ParticipantState(ParticipantId participantId) const -> sync::ParticipantState
{
    return ParticipantStatus(participantId).state;
}

auto SystemMonitor::ParticipantStatus(ParticipantId participantId) const -> const sync::ParticipantStatus&
{
    auto&& statusIter = _participantStatus.find(participantId);
    if (statusIter == _participantStatus.end())
    {
        throw std::runtime_error{"Unknown ParticipantId"};
    }

    return statusIter->second;
}

void SystemMonitor::SetEndpointAddress(const mw::EndpointAddress& addr)
{
    _endpointAddress = addr;
}

auto SystemMonitor::EndpointAddress() const -> const mw::EndpointAddress&
{
    return _endpointAddress;
}

void SystemMonitor::ReceiveIbMessage(mw::EndpointAddress from, const sync::ParticipantStatus& msg)
{
    auto participantId = from.participant;

    auto&& statusIter = _participantStatus.find(participantId);
    if (statusIter == _participantStatus.end())
    {
        std::cerr << "Received ParticipantStatus from unknown ParticipantID=" << participantId << "\n";
        return;
    }

    _participantStatus[participantId] = msg;
    auto oldSystemState = _systemState;
    UpdateSystemState(msg);

    for (auto&& handler : _participantStatusHandlers)
        handler(msg);

    for (auto&& handler : _participantStateHandlers)
        handler(msg.state);

    if (oldSystemState != _systemState)
    {
        for (auto&& handler : _systemStateHandlers)
            handler(_systemState);
    }
}

bool SystemMonitor::AllParticipantsInState(sync::ParticipantState state) const
{
    return
        std::all_of(
            begin(_participantStatus),
            end(_participantStatus),
            [state](auto&& kv) { return kv.second.state == state; }
        );
}

bool SystemMonitor::ValidParticipantState(sync::ParticipantState newState, std::initializer_list<sync::ParticipantState> validStates)
{
    auto isValid =
        std::any_of(
            begin(validStates),
            end(validStates),
            [newState](auto state) { return newState == state; }
        );

    if (!isValid)
    {
        _invalidTransitionCount++;
        std::cerr << "Invalid ParticipantState::" << newState << " while in SystemState::" << _systemState << "\n";
    }

    return isValid;
}

void SystemMonitor::UpdateSystemState(const sync::ParticipantStatus& newStatus)
{
    auto newState = newStatus.state;
    // ParticipantState::Error will always cause a switch to SystemState::Error
    if (newState == sync::ParticipantState::Error)
    {
        SetSystemState(sync::SystemState::Error);
        return;
    }

    switch (_systemState)
    {
    case sync::SystemState::Invalid:
        // valid new participant states: idle
        if (!ValidParticipantState(newState, {sync::ParticipantState::Idle}))
            return;

        if (AllParticipantsInState(sync::ParticipantState::Idle))
            SetSystemState(sync::SystemState::Idle);

        return;

    case sync::SystemState::Idle:
        // valid new participant states: initializing
        if (!ValidParticipantState(newState, {sync::ParticipantState::Initializing}))
            return;

        SetSystemState(sync::SystemState::Initializing);
        return;

    case sync::SystemState::Initializing:
        // valid new participant states: initialized, initializing
        if (!ValidParticipantState(newState, {sync::ParticipantState::Initializing, sync::ParticipantState::Initialized}))
            return;

        if (AllParticipantsInState(sync::ParticipantState::Initialized))
            SetSystemState(sync::SystemState::Initialized);

        return;

    case sync::SystemState::Initialized:
        // valid new participant states: running
        if (!ValidParticipantState(newState, {sync::ParticipantState::Running}))
            return;

        if (AllParticipantsInState(sync::ParticipantState::Running))
            SetSystemState(sync::SystemState::Running);

        return;

    case sync::SystemState::Running:
        // valid new participant states: paused, stopped
        if (!ValidParticipantState(newState, {sync::ParticipantState::Paused, sync::ParticipantState::Stopped}))
            return;

        if (newState == sync::ParticipantState::Paused)
        {
            SetSystemState(sync::SystemState::Paused);
        }
        else if (newState == sync::ParticipantState::Stopped)
        {
            SetSystemState(sync::SystemState::Stopping);
            // if there's only one participant, switch directly to stopped.
            if (AllParticipantsInState(sync::ParticipantState::Stopped))
                SetSystemState(sync::SystemState::Stopped);
        }

        return;

    case sync::SystemState::Paused:
        // valid new participant states: Running, Paused
        if (!ValidParticipantState(newState, {sync::ParticipantState::Running, sync::ParticipantState::Paused}))
            return;

        // nothing to be done when we're already in Paused and another Participant reports a Paused state.
        if (newState == sync::ParticipantState::Paused)
            return;

        if (AllParticipantsInState(sync::ParticipantState::Running))
            SetSystemState(sync::SystemState::Running);

        return;


    case sync::SystemState::Stopping:
        // valid new participant states: Stopped
        if (!ValidParticipantState(newState, {sync::ParticipantState::Stopped}))
            return;

        if (AllParticipantsInState(sync::ParticipantState::Stopped))
            SetSystemState(sync::SystemState::Stopped);

        return;

    case sync::SystemState::Stopped:
        // valid new participant states: Initializing, Shutdown
        if (!ValidParticipantState(newState, {sync::ParticipantState::Initializing, sync::ParticipantState::Shutdown}))
            return;

        if (newState == sync::ParticipantState::Initializing)
        {
            SetSystemState(sync::SystemState::Initializing);
        }
        else if (newState == sync::ParticipantState::Shutdown)
        {
            if (AllParticipantsInState(sync::ParticipantState::Shutdown))
                SetSystemState(sync::SystemState::Shutdown);
            else
                SetSystemState(sync::SystemState::ShuttingDown);
        }
        return;

    case sync::SystemState::ShuttingDown:
        // valid new participant states: Shutdown
        if (!ValidParticipantState(newState, {sync::ParticipantState::Shutdown}))
            return;

        if (AllParticipantsInState(sync::ParticipantState::Shutdown))
            SetSystemState(sync::SystemState::Shutdown);

        return;

    case sync::SystemState::Shutdown:
        // valid new participant states: none

        // log state transition attempts
        ValidParticipantState(newState, {});
        return;

    case sync::SystemState::Error:
        // valid new participant states: Shutdown, Initializing
        if (!ValidParticipantState(newState, {sync::ParticipantState::Shutdown, sync::ParticipantState::Initializing}))
            return;

        // Check if all participants have reached a stable state since the
        // detection of the error. I.e., if they are in state Error or Stopped.
        // If not, issue a warning
        for (auto&& statusIter : _participantStatus)
        {
            auto&& status = statusIter.second;
            switch (status.state)
            {
            case sync::ParticipantState::Stopped:
                continue;
            case sync::ParticipantState::Error:
                continue;
            default:
                std::cerr
                    << "WARNING: SystemMonitor detected participant transition to " << newState
                    << "while participant \"" << status.participantName << "\" is still in state " << status.state << "\n";
            }
        }

        if (newState == sync::ParticipantState::Initializing)
            SetSystemState(sync::SystemState::Initializing);
        else if (newState == sync::ParticipantState::Shutdown)
            SetSystemState(sync::SystemState::ShuttingDown);

        return;

    default:
        // All cases must be explicitly covered. Thus, there is no valid state transition.
        std::cerr << "WARNING SystemMonitor::UpdateSystemState(): unhandled system state: " << _systemState << "\n";

        // Validate state request against an empty list to log the request.
        ValidParticipantState(newState, {});
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
