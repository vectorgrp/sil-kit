// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <deque>
#include <map>
#include <mutex>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "silkit/experimental/serviceDiscovery/ServiceDiscoveryDatatypes.hpp"
#include "silkit/services/orchestration/OrchestrationDatatypes.hpp"

namespace Introspection {

using Clock = std::chrono::steady_clock;
using SilKit::Experimental::ServiceDiscovery::ServiceDescriptor;
using SilKit::Experimental::ServiceDiscovery::ServiceDiscoveryEventType;
using SilKit::Experimental::ServiceDiscovery::ServiceKind;
using SilKit::Experimental::NetworkSimulation::SimulatedNetworkType;
using SilKit::Services::Orchestration::OperationMode;
using SilKit::Services::Orchestration::ParticipantState;
using SilKit::Services::Orchestration::SystemState;

//! Tracks when an element appeared and, once it is gone, when it disappeared. Removed elements are kept
//! for a short while as "ghosts" so that the dashboard can show what just vanished.
struct Lifetime
{
    Clock::time_point born{Clock::now()};
    Clock::time_point died{};
    bool removed{false};

    void Remove()
    {
        if (!removed)
        {
            removed = true;
            died = Clock::now();
        }
    }

    //! An element that was already there when the observer joined: not highlighted as new.
    static auto Settled() -> Lifetime
    {
        Lifetime life;
        life.born = Clock::time_point{};
        return life;
    }
};

//! Whether a participant takes part in the virtual time synchronization; unknown until its lifecycle started.
enum class TimeSync
{
    Unknown,
    Synchronized,
    Unsynchronized,
};

struct Participant
{
    std::string name;
    bool connected{false};
    bool hasState{false};
    ParticipantState state{ParticipantState::Invalid};
    OperationMode operationMode{OperationMode::Invalid}; // Invalid = unknown (lifecycle not started yet)
    TimeSync timeSync{TimeSync::Unknown};
    Lifetime life;
};

struct Service
{
    std::string participant;
    std::string name;
    uint64_t serviceId{0};
    ServiceKind kind{ServiceKind::Undefined};
    std::string id; // network name, topic or function name
    SimulatedNetworkType networkType{SimulatedNetworkType::Undefined}; // bus controllers only
    std::string mediaType;
    std::vector<SilKit::Services::MatchingLabel> labels;
    Lifetime life;
};

//! A pub/sub or RPC match. The parent is the receiving side (DataSubscriber / RpcServer), the peer the
//! connecting side (DataPublisher / RpcClient).
struct Match
{
    std::string parentParticipant;
    std::string parentService;
    uint64_t parentId{0};
    std::string peerParticipant;
    std::string peerService;
    uint64_t peerId{0};
    std::string id;
    bool rpc{false};
    Lifetime life;
};

//! A network-simulator link: the participant simulates the named network.
struct SimLink
{
    std::string participant;
    std::string network;
    SimulatedNetworkType networkType{SimulatedNetworkType::Undefined};
    Lifetime life;
};

struct LogEvent
{
    enum class Type
    {
        ServiceAdded,
        ServiceRemoved,
        MatchAdded,
        MatchRemoved,
        SimAdded,
        SimRemoved,
        Joined,
        Left,
        StateChanged,
        ModeChanged,
        SystemStateChanged,
        TimeSyncJoined,
    };

    explicit LogEvent(Type eventType)
        : type{eventType}
    {
    }

    Type type;
    std::chrono::system_clock::time_point time{std::chrono::system_clock::now()};
    std::string participant;
    std::string service;
    ServiceKind kind{ServiceKind::Undefined};
    std::string id;
    SimulatedNetworkType networkType{SimulatedNetworkType::Undefined};
    std::string peerParticipant;
    std::string peerService;
    bool hadState{false};
    ParticipantState oldState{ParticipantState::Invalid};
    ParticipantState newState{ParticipantState::Invalid};
    OperationMode operationMode{OperationMode::Invalid};
    TimeSync timeSync{TimeSync::Unknown};
    SystemState oldSystemState{SystemState::Invalid};
    SystemState newSystemState{SystemState::Invalid};
    std::chrono::nanoseconds simulationTime{0};
    uint64_t sequence{0};
};

//! Everything the dashboard draws. Copied out of the model so rendering never holds the model lock.
struct NetworkState
{
    // Services are identified by their participant and service id, as reported by the service discovery.
    using ServiceKey = std::pair<std::string, uint64_t>;
    using MatchKey = std::tuple<std::string, uint64_t, std::string, uint64_t>; // receiving side, connected side
    using SimKey = ServiceKey;

    std::map<std::string, Participant> participants;
    std::map<ServiceKey, Service> services;
    std::map<MatchKey, Match> matches;
    std::map<SimKey, SimLink> simLinks;
    //! Aggregated state of the required participants; Invalid if no workflow configuration is set.
    SystemState systemState{SystemState::Invalid};

    //! Global simulation time as seen by the observer's own time sync service (only with --sim-time).
    struct SimulationTime
    {
        bool enabled{false};
        bool valid{false};   // at least one step executed since the observer (re)joined the time sync
        bool waiting{false}; // no other synchronized participant: the observer holds its clock
        std::chrono::nanoseconds now{0};
        std::chrono::nanoseconds stepSize{0};
        double realTimeFactor{0.0}; // simulated seconds per wall-clock second, 0 if unknown
    } simulationTime;

    std::deque<LogEvent> log;
};

inline bool IsBusController(ServiceKind kind)
{
    return kind == ServiceKind::CanController || kind == ServiceKind::EthernetController
           || kind == ServiceKind::FlexrayController || kind == ServiceKind::LinController;
}

//! Builds and maintains the network picture from the service-discovery and system-monitor callbacks.
//! All entry points are thread-safe; they are called from SIL Kit threads and must not block.
class NetworkModel
{
public:
    explicit NetworkModel(std::string ownName)
        : _ownName{std::move(ownName)}
    {
    }

    void OnServiceEvent(ServiceDiscoveryEventType type, const ServiceDescriptor& descriptor)
    {
        if (type == ServiceDiscoveryEventType::Invalid || descriptor.participantName == _ownName)
        {
            return;
        }
        const bool created = type == ServiceDiscoveryEventType::ServiceCreated;

        std::lock_guard<std::mutex> lock{_mutex};
        if (descriptor.serviceKind == ServiceKind::LifecycleService
            || descriptor.serviceKind == ServiceKind::TimeSyncService)
        {
            OnOrchestration(created, descriptor);
            return;
        }
        if (descriptor.serviceKind == ServiceKind::NetworkSimulatorLink)
        {
            OnSimLink(created, descriptor);
            return;
        }
        if (descriptor.serviceKind == ServiceKind::PubSubMatch || descriptor.serviceKind == ServiceKind::RpcMatch)
        {
            OnMatch(created, descriptor);
            return;
        }

        const NetworkState::ServiceKey key{descriptor.participantName, descriptor.serviceId};
        if (created)
        {
            TouchParticipant(descriptor.participantName);

            Service service;
            service.participant = descriptor.participantName;
            service.name = descriptor.serviceName;
            service.serviceId = descriptor.serviceId;
            service.kind = descriptor.serviceKind;
            service.id = descriptor.primaryIdentifier;
            service.networkType = descriptor.networkType;
            service.mediaType = descriptor.mediaType;
            service.labels = descriptor.labels;
            Insert(_state.services, key, std::move(service), descriptor.isSnapshot,
                   [this](const Service& added) { Log(MakeServiceEvent(LogEvent::Type::ServiceAdded, added)); });
        }
        else
        {
            const auto it = _state.services.find(key);
            if (it == _state.services.end() || it->second.life.removed)
            {
                return;
            }
            it->second.life.Remove();
            Log(MakeServiceEvent(LogEvent::Type::ServiceRemoved, it->second));
        }
    }

    void OnParticipantConnected(const std::string& name)
    {
        if (name == _ownName)
        {
            return;
        }
        std::lock_guard<std::mutex> lock{_mutex};
        auto& participant = TouchParticipant(name);
        if (!participant.connected)
        {
            participant.connected = true;
            LogEvent event{LogEvent::Type::Joined};
            event.participant = name;
            Log(event);
        }
    }

    void OnParticipantDisconnected(const std::string& name)
    {
        std::lock_guard<std::mutex> lock{_mutex};
        const auto it = _state.participants.find(name);
        if (it == _state.participants.end() || it->second.life.removed)
        {
            return;
        }
        it->second.connected = false;
        it->second.life.Remove();

        LogEvent event{LogEvent::Type::Left};
        event.participant = name;
        Log(event);

        // Its services are gone with it; their own removal events may still follow and are then not logged
        // twice. Matches and network-simulator links are left to their own removal events.
        for (auto& entry : _state.services)
        {
            if (entry.second.participant == name)
            {
                entry.second.life.Remove();
            }
        }
    }

    void OnParticipantStatus(const std::string& name, ParticipantState state)
    {
        if (name == _ownName)
        {
            return;
        }
        std::lock_guard<std::mutex> lock{_mutex};
        auto& participant = TouchParticipant(name);
        if (participant.hasState && participant.state == state)
        {
            return;
        }
        LogEvent event{LogEvent::Type::StateChanged};
        event.participant = name;
        event.hadState = participant.hasState;
        event.oldState = participant.state;
        event.newState = state;

        // Keep the log concise: a burst of transitions (e.g. the start-up sequence) collapses into one line.
        if (!_state.log.empty())
        {
            const auto& last = _state.log.back();
            if (last.type == LogEvent::Type::StateChanged && last.participant == name
                && event.time - last.time < std::chrono::seconds{1})
            {
                event.hadState = last.hadState;
                event.oldState = last.oldState;
                _state.log.pop_back();
            }
        }
        Log(event);

        participant.hasState = true;
        participant.state = state;
    }

    void OnSystemState(SystemState state)
    {
        std::lock_guard<std::mutex> lock{_mutex};
        if (state == _state.systemState)
        {
            return;
        }
        LogEvent event{LogEvent::Type::SystemStateChanged};
        event.oldSystemState = _state.systemState;
        event.newSystemState = state;
        _state.systemState = state;

        // A burst of transitions (e.g. the start-up sequence) collapses into one line.
        if (!_state.log.empty())
        {
            const auto& last = _state.log.back();
            if (last.type == LogEvent::Type::SystemStateChanged && event.time - last.time < std::chrono::seconds{1})
            {
                event.oldSystemState = last.oldSystemState;
                _state.log.pop_back();
            }
        }
        Log(event);
    }

    void EnableSimulationTime(std::chrono::nanoseconds stepSize)
    {
        std::lock_guard<std::mutex> lock{_mutex};
        _state.simulationTime.enabled = true;
        _state.simulationTime.stepSize = stepSize;
    }

    //! Called from the observer's own simulation step handler.
    void OnSimulationStep(std::chrono::nanoseconds now)
    {
        std::lock_guard<std::mutex> lock{_mutex};
        auto& simTime = _state.simulationTime;
        const auto wallNow = Clock::now();
        if (!simTime.valid)
        {
            LogEvent event{LogEvent::Type::TimeSyncJoined};
            event.simulationTime = now;
            Log(event);
            _rateAnchorWall = wallNow;
            _rateAnchorSim = now;
        }
        simTime.valid = true;
        simTime.now = now;

        // Real-time factor, measured over windows of about one second.
        const auto wallElapsed = wallNow - _rateAnchorWall;
        if (wallElapsed >= std::chrono::seconds{1})
        {
            simTime.realTimeFactor = std::chrono::duration<double>(now - _rateAnchorSim).count()
                                     / std::chrono::duration<double>(wallElapsed).count();
            _rateAnchorWall = wallNow;
            _rateAnchorSim = now;
        }
    }

    void SetSimulationTimeWaiting(bool waiting)
    {
        std::lock_guard<std::mutex> lock{_mutex};
        _state.simulationTime.waiting = waiting;
        if (waiting)
        {
            _state.simulationTime.realTimeFactor = 0.0;
        }
    }

    //! The observer left the time sync and rejoins from time zero.
    void ResetSimulationTime()
    {
        std::lock_guard<std::mutex> lock{_mutex};
        _state.simulationTime.valid = false;
        _state.simulationTime.realTimeFactor = 0.0;
    }

    //! True if another participant takes part in the virtual time synchronization.
    auto HasSynchronizedPeers() -> bool
    {
        std::lock_guard<std::mutex> lock{_mutex};
        for (const auto& entry : _state.participants)
        {
            if (!entry.second.life.removed && entry.second.timeSync == TimeSync::Synchronized)
            {
                return true;
            }
        }
        return false;
    }

    //! Drops ghosts older than ghostDuration and returns a copy of the current state.
    auto Snapshot(Clock::duration ghostDuration) -> NetworkState
    {
        std::lock_guard<std::mutex> lock{_mutex};
        const auto now = Clock::now();
        const auto expired = [now, ghostDuration](const Lifetime& life) {
            return life.removed && now - life.died > ghostDuration;
        };
        EraseIf(_state.services, [&](const Service& s) { return expired(s.life); });
        EraseIf(_state.matches, [&](const Match& l) { return expired(l.life); });
        EraseIf(_state.simLinks, [&](const SimLink& l) { return expired(l.life); });
        EraseIf(_state.participants, [&](const Participant& p) {
            if (!expired(p.life))
            {
                return false;
            }
            for (const auto& entry : _state.services)
            {
                if (entry.second.participant == p.name)
                {
                    return false;
                }
            }
            return true;
        });
        return _state;
    }

private:
    static constexpr size_t MaxLogEntries = 500;

    template <typename Map, typename Predicate>
    static void EraseIf(Map& map, Predicate predicate)
    {
        for (auto it = map.begin(); it != map.end();)
        {
            it = predicate(it->second) ? map.erase(it) : std::next(it);
        }
    }

    //! Adds an element reported as created. Elements from the initial snapshot were there before the observer
    //! joined: they are neither logged nor highlighted. A creation of an element that is still known (e.g. the
    //! snapshot after the observer rejoined) keeps its lifetime and is not logged again.
    template <typename Map, typename Value, typename LogAdded>
    static void Insert(Map& map, const typename Map::key_type& key, Value value, bool isSnapshot, LogAdded logAdded)
    {
        const auto it = map.find(key);
        const bool known = it != map.end() && !it->second.life.removed;
        if (known)
        {
            value.life = it->second.life;
        }
        else if (isSnapshot)
        {
            value.life = Lifetime::Settled();
        }
        map[key] = value;
        if (!known && !isSnapshot)
        {
            logAdded(value);
        }
    }

    auto TouchParticipant(const std::string& name) -> Participant&
    {
        auto& participant = _state.participants[name];
        if (participant.name.empty() || participant.life.removed)
        {
            participant = Participant{};
            participant.name = name;
        }
        return participant;
    }

    static auto MakeServiceEvent(LogEvent::Type type, const Service& service) -> LogEvent
    {
        LogEvent event{type};
        event.participant = service.participant;
        event.service = service.name;
        event.kind = service.kind;
        event.id = service.id;
        return event;
    }

    //! Lifecycle and time sync services are properties of their participant, not services of their own.
    void OnOrchestration(bool created, const ServiceDescriptor& descriptor)
    {
        auto& participant = TouchParticipant(descriptor.participantName);
        const auto mode = created ? descriptor.operationMode : OperationMode::Invalid;
        const auto timeSync = !created ? TimeSync::Unknown
                              : descriptor.timeSyncActive ? TimeSync::Synchronized
                                                          : TimeSync::Unsynchronized;
        const bool isLifecycle = descriptor.serviceKind == ServiceKind::LifecycleService;
        const bool changed = isLifecycle ? participant.operationMode != mode : participant.timeSync != timeSync;
        if (isLifecycle)
        {
            participant.operationMode = mode;
        }
        else
        {
            participant.timeSync = timeSync;
        }
        // Removal comes with the participant leaving, which is logged already; the snapshot is not logged.
        if (!changed || !created || descriptor.isSnapshot)
        {
            return;
        }

        LogEvent event{LogEvent::Type::ModeChanged};
        event.participant = descriptor.participantName;
        event.operationMode = participant.operationMode;
        event.timeSync = participant.timeSync;
        // Lifecycle and time sync are announced back to back: report them in one line.
        if (!_state.log.empty())
        {
            const auto& last = _state.log.back();
            if (last.type == LogEvent::Type::ModeChanged && last.participant == event.participant
                && event.time - last.time < std::chrono::seconds{1})
            {
                _state.log.pop_back();
            }
        }
        Log(event);
    }

    void OnSimLink(bool created, const ServiceDescriptor& descriptor)
    {
        const NetworkState::SimKey key{descriptor.participantName, descriptor.serviceId};
        if (created)
        {
            TouchParticipant(descriptor.participantName);
            SimLink sim{descriptor.participantName, descriptor.primaryIdentifier, descriptor.networkType, {}};
            Insert(_state.simLinks, key, std::move(sim), descriptor.isSnapshot,
                   [this](const SimLink& added) { Log(MakeSimEvent(LogEvent::Type::SimAdded, added)); });
        }
        else
        {
            const auto it = _state.simLinks.find(key);
            if (it != _state.simLinks.end() && !it->second.life.removed)
            {
                it->second.life.Remove();
                Log(MakeSimEvent(LogEvent::Type::SimRemoved, it->second));
            }
        }
    }

    static auto MakeSimEvent(LogEvent::Type type, const SimLink& sim) -> LogEvent
    {
        LogEvent event{type};
        event.participant = sim.participant;
        event.id = sim.network;
        event.networkType = sim.networkType;
        return event;
    }

    void OnMatch(bool created, const ServiceDescriptor& descriptor)
    {
        const NetworkState::MatchKey key{descriptor.participantName, descriptor.serviceId,
                                         descriptor.connectedParticipantName, descriptor.connectedServiceId};
        if (created)
        {
            Match match;
            match.parentParticipant = descriptor.participantName;
            match.parentService = descriptor.serviceName;
            match.parentId = descriptor.serviceId;
            match.peerParticipant = descriptor.connectedParticipantName;
            match.peerService = descriptor.connectedServiceName;
            match.peerId = descriptor.connectedServiceId;
            match.id = descriptor.primaryIdentifier;
            match.rpc = descriptor.serviceKind == ServiceKind::RpcMatch;
            Insert(_state.matches, key, std::move(match), descriptor.isSnapshot,
                   [this](const Match& added) { Log(MakeMatchEvent(LogEvent::Type::MatchAdded, added)); });
        }
        else
        {
            const auto it = _state.matches.find(key);
            if (it != _state.matches.end() && !it->second.life.removed)
            {
                it->second.life.Remove();
                Log(MakeMatchEvent(LogEvent::Type::MatchRemoved, it->second));
            }
        }
    }

    static auto MakeMatchEvent(LogEvent::Type type, const Match& match) -> LogEvent
    {
        LogEvent event{type};
        event.participant = match.parentParticipant;
        event.service = match.parentService;
        event.peerParticipant = match.peerParticipant;
        event.peerService = match.peerService;
        event.id = match.id;
        event.kind = match.rpc ? ServiceKind::RpcServer : ServiceKind::DataSubscriber;
        return event;
    }

    void Log(LogEvent event)
    {
        event.sequence = ++_sequence;
        _state.log.push_back(std::move(event));
        while (_state.log.size() > MaxLogEntries)
        {
            _state.log.pop_front();
        }
    }

private:
    std::string _ownName;
    std::mutex _mutex;
    NetworkState _state;
    uint64_t _sequence{0};
    Clock::time_point _rateAnchorWall{};
    std::chrono::nanoseconds _rateAnchorSim{0};
};

} // namespace Introspection
