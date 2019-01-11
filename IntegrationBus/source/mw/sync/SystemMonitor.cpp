// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "SystemMonitor.hpp"

#include <algorithm>
#include <ctime>

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

    if (_systemState != sync::SystemState::Invalid)
    {
        auto&& newHandler = _systemStateHandlers[_systemStateHandlers.size() - 1];
        newHandler(_systemState);
    }
}

void SystemMonitor::RegisterParticipantStateHandler(ParticipantStateHandlerT handler)
{
    _participantStateHandlers.emplace_back(std::move(handler));

    auto&& newHandler = _participantStateHandlers[_participantStateHandlers.size() - 1];
    for (auto&& kv : _participantStatus)
    {
        auto&& participantStatus = kv.second;
        if (participantStatus.state == sync::ParticipantState::Invalid)
            continue;

        newHandler(participantStatus.state);
    }
}

void SystemMonitor::RegisterParticipantStatusHandler(ParticipantStatusHandlerT handler)
{
    _participantStatusHandlers.emplace_back(std::move(handler));

    auto&& newHandler = _participantStatusHandlers[_participantStatusHandlers.size() - 1];
    for (auto&& kv : _participantStatus)
    {
        auto&& participantStatus = kv.second;
        if (participantStatus.state == sync::ParticipantState::Invalid)
            continue;

        newHandler(participantStatus);
    }
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

void SystemMonitor::ReceiveIbMessage(mw::EndpointAddress from, const sync::ParticipantStatus& newParticipantStatus)
{
    auto participantId = from.participant;

    auto&& statusIter = _participantStatus.find(participantId);
    if (statusIter == _participantStatus.end())
    {
        std::cerr << "Received ParticipantStatus from unknown ParticipantID=" << participantId << "\n";
        return;
    }

    auto oldParticipantState = _participantStatus[participantId].state;
    auto oldSystemState = _systemState;

    _participantStatus[participantId] = newParticipantStatus;
    ValidateParticipantStatusUpdate(newParticipantStatus, oldParticipantState);
    UpdateSystemState(newParticipantStatus, oldParticipantState);

    for (auto&& handler : _participantStatusHandlers)
        handler(newParticipantStatus);

    for (auto&& handler : _participantStateHandlers)
        handler(newParticipantStatus.state);

    if (oldSystemState != _systemState)
    {
        for (auto&& handler : _systemStateHandlers)
            handler(_systemState);
    }
}

bool SystemMonitor::AllParticipantsInState(sync::ParticipantState state) const
{
    return std::all_of(begin(_participantStatus), end(_participantStatus), [state](auto&& kv) {
        return kv.second.state == state;
    });
}

bool SystemMonitor::AllParticipantsInState(std::initializer_list<sync::ParticipantState> acceptedStates) const
{
    for (auto&& participantStatus : _participantStatus)
    {
        bool isAcceptedState = std::any_of(begin(acceptedStates), end(acceptedStates), [participantState = participantStatus.second.state](auto acceptedState) {
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
    case sync::ParticipantState::Idle:
        if (oldState == sync::ParticipantState::Invalid)
            return;

    case sync::ParticipantState::Initializing:
        if (is_any_of(oldState, {sync::ParticipantState::Idle, sync::ParticipantState::Error, sync::ParticipantState::Stopped}))
            return;
    case sync::ParticipantState::Initialized:
        if (oldState == sync::ParticipantState::Initializing)
            return;

    case sync::ParticipantState::Running:
        if (is_any_of(oldState, {sync::ParticipantState::Initialized, sync::ParticipantState::Paused}))
            return;

    case sync::ParticipantState::Paused:
        if (oldState == sync::ParticipantState::Running)
            return;

    case sync::ParticipantState::Stopped:
        if (is_any_of(oldState, {sync::ParticipantState::Running, sync::ParticipantState::Paused}))
            return;

    case sync::ParticipantState::Shutdown:
        if (is_any_of(oldState, {sync::ParticipantState::Error, sync::ParticipantState::Stopped}))
            return;
    case sync::ParticipantState::Error:
        return;

    default:
        std::cerr << "ERROR: SystemMonitor::ValidateParticipantStatusUpdate() Unhandled ParticipantState::" << oldState << "\n";
    }

    std::time_t enterTime = std::chrono::system_clock::to_time_t(newStatus.enterTime);
    char timebuffer[32];
    std::strftime(timebuffer, sizeof(timebuffer), "%FT%T", std::localtime(&enterTime));

    std::cerr
        << "ERROR: SystemMonitor detected invalid ParticipantState transition from " << oldState << " to " << newStatus.state
        << " {EnterTime=" << timebuffer
        << ", EnterReason=\"" << newStatus.enterReason
        << "\"\n";

    _invalidTransitionCount++;
}

void SystemMonitor::UpdateSystemState(const sync::ParticipantStatus& newStatus, sync::ParticipantState oldState)
{
    switch (newStatus.state)
    {
    case sync::ParticipantState::Idle:
        if (AllParticipantsInState(sync::ParticipantState::Idle))
            SetSystemState(sync::SystemState::Idle);

        return;

    case sync::ParticipantState::Initializing:
        if (AllParticipantsInState({sync::ParticipantState::Initializing, sync::ParticipantState::Idle})
            || AllParticipantsInState({sync::ParticipantState::Initializing, sync::ParticipantState::Stopped, sync::ParticipantState::Error}))
        {
            SetSystemState(sync::SystemState::Initializing);
        }
        return;

    case sync::ParticipantState::Initialized:
        if (AllParticipantsInState(sync::ParticipantState::Initialized))
            SetSystemState(sync::SystemState::Initialized);

        return;

    case sync::ParticipantState::Running:
        if (AllParticipantsInState(sync::ParticipantState::Running))
            SetSystemState(sync::SystemState::Running);

        return;

    case sync::ParticipantState::Paused:
        if (AllParticipantsInState({sync::ParticipantState::Paused, sync::ParticipantState::Running}))
            SetSystemState(sync::SystemState::Paused);

        return;

    case sync::ParticipantState::Stopped:
        if (AllParticipantsInState(sync::ParticipantState::Stopped))
            SetSystemState(sync::SystemState::Stopped);
        else if (AllParticipantsInState({sync::ParticipantState::Stopped, sync::ParticipantState::Paused, sync::ParticipantState::Running}))
            SetSystemState(sync::SystemState::Stopping);

        return;

    case sync::ParticipantState::Shutdown:
        if (AllParticipantsInState(sync::ParticipantState::Shutdown))
            SetSystemState(sync::SystemState::Shutdown);
        else if (AllParticipantsInState({sync::ParticipantState::Shutdown, sync::ParticipantState::Stopped, sync::ParticipantState::Error, sync::ParticipantState::Idle, sync::ParticipantState::Initialized}))
            SetSystemState(sync::SystemState::ShuttingDown);

        return;

    case sync::ParticipantState::Error:
        SetSystemState(sync::SystemState::Error);
        return;

    default:
        return;
    }
    
    SetSystemState(sync::SystemState::Invalid);
}

void SystemMonitor::SetSystemState(sync::SystemState newState)
{
    _systemState = newState;
}


} // namespace sync
} // namespace mw
} // namespace ib
