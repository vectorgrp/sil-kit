/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "TimeConfiguration.hpp"
#include "ILogger.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

TimeConfiguration::TimeConfiguration(Logging::ILogger* logger) 
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
    for (auto const& it : _otherNextTasks)
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
        Logging::Error(_logger,
                       "Chonology error: Received NextSimTask from participant \'{}\' with lower timePoint {} than last "
                       "known timePoint {}",
                       participantName, nextStep.timePoint.count(), itOtherNextTask->second.timePoint.count());
    }

    _otherNextTasks.at(participantName) = std::move(nextStep);
    Logging::Debug(_logger, "Updated _otherNextTasks for participant {} with time {}", participantName,
                   nextStep.timePoint.count());
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

bool TimeConfiguration::HandleHopOn()
{
    // HopOn can happen only once
    if (!_hoppedOn)
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
    }
    return false;
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
