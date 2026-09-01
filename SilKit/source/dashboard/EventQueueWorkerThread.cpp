// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "dashboard/EventQueueWorkerThread.hpp"

#include <vector>

#include "core/internal/traits/SilKitLoggingTraits.hpp"
#include "services/logging/LoggerMessage.hpp"

namespace VSilKit {

using SilKit::Dashboard::DashboardBulkUpdate;
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

void EventQueueWorkerThread::FlushAccumulated(std::unordered_map<uint64_t, DashboardBulkUpdate>& bulkUpdates) const
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
        _dashboardRestClient->OnBulkUpdate(it->first, bulkUpdate);

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

void EventQueueWorkerThread::ProcessEvents() const
{
    std::unordered_map<std::string, uint64_t> simulationNameToId;
    std::unordered_map<uint64_t, DashboardBulkUpdate> simulationBulkUpdates;

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

            // OnSimulationStart is handled separately: it establishes the simulation id that every
            // other event for that simulation needs.
            if (event.Type() == SilKitEventType::OnSimulationStart)
            {
                FlushAccumulated(simulationBulkUpdates);

                const auto known{simulationNameToId.find(event.GetSimulationName())};
                if (known != simulationNameToId.end())
                {
                    // Queuing means a simulation can be announced more than once.
                    _logger->MakeMessage(Level::Debug, TopicOf(*this))
                        .SetMessage("Dashboard: Simulation {} already has id {}", event.GetSimulationName(),
                                    known->second)
                        .Dispatch();
                    continue;
                }

                const auto& simulationStart = event.GetSimulationStart();
                const auto simulationId =
                    _dashboardRestClient->OnSimulationStart(simulationStart.connectUri, simulationStart.time);

                if (simulationId == 0)
                {
                    _logger->MakeMessage(Level::Warn, TopicOf(*this))
                        .SetMessage("Dashboard: Simulation {} could not be created", event.GetSimulationName())
                        .Dispatch();
                    continue;
                }

                simulationNameToId.emplace(event.GetSimulationName(), simulationId);
                continue;
            }

            const auto it{simulationNameToId.find(event.GetSimulationName())};
            if (it == simulationNameToId.end())
            {
                _logger->MakeMessage(Level::Warn, TopicOf(*this))
                    .SetMessage("Dashboard: Simulation {} is unknown", event.GetSimulationName())
                    .Dispatch();
                continue;
            }

            const auto simulationId{it->second};
            auto& bulkUpdate{simulationBulkUpdates[simulationId]};

            switch (event.Type())
            {
            case SilKitEventType::OnSimulationStart:
                break; // handled above

            case SilKitEventType::OnParticipantConnected:
                bulkUpdate.participantConnectionInformations.emplace_back(
                    event.GetParticipantConnectionInformation());
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
                simulationNameToId.erase(it);
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

        events.clear();
        FlushAccumulated(simulationBulkUpdates);
    }
}

} // namespace VSilKit
