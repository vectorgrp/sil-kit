// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <queue>
#include <functional>
#include <chrono>

class Scheduler
{
public:
    Scheduler();

    // Register an event with a timestamp and a callback function
    void ScheduleEvent(std::chrono::nanoseconds delta, const std::function<void()>& callback);

    // Run the scheduler loop to check for due events
    void OnSimulationStep(std::chrono::nanoseconds now);

    std::chrono::nanoseconds Now()
    {
        return _now;
    }

    struct Event
    {
        std::chrono::nanoseconds timestamp;
        std::function<void()> callback;
    };

private:
    // Custom comparison function for the priority queue
    struct CompareEvent
    {
        bool operator()(const Event& e1, const Event& e2)
        {
            return e1.timestamp > e2.timestamp;
        }
    };

    std::priority_queue<Event, std::vector<Event>, CompareEvent> events;
    std::chrono::nanoseconds _now{0};
};
