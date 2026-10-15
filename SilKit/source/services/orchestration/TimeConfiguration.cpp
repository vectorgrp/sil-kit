// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "services/orchestration/TimeConfiguration.hpp"
#include "services/logging/LoggerMessage.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {


TimeConfiguration::TimeConfiguration(Logging::ILoggerInternal* logger)
    : _blocking(false)
    , _logger(logger)

{
    Initialize();
    // NB: This is used when SetPeriod is never called
    _myNextTask.duration = 1ms;
}

void TimeConfiguration::SetDynamicStepSizeEnabled(bool enabled)
{
    Lock lock{_mx};
    _dynamicStepSizeEnabled = enabled;
    _peerLatencyCache.clear();
}

bool TimeConfiguration::IsDynamicStepSizeEnabled() const
{
    Lock lock{_mx};
    return _dynamicStepSizeEnabled;
}

void TimeConfiguration::SetBlockingMode(bool blocking)
{
    _blocking = blocking;
}

void TimeConfiguration::AddSynchronizedParticipant(const std::string& otherParticipantName)
{
    Lock lock{_mx};
    if (_otherNextTasks.find(otherParticipantName) != _otherNextTasks.end())
    {
        // ignore already known participants
        return;
    }
    NextSimTask task;
    task.timePoint = -1ns;
    task.duration = 0ns;
    _otherNextTasks.emplace(otherParticipantName, task);
}


bool TimeConfiguration::RemoveSynchronizedParticipant(const std::string& otherParticipantName)
{
    Lock lock{_mx};
    auto it = _otherNextTasks.find(otherParticipantName);
    if (it != _otherNextTasks.end())
    {
        _otherNextTasks.erase(it);
        _peerNetworks.erase(otherParticipantName);
        _sendTimeBounds.erase(otherParticipantName);
        _peerLatencyCache.erase(otherParticipantName);
        return true;
    }
    return false;
}

auto TimeConfiguration::GetSynchronizedParticipantNames() -> std::vector<std::string>
{
    std::vector<std::string> participantNames;
    for (const auto& it : _otherNextTasks)
    {
        participantNames.push_back(it.first);
    }
    return participantNames;
}

void TimeConfiguration::OnReceiveNextSimStep(const std::string& participantName, NextSimTask nextStep)
{
    Lock lock{_mx};

    auto&& itOtherNextTask = _otherNextTasks.find(participantName);
    if (itOtherNextTask == _otherNextTasks.end())
    {
        _logger->MakeMessage(Logging::Level::Error, TopicOf(*this))
            .SetMessage("Received NextSimTask from unknown participant {}", participantName)
            .Dispatch();
        return;
    }

    if (nextStep.timePoint < itOtherNextTask->second.timePoint)
    {
        _logger->MakeMessage(Logging::Level::Error, TopicOf(*this))
            .SetMessage("Chonology error: Received NextSimTask from participant \'{}\' with lower timePoint {} than last "
                "known timePoint {}",
                participantName, nextStep.timePoint.count(), itOtherNextTask->second.timePoint.count())
            .Dispatch();
    }

    _otherNextTasks.at(participantName) = std::move(nextStep);

    _logger->MakeMessage(Logging::Level::Debug, TopicOf(*this))
        .SetMessage("Updated _otherNextTasks for participant {} with time {}", participantName,
                    nextStep.timePoint.count())
        .Dispatch();
}


void TimeConfiguration::SetStepDuration(std::chrono::nanoseconds duration)
{
    Lock lock{_mx};

    if (duration == 0ns)
    {
        throw SilKitError("Attempted to set step duration to zero.");
    }

    _myNextTask.duration = duration;
}

auto TimeConfiguration::GetMinimalAlignedDuration() const -> std::chrono::nanoseconds
{
    if (_otherNextTasks.empty())
    {
        return std::chrono::nanoseconds::max();
    }

    auto earliestOtherTimepoint = std::chrono::nanoseconds::max();
    for (const auto& entry : _otherNextTasks)
    {
        // Both start and end of other participant's step could be the earliest next timepoint
        auto nextStepStart = entry.second.timePoint;
        auto nextStepEnd = entry.second.timePoint + entry.second.duration;

        if (nextStepStart > _currentTask.timePoint)
        {
            earliestOtherTimepoint = std::min(earliestOtherTimepoint, nextStepStart);
        }

        if (nextStepEnd > _currentTask.timePoint)
        {
            earliestOtherTimepoint = std::min(earliestOtherTimepoint, nextStepEnd);
        }
    }

    Logging::Debug(_logger, "Earliest next timepoint among other participants is {}ns", earliestOtherTimepoint.count());

    const auto minAlignedDuration = earliestOtherTimepoint - _currentTask.timePoint;

    if (minAlignedDuration < 0ns)
    {
        Logging::Error(_logger,
                       "Chonology error: Calculated minimal aligned duration is non-positive ({}ns). This indicates "
                       "that at least one participant has not advanced its time correctly.",
                       minAlignedDuration.count());
        return std::chrono::nanoseconds::max();
    }

    return minAlignedDuration;
}

void TimeConfiguration::AdvanceTimeStep()
{
    Lock lock{_mx};
    _currentTask = _myNextTask;

    if (_dynamicStepSizeEnabled)
    {
        const auto minAlignedDuration = GetMinimalAlignedDuration();

        if (minAlignedDuration < _currentTask.duration)
        {
            Logging::Debug(_logger, "Adjusting my step duration from {}ms to {}ms",
                           std::chrono::duration_cast<std::chrono::nanoseconds>(_currentTask.duration).count(),
                           std::chrono::duration_cast<std::chrono::nanoseconds>(minAlignedDuration).count());
            _currentTask.duration = minAlignedDuration;
        }
    }

    _myNextTask.timePoint = _currentTask.timePoint + _currentTask.duration;
}

auto TimeConfiguration::CurrentSimStep() const -> NextSimTask
{
    Lock lock{_mx};
    return _currentTask;
}

auto TimeConfiguration::NextSimStep() const -> NextSimTask
{
    Lock lock{_mx};
    return _myNextTask;
}

bool TimeConfiguration::OtherParticipantHasLowerTimepoint() const
{
    Lock lock{_mx};
    return OtherParticipantHasLowerTimepointUnlocked();
}

bool TimeConfiguration::OtherParticipantHasLowerTimepointUnlocked() const
{
    const auto maxRunAhead = EffectiveMaxRunAheadUnlocked();
    // Outside exact mode, skip the per-peer lookups: this runs for every peer on every step
    const bool exactMode = IsExactModeEnabledUnlocked();

    for (const auto& otherTask : _otherNextTasks)
    {
        // Never run ahead of a participant that has not announced its first step yet
        const bool notYetAnnounced = otherTask.second.timePoint < 0ns;
        const auto progress = exactMode ? PeerProgressUnlocked(otherTask.first, otherTask.second)
                                        : otherTask.second.timePoint;
        const auto latency = exactMode ? PeerLatencyUnlocked(otherTask.first) : 0ns;
        // Exact mode: everything the peer sends from now on is due at progress + latency or later, so the
        // whole step has to fit below that. Otherwise: the classic rule, widened by the run-ahead.
        const bool tooFarAhead =
            latency > 0ns ? _myNextTask.timePoint + _myNextTask.duration > progress + latency + maxRunAhead
                          : _myNextTask.timePoint > progress + maxRunAhead;
        if (notYetAnnounced || tooFarAhead)
        {
            _logger->MakeMessage(Logging::Level::Debug, TopicOf(*this))
                .SetMessage("Not advancing because participant \'{}\' has lower timepoint {}", otherTask.first,
                            otherTask.second.timePoint.count())
                .Dispatch();
            return true;
        }
    }
    return false;
}

void TimeConfiguration::SetMaxRunAhead(std::chrono::nanoseconds maxRunAhead)
{
    Lock lock{_mx};
    if (maxRunAhead < 0ns)
    {
        throw SilKitError("Attempted to set a negative MaxRunAhead.");
    }
    _maxRunAhead = maxRunAhead;
}

auto TimeConfiguration::EffectiveMaxRunAhead() const -> std::chrono::nanoseconds
{
    Lock lock{_mx};
    return EffectiveMaxRunAheadUnlocked();
}

auto TimeConfiguration::EffectiveMaxRunAheadUnlocked() const -> std::chrono::nanoseconds
{
    return _dynamicStepSizeEnabled ? 0ns : _maxRunAhead;
}

bool TimeConfiguration::ShouldAnnounceNextSimStep(std::chrono::nanoseconds lastAnnouncedTimePoint) const
{
    Lock lock{_mx};
    if (lastAnnouncedTimePoint < 0ns || (!IsExactModeEnabledUnlocked() && EffectiveMaxRunAheadUnlocked() == 0ns))
    {
        return true;
    }

    // The smallest window any peer has with respect to us; we use our own latencies for the peers' ones
    auto window = std::chrono::nanoseconds::max();
    for (const auto& otherTask : _otherNextTasks)
    {
        window = std::min(window, PeerLatencyUnlocked(otherTask.first) + EffectiveMaxRunAheadUnlocked());
    }
    if (_otherNextTasks.empty())
    {
        window = EffectiveMaxRunAheadUnlocked() + LatencyOfUnlocked({});
    }

    if (window == 0ns || lastAnnouncedTimePoint < 0ns)
    {
        return true;
    }
    if (_myNextTask.timePoint - lastAnnouncedTimePoint >= window / 2)
    {
        return true;
    }
    return OtherParticipantHasLowerTimepointUnlocked();
}

void TimeConfiguration::SetLatencies(std::chrono::nanoseconds latency,
                                     std::map<std::string, std::chrono::nanoseconds> perNetwork)
{
    Lock lock{_mx};
    if (latency < 0ns)
    {
        throw SilKitError("Attempted to set a negative Latency.");
    }
    for (const auto& entry : perNetwork)
    {
        if (entry.second < 0ns)
        {
            throw SilKitError("Attempted to set a negative latency for network " + entry.first + ".");
        }
    }
    _latency = latency;
    _networkLatencies = std::move(perNetwork);
    _peerLatencyCache.clear();
}

auto TimeConfiguration::LatencyOf(const std::string& linkName) const -> std::chrono::nanoseconds
{
    Lock lock{_mx};
    return LatencyOfUnlocked(linkName);
}

auto TimeConfiguration::LatencyOfUnlocked(const std::string& linkName) const -> std::chrono::nanoseconds
{
    if (_dynamicStepSizeEnabled)
    {
        return 0ns;
    }
    const auto it = _networkLatencies.find(linkName);
    return it != _networkLatencies.end() ? it->second : _latency;
}

bool TimeConfiguration::IsExactModeEnabled() const
{
    Lock lock{_mx};
    return IsExactModeEnabledUnlocked();
}

bool TimeConfiguration::IsExactModeEnabledUnlocked() const
{
    if (_dynamicStepSizeEnabled)
    {
        return false;
    }
    if (_latency > 0ns)
    {
        return true;
    }
    for (const auto& entry : _networkLatencies)
    {
        if (entry.second > 0ns)
        {
            return true;
        }
    }
    return false;
}

auto TimeConfiguration::MinimalLatency() const -> std::chrono::nanoseconds
{
    Lock lock{_mx};
    auto minimal = _latency;
    for (const auto& entry : _networkLatencies)
    {
        minimal = std::min(minimal, entry.second);
    }
    return minimal;
}

bool TimeConfiguration::AddPeerNetwork(const std::string& participantName, const std::string& network)
{
    Lock lock{_mx};
    _peerLatencyCache.erase(participantName);
    return _peerNetworks[participantName].insert(network).second;
}

bool TimeConfiguration::RaiseSendTimeBound(const std::string& participantName, std::chrono::nanoseconds sendTime)
{
    Lock lock{_mx};
    auto it = _sendTimeBounds.find(participantName);
    if (it == _sendTimeBounds.end())
    {
        _sendTimeBounds.emplace(participantName, sendTime);
        return true;
    }
    if (sendTime > it->second)
    {
        it->second = sendTime;
        return true;
    }
    return false;
}

auto TimeConfiguration::PeerLatencyUnlocked(const std::string& participantName) const -> std::chrono::nanoseconds
{
    if (_dynamicStepSizeEnabled)
    {
        return 0ns;
    }
    const auto cached = _peerLatencyCache.find(participantName);
    if (cached != _peerLatencyCache.end())
    {
        return cached->second;
    }
    auto latency = _latency;
    const auto it = _peerNetworks.find(participantName);
    if (it != _peerNetworks.end() && !it->second.empty())
    {
        latency = std::chrono::nanoseconds::max();
        for (const auto& network : it->second)
        {
            latency = std::min(latency, LatencyOfUnlocked(network));
        }
    }
    _peerLatencyCache.emplace(participantName, latency);
    return latency;
}

auto TimeConfiguration::PeerProgressUnlocked(const std::string& participantName, const NextSimTask& announced) const
    -> std::chrono::nanoseconds
{
    const auto it = _sendTimeBounds.find(participantName);
    return it != _sendTimeBounds.end() ? std::max(announced.timePoint, it->second) : announced.timePoint;
}

void TimeConfiguration::Initialize()
{
    Lock lock{_mx};
    _currentTask.timePoint = -1ns;
    _currentTask.duration = 0ns;
    _myNextTask.timePoint = 0ns;
    _hoppedOn = false;
}

bool TimeConfiguration::IsBlocking() const
{
    return _blocking;
}

bool TimeConfiguration::HoppedOn()
{
    return _hoppedOn;
}

bool TimeConfiguration::IsHopOn()
{
    Lock lock{_mx};

    if (_currentTask.timePoint == -1ns) // On initial time
    {
        std::chrono::nanoseconds minimalOtherTime = std::chrono::nanoseconds::max();
        // At the start, peers may legitimately have run ahead of us by up to their window
        const auto maxRunAhead = EffectiveMaxRunAheadUnlocked();
        for (const auto& otherTask : _otherNextTasks)
        {
            const auto window = maxRunAhead + PeerLatencyUnlocked(otherTask.first);
            // Any other participant has already advanced further that its duration -> HopOn
            if (otherTask.second.timePoint > otherTask.second.duration + window)
            {
                _hoppedOn = true;
                if (otherTask.second.timePoint < minimalOtherTime)
                {
                    minimalOtherTime = otherTask.second.timePoint;
                }
            }
        }
        if (_hoppedOn)
        {
            _myNextTask.timePoint = minimalOtherTime;
            _logger->MakeMessage(Logging::Level::Debug, TopicOf(*this))
                .SetMessage("Simulation time already advanced. Starting at {}ns", _myNextTask.timePoint.count())
                .Dispatch();
            return true;
        }
    }
    return false;
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
