// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>
#include <cstdint>
#include <string>
#include <unordered_map>

#include "services/logging/ILoggerInternal.hpp"

#include "dashboard/DashboardBulkUpdate.hpp"
#include "dashboard/IRestClient.hpp"
#include "dashboard/LockedQueue.hpp"
#include "dashboard/SilKitEvent.hpp"

namespace VSilKit {

/*! Drains the dashboard event queue, batches the events per simulation and issues the requests.
 *
 *  Normally run on its own thread by DashboardInstance, which owns everything referenced here and
 *  joins the thread before those objects die. All of the worker's own state is local to
 *  ProcessEvents(), so an instance is cheap and holds nothing between runs.
 *
 *  Batching: DequeueAllInto() blocks until at least one event is available, then takes everything
 *  queued so far. Those events are folded into one DashboardBulkUpdate per simulation, and the
 *  accumulated updates are flushed once at the end of the batch. Batch size is therefore whatever
 *  accumulated while the previous batch was being sent - it self-tunes, with no timer.
 *
 *  Two events are not batched: OnSimulationStart, which must first obtain the simulation id that
 *  every other event needs, and OnMetricUpdate, which has its own endpoint.
 */
class EventQueueWorkerThread
{
public:
    EventQueueWorkerThread(SilKit::Services::Logging::ILoggerInternal* logger, IRestClient* dashboardRestClient,
                           LockedQueue<SilKitEvent>* eventQueue, const std::atomic<bool>* abort);

    //! Runs until the queue is stopped or the abort flag is set. Never throws.
    void operator()() const;

private:
    auto IsAborted() const -> bool;

    /*! Sends every non-empty accumulated update.
     *
     *  An update carrying `stopped` is the last one for that simulation, so its entry is dropped
     *  afterwards instead of being kept around empty for the rest of the process's life.
     */
    void FlushAccumulated(std::unordered_map<uint64_t, SilKit::Dashboard::DashboardBulkUpdate>& bulkUpdates) const;

    void ProcessEvents() const;

    SilKit::Services::Logging::ILoggerInternal* _logger{nullptr};
    IRestClient* _dashboardRestClient{nullptr};
    LockedQueue<SilKitEvent>* _eventQueue{nullptr};
    const std::atomic<bool>* _abort{nullptr};
};

} // namespace VSilKit
