// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "Scheduler.hpp"

Scheduler::Scheduler() : _now{0}
{}

void Scheduler::ScheduleEvent(std::chrono::nanoseconds delta, const std::function<void()>& callback)
{
    events.push({_now + delta, callback});
}

void Scheduler::OnSimulationStep(std::chrono::nanoseconds now)
{
    _now = now; 

    while (!events.empty() && events.top().timestamp <= now)
    {
        Event event = events.top();
        events.pop();
        if (event.callback)
        {
            event.callback();
        }
    }
}

