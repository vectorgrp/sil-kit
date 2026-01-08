// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "TimeConfiguration.hpp"
#include "LoggerMessage.hpp"

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

void TimeConfiguration::SetTimeAdvanceMode(TimeAdvanceMode timeAdvanceMode)
{
    _timeAdvanceMode = timeAdvanceMode;
}

auto TimeConfiguration::GetTimeAdvanceMode() const -> TimeAdvanceMode
{
    return _timeAdvanceMode;
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
        Logging::Error(_logger, "Received NextSimTask from unknown participant {}", participantName);
        return;
    }

    if (nextStep.timePoint < itOtherNextTask->second.timePoint)
    {
        Logging::Error(
            _logger,
            "Chonology error: Received NextSimTask from participant \'{}\' with lower timePoint {} than last "
            "known timePoint {}",
            participantName, nextStep.timePoint.count(), itOtherNextTask->second.timePoint.count());
    }

    _otherNextTasks.at(participantName) = nextStep;
    Logging::Debug(_logger, "Updated _otherNextTasks for participant {} with time {}", participantName,
                   nextStep.timePoint.count());
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
        if (nextStepStart > _currentTask.timePoint && nextStepStart < earliestOtherTimepoint)
        {
            earliestOtherTimepoint = nextStepStart;
        }
        else if (nextStepEnd < earliestOtherTimepoint)
        {
            earliestOtherTimepoint = nextStepEnd;
        }
    }
    //Logging::Info(_logger, "Earliest next timepoint among other participants is {}ms",
    //              std::chrono::duration_cast<std::chrono::milliseconds>(earliestOtherTimepoint).count());

    auto minAlignedDuration = earliestOtherTimepoint - _currentTask.timePoint;

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
    
    if (_timeAdvanceMode == TimeAdvanceMode::ByMinimalDuration)
    {
        auto minAlignedDuration = GetMinimalAlignedDuration();
        if (minAlignedDuration < _currentTask.duration)
        {
            //Logging::Info(_logger, "Adjusting my step duration from {}ms to {}ms",
            //              std::chrono::duration_cast<std::chrono::milliseconds>(_currentTask.duration).count(),
            //              std::chrono::duration_cast<std::chrono::milliseconds>(minAlignedDuration).count());
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

    for (const auto& otherTask : _otherNextTasks)
    {
        if (_myNextTask.timePoint > otherTask.second.timePoint)
        {
            Debug(_logger, "Not advancing because participant \'{}\' has lower timepoint {}", otherTask.first,
                  otherTask.second.timePoint.count());
            return true;
        }
    }
    return false;
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
        for (const auto& otherTask : _otherNextTasks)
        {
            // Any other participant has already advanced further that its duration -> HopOn
            if (otherTask.second.timePoint > otherTask.second.duration)
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
            Logging::Debug(_logger, "Simulation time already advanced. Starting at {}ns",
                           _myNextTask.timePoint.count());
            return true;
        }
    }
    return false;
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
