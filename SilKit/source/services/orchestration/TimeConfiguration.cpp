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

void TimeConfiguration::SynchronizedParticipantRemoved(const std::string& otherParticipantName)
{
    Lock lock{_mx};
    if (_otherNextTasks.find(otherParticipantName) != _otherNextTasks.end())
    {
        const std::string errorMessage{"Participant " + otherParticipantName + " unknown."};
        throw SilKitError{errorMessage};
    }
    auto it = _otherNextTasks.find(otherParticipantName);
    if (it != _otherNextTasks.end())
    {
        _otherNextTasks.erase(it);
    }
}
void TimeConfiguration::SetStepDuration(std::chrono::nanoseconds duration)
{
    Lock lock{_mx};
    _myNextTask.duration = duration;
}

void TimeConfiguration::AdvanceTimeStep()
{
    Lock lock{_mx};
    _currentTask = _myNextTask;
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
            _logger->MakeMessage(Logging::Level::Debug, TopicOf(*this))
                .SetMessage("Not advancing because participant \'{}\' has lower timepoint {}", otherTask.first,
                            otherTask.second.timePoint.count())
                .Dispatch();
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
