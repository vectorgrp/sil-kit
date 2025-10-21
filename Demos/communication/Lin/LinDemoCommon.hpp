// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/services/string_utils.hpp"
#include "silkit/services/lin/all.hpp"
#include "silkit/services/lin/string_utils.hpp"
#include "silkit/services/logging/ILogger.hpp"

using namespace SilKit::Services::Lin;

// This is the common behavior used in LinSlaveDemo and LinMasterDemo
namespace LinDemoCommon {

class Timer
{
public:
    void Set(std::chrono::nanoseconds timeOut, std::function<void(std::chrono::nanoseconds)> action) noexcept
    {
        auto lock = Lock();

        _isActive = true;
        _timeOut = timeOut;
        _action = std::move(action);
    }

    void Clear() noexcept
    {
        auto lock = Lock();

        _isActive = false;
        _timeOut = std::chrono::nanoseconds::max();
        _action = std::function<void(std::chrono::nanoseconds)>{};
    }

    auto ExecuteIfDue(std::chrono::nanoseconds now) -> bool
    {
        std::function<void(std::chrono::nanoseconds)> action;

        {
            auto lock = Lock();

            if (!_isActive || (now < _timeOut))
            {
                return false;
            }

            action = std::move(_action);

            // Clear
            _isActive = false;
            _timeOut = std::chrono::nanoseconds::max();
            _action = std::function<void(std::chrono::nanoseconds)>{};
        }

        action(now);

        return true;
    }

private:
    bool _isActive = false;
    std::chrono::nanoseconds _timeOut = std::chrono::nanoseconds::max();
    std::function<void(std::chrono::nanoseconds)> _action;

    std::mutex _mutex;
    auto Lock() -> std::unique_lock<decltype(_mutex)>
    {
        return std::unique_lock<decltype(_mutex)>{_mutex};
    }
};

class Schedule
{
public:
    Schedule(
        std::initializer_list<std::pair<std::chrono::nanoseconds, std::function<void(std::chrono::nanoseconds)>>> tasks,
        bool autoScheduleNext = true)
        : _autoScheduleNext{autoScheduleNext}
    {
        for (auto&& task : tasks)
        {
            _schedule.emplace_back(task.first, task.second);
        }
        Reset();
    }

    void Reset()
    {
        _nextTask = _schedule.begin();
        ScheduleNextTask();
    }

    void ExecuteTask(std::chrono::nanoseconds now)
    {
        _now = now;
        if (_timer.ExecuteIfDue(now) && _autoScheduleNext)
        {
            ScheduleNextTask();
        }
    }

    void ScheduleNextTask()
    {
        auto currentTask = _nextTask++;
        if (_nextTask == _schedule.end())
        {
            _nextTask = _schedule.begin();
        }

        _timer.Set(_now + currentTask->delay, currentTask->action);
    }

private:
    struct Task
    {
        Task(std::chrono::nanoseconds delay, std::function<void(std::chrono::nanoseconds)> action)
            : delay{delay}
            , action{action}
        {
        }

        std::chrono::nanoseconds delay;
        std::function<void(std::chrono::nanoseconds)> action;
    };

    Timer _timer;
    std::vector<Task> _schedule;
    std::vector<Task>::iterator _nextTask;
    std::chrono::nanoseconds _now = 0ns;
    bool _autoScheduleNext;
};

} // namespace LinDemoCommon
