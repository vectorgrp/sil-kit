// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "dashboard/EventQueueWorkerThread.hpp"

#include <vector>

#include "core/internal/traits/SilKitLoggingTraits.hpp"
#include "services/logging/LoggerMessage.hpp"

namespace VSilKit {

using SilKit::Services::Logging::ILoggerInternal;
using SilKit::Services::Logging::Level;

EventQueueWorkerThread::EventQueueWorkerThread(ILoggerInternal* logger, IRestClient* dashboardRestClient,
                                               LockedQueue<SilKitEvent>* eventQueue,
                                               const std::atomic<bool>* abort)
    : _logger{logger}
    , _dashboardRestClient{dashboardRestClient}
    , _eventQueue{eventQueue}
    , _abort{abort}
{
}

void EventQueueWorkerThread::operator()() const
try
{
    ProcessEvents();
}
catch (const std::exception& exception)
{
    _logger->MakeMessage(Level::Error, TopicOf(*this))
        .SetMessage("Dashboard: event queue worker failed: {}", exception.what())
        .Dispatch();
}
catch (...)
{
    _logger->MakeMessage(Level::Error, TopicOf(*this))
        .SetMessage("Dashboard: event queue worker failed with unknown exception")
        .Dispatch();
}

auto EventQueueWorkerThread::IsAborted() const -> bool
{
    return _abort != nullptr && _abort->load(std::memory_order_acquire);
}

template <typename Action>
auto EventQueueWorkerThread::WithoutPropagating(const char* what, Action&& action) const -> bool
try
{
    action();
    return true;
}
catch (const std::exception& exception)
{
    _logger->MakeMessage(Level::Error, TopicOf(*this))
        .SetMessage("Dashboard: {} failed and was skipped: {}", what, exception.what())
        .Dispatch();
    return false;
}
catch (...)
{
    _logger->MakeMessage(Level::Error, TopicOf(*this))
        .SetMessage("Dashboard: {} failed and was skipped: unknown exception", what)
        .Dispatch();
    return false;
}

void EventQueueWorkerThread::FlushAccumulated(BulkUpdates& bulkUpdates) const
{
    for (auto it = bulkUpdates.begin(); it != bulkUpdates.end();)
    {
        auto& bulkUpdate = it->second;

        if (bulkUpdate.Empty())
        {
            ++it;
            continue;
        }

        const bool simulationEnded = bulkUpdate.stopped.has_value();

        // An update that cannot be sent is still cleared below: keeping it would mean retrying the
        // same unusable content on every following batch, and losing everything queued behind it.
        WithoutPropagating("sending a bulk update", [&] { _dashboardRestClient->OnBulkUpdate(it->first, bulkUpdate); });

        if (simulationEnded)
        {
            it = bulkUpdates.erase(it);
        }
        else
        {
            bulkUpdate.Clear();
            ++it;
        }
    }
}

void EventQueueWorkerThread::ProcessEvent(const SilKitEvent& event, SimulationIds& simulationIds,
                                          BulkUpdates& bulkUpdates) const
{
    // OnSimulationStart is handled separately: it establishes the simulation id that every
    // other event for that simulation needs.
    if (event.Type() == SilKitEventType::OnSimulationStart)
    {
        FlushAccumulated(bulkUpdates);

        const auto known{simulationIds.find(event.GetSimulationName())};
        if (known != simulationIds.end())
        {
            // Queuing means a simulation can be announced more than once.
            _logger->MakeMessage(Level::Debug, TopicOf(*this))
                .SetMessage("Dashboard: Simulation {} already has id {}", event.GetSimulationName(), known->second)
                .Dispatch();
            return;
        }

        const auto& simulationStart = event.GetSimulationStart();
        const auto simulationId =
            _dashboardRestClient->OnSimulationStart(simulationStart.connectUri, simulationStart.time);

        if (simulationId == 0)
        {
            _logger->MakeMessage(Level::Warn, TopicOf(*this))
                .SetMessage("Dashboard: Simulation {} could not be created", event.GetSimulationName())
                .Dispatch();
            return;
        }

        simulationIds.emplace(event.GetSimulationName(), simulationId);
        return;
    }

    const auto it{simulationIds.find(event.GetSimulationName())};
    if (it == simulationIds.end())
    {
        _logger->MakeMessage(Level::Warn, TopicOf(*this))
            .SetMessage("Dashboard: Simulation {} is unknown", event.GetSimulationName())
            .Dispatch();
        return;
    }

    const auto simulationId{it->second};
    auto& bulkUpdate{bulkUpdates[simulationId]};

    switch (event.Type())
    {
    case SilKitEventType::OnSimulationStart:
        break; // handled above

    case SilKitEventType::OnParticipantConnected:
        bulkUpdate.participantConnectionInformations.emplace_back(event.GetParticipantConnectionInformation());
        break;

    case SilKitEventType::OnSystemStateChanged:
        bulkUpdate.systemStates.emplace_back(event.GetSystemState());
        break;

    case SilKitEventType::OnParticipantStatusChanged:
        bulkUpdate.participantStatuses.emplace_back(event.GetParticipantStatus());
        break;

    case SilKitEventType::OnServiceDiscoveryEvent:
        bulkUpdate.serviceDatas.emplace_back(event.GetServiceData());
        break;

    case SilKitEventType::OnSimulationEnd:
        bulkUpdate.stopped = event.GetSimulationEnd().time;
        simulationIds.erase(it);
        break;

    case SilKitEventType::OnMetricUpdate:
    {
        // Metrics are not batched; they go out on their own endpoint immediately.
        const auto& data = event.GetMetricsUpdate();
        _dashboardRestClient->OnMetricsUpdate(simulationId, data.first, data.second);
        break;
    }
    }
}

void EventQueueWorkerThread::ProcessEvents() const
{
    SimulationIds simulationIds;
    BulkUpdates simulationBulkUpdates;

    std::vector<SilKitEvent> events;
    while (_eventQueue->DequeueAllInto(events))
    {
        for (const auto& event : events)
        {
            if (IsAborted())
            {
                // Send what has already been accumulated; the shutdown grace period exists precisely
                // so this last update still reaches the dashboard.
                FlushAccumulated(simulationBulkUpdates);
                return;
            }

            WithoutPropagating("processing an event", [&] {
                ProcessEvent(event, simulationIds, simulationBulkUpdates);
            });
        }

        events.clear();
        FlushAccumulated(simulationBulkUpdates);
    }
}

} // namespace VSilKit
