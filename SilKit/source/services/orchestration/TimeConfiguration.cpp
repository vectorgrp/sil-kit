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

namespace SilKit {
namespace Services {
namespace Orchestration {

TimeConfiguration::TimeConfiguration() 
    : _blocking(false)
{
    Initialize();
    // NB: This is used when SetPeriod is never called
    _myNextTask.duration = 1ms;
}

void TimeConfiguration::SetBlockingMode(bool blocking)
{
    _blocking = blocking;
}

void TimeConfiguration::SynchronizedParticipantAdded(const std::string& otherParticipantName)
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
    _otherNextTasks[otherParticipantName] = task;
}

void TimeConfiguration::OnReceiveNextSimStep(const std::string& participantName, NextSimTask nextStep)
{
    Lock lock{_mx};
    _otherNextTasks[participantName] = std::move(nextStep);
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
            return true;
    }
    return false;
}

void TimeConfiguration::Initialize()
{
    Lock lock{_mx};
    _currentTask.timePoint = -1ns;
    _currentTask.duration = 0ns;
    _myNextTask.timePoint = 0ns;
}

bool TimeConfiguration::IsBlocking() const
{
    return _blocking;
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
