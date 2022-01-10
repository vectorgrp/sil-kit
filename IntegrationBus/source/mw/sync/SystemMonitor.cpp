// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SystemMonitor.hpp"

#include <algorithm>
#include <ctime>

#include "ib/mw/logging/ILogger.hpp"
#include "ib/mw/sync/string_utils.hpp"

namespace ib {
namespace mw {
namespace sync {

SystemMonitor::SystemMonitor(IComAdapterInternal* comAdapter, cfg::SimulationSetup simulationSetup)
    : _comAdapter{comAdapter}
    , _simulationSetup{std::move(simulationSetup)}
    , _logger{comAdapter->GetLogger()}
{
    for (auto&& participant : _simulationSetup.participants)
    {
        if (!participant.participantController)
            continue;

        _participantStatus[participant.name] = sync::ParticipantStatus{};
        _participantStatus[participant.name].state = sync::ParticipantState::Invalid;
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

auto SystemMonitor::ParticipantStatus(const std::string& participantId) const -> const sync::ParticipantStatus&
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
    _serviceDescriptor.legacyEpa = addr;
}

auto SystemMonitor::EndpointAddress() const -> const mw::EndpointAddress&
{
    return _serviceDescriptor.legacyEpa;
}

void SystemMonitor::ReceiveIbMessage(const IIbServiceEndpoint* from, const sync::ParticipantStatus& newParticipantStatus)
{
    auto participantId = newParticipantStatus.participantName;

    // TODO VIB-560: Need to figure out what the system monitor is supposed to do in the new configuration concept
    auto&& statusIter = _participantStatus.find(participantId);
    if (statusIter == _participantStatus.end())
    {
        _logger->Warn("Received ParticipantStatus from unknown ParticipantID={}", participantId);
        return;
    }

    auto oldParticipantState = _participantStatus[participantId].state;
    auto oldSystemState = _systemState;

    if (oldParticipantState == sync::ParticipantState::Shutdown)
    {
        _logger->Debug("Ignoring ParticipantState update from participant {} to ParticipantState::{} because participant is already in terminal state ParticipantState::Shutdown.",
            newParticipantStatus.participantName, newParticipantStatus.state);
        return;
    }

    _participantStatus[participantId] = newParticipantStatus;
    ValidateParticipantStatusUpdate(newParticipantStatus, oldParticipantState);
    UpdateSystemState(newParticipantStatus);

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
        if (is_any_of(oldState, {sync::ParticipantState::Invalid, sync::ParticipantState::ColdswapShutdown}))
            return;

    case sync::ParticipantState::Initializing:
        if (is_any_of(oldState, {sync::ParticipantState::Idle, sync::ParticipantState::Error, sync::ParticipantState::Stopped, sync::ParticipantState::ColdswapIgnored}))
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

    case sync::ParticipantState::Stopping:
        if (is_any_of(oldState, {sync::ParticipantState::Running, sync::ParticipantState::Paused}))
            return;

    case sync::ParticipantState::Stopped:
        if (oldState == sync::ParticipantState::Stopping)
            return;

    case sync::ParticipantState::ColdswapPrepare:
        if (is_any_of(oldState, {sync::ParticipantState::Stopped, sync::ParticipantState::Error}))
            return;

    case sync::ParticipantState::ColdswapReady:
        if (oldState == sync::ParticipantState::ColdswapPrepare)
            return;

    case sync::ParticipantState::ColdswapIgnored:
        if (oldState == sync::ParticipantState::ColdswapReady)
            return;

    case sync::ParticipantState::ColdswapShutdown:
        if (oldState == sync::ParticipantState::ColdswapReady)
            return;

    case sync::ParticipantState::ShuttingDown:
        if (is_any_of(oldState, {sync::ParticipantState::Error, sync::ParticipantState::Stopped}))
            return;

    case sync::ParticipantState::Shutdown:
        if (oldState == sync::ParticipantState::ShuttingDown)
            return;

    case sync::ParticipantState::Error:
        return;

    default:
        _logger->Error("SystemMonitor::ValidateParticipantStatusUpdate() Unhandled ParticipantState::{}", newStatus.state);
    }

    std::time_t enterTime = std::chrono::system_clock::to_time_t(newStatus.enterTime);
    std::tm tmBuffer;
#if defined(_MSC_VER)
    localtime_s(&tmBuffer, &enterTime);
#else
    localtime_r(&enterTime, &tmBuffer);
#endif
    char timeString[32];
    std::strftime(timeString, sizeof(timeString), "%FT%T", &tmBuffer);

    _logger->Error(
        "SystemMonitor detected invalid ParticipantState transition from {} to {} EnterTime={}, EnterReason=\"{}\"",
        oldState,
        newStatus.state,
        timeString,
        newStatus.enterReason);

    _invalidTransitionCount++;
}

void SystemMonitor::UpdateSystemState(const sync::ParticipantStatus& newStatus)
{
    switch (newStatus.state)
    {
    case sync::ParticipantState::Idle:
        if (SystemState() == sync::SystemState::ColdswapPending)
        {
            if (AllParticipantsInState({sync::ParticipantState::Idle, sync::ParticipantState::ColdswapIgnored}))
            {
                SetSystemState(sync::SystemState::ColdswapDone);
            }

        }
        else
        {
            if (AllParticipantsInState(sync::ParticipantState::Idle))
                SetSystemState(sync::SystemState::Idle);
            else if (AllParticipantsInState({sync::ParticipantState::Idle, sync::ParticipantState::Initializing, sync::ParticipantState::Initialized}))
                SetSystemState(sync::SystemState::Initializing);
        }
        return;

    case sync::ParticipantState::Initializing:
        if (AllParticipantsInState({sync::ParticipantState::Initializing, sync::ParticipantState::Idle}) // regular start
            || AllParticipantsInState({sync::ParticipantState::Initializing, sync::ParticipantState::Stopped, sync::ParticipantState::Error}) // re-initialization after one run
            || AllParticipantsInState({sync::ParticipantState::Initializing, sync::ParticipantState::Idle, sync::ParticipantState::ColdswapIgnored}) // after coldswap
            )
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

    case sync::ParticipantState::Stopping:
        if (AllParticipantsInState({sync::ParticipantState::Stopping, sync::ParticipantState::Stopped, sync::ParticipantState::Paused, sync::ParticipantState::Running}))
            SetSystemState(sync::SystemState::Stopping);

    case sync::ParticipantState::Stopped:
        if (AllParticipantsInState(sync::ParticipantState::Stopped))
            SetSystemState(sync::SystemState::Stopped);
        return;

    case sync::ParticipantState::ColdswapPrepare:
        if (AllParticipantsInState({sync::ParticipantState::Stopped, sync::ParticipantState::ColdswapPrepare, sync::ParticipantState::ColdswapReady, sync::ParticipantState::Error}))
            SetSystemState(sync::SystemState::ColdswapPrepare);
        return;

    case sync::ParticipantState::ColdswapReady:
        if (AllParticipantsInState(sync::ParticipantState::ColdswapReady))
            SetSystemState(sync::SystemState::ColdswapReady);
        return;

    case sync::ParticipantState::ColdswapShutdown:
        if (AllParticipantsInState({sync::ParticipantState::ColdswapReady, sync::ParticipantState::ColdswapShutdown, sync::ParticipantState::ColdswapIgnored}))
            SetSystemState(sync::SystemState::ColdswapPending);
        return;

    case sync::ParticipantState::ColdswapIgnored:
        if (AllParticipantsInState(sync::ParticipantState::ColdswapIgnored))
            SetSystemState(sync::SystemState::ColdswapDone);
        else if (AllParticipantsInState({sync::ParticipantState::ColdswapReady, sync::ParticipantState::ColdswapShutdown, sync::ParticipantState::ColdswapIgnored}))
            SetSystemState(sync::SystemState::ColdswapPending);
        return;

    case sync::ParticipantState::ShuttingDown:
        if (AllParticipantsInState({sync::ParticipantState::ShuttingDown, sync::ParticipantState::Shutdown, sync::ParticipantState::Stopped, sync::ParticipantState::Error, sync::ParticipantState::Idle, sync::ParticipantState::Initialized}))
            SetSystemState(sync::SystemState::ShuttingDown);

    case sync::ParticipantState::Shutdown:
        if (AllParticipantsInState(sync::ParticipantState::Shutdown))
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
