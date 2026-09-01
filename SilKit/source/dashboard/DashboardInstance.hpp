// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>

#include "core/vasio/IRegistryEventListener.hpp"
#include "services/orchestration/SystemStateTracker.hpp"
#include "util/Uri.hpp"

#include "dashboard/IDashboardInstance.hpp"
#include "dashboard/IRestClient.hpp"
#include "dashboard/LockedQueue.hpp"
#include "dashboard/SilKitEvent.hpp"

namespace VSilKit {

/*! Forwards what the registry reports to the SIL Kit Dashboard's REST service.
 *
 *  Two threads are involved. The registry calls the IRegistryEventListener methods below from its
 *  I/O thread; those only track per-simulation state and push onto _silKitEventQueue. A dedicated
 *  worker thread ("SK-Dash-Cons") drains that queue, batches the events per simulation and performs
 *  every HTTP request. The queue is the only synchronisation between the two.
 *
 *  IRegistryEventListener is inherited privately on purpose: the registry receives a listener
 *  pointer from GetRegistryEventListener(), but the On* methods are not part of the public
 *  IDashboardInstance surface.
 */
class DashboardInstance final
    : public IDashboardInstance
    , private SilKit::Core::IRegistryEventListener
{
public:
    //! Throws if dashboardUri is not a usable http URI, so a bad --dashboard-uri fails at creation.
    explicit DashboardInstance(const std::string& dashboardUri);

    DashboardInstance(const DashboardInstance&) = delete;
    DashboardInstance(DashboardInstance&&) = delete;

    DashboardInstance& operator=(const DashboardInstance&) = delete;
    DashboardInstance& operator=(DashboardInstance&&) = delete;

    ~DashboardInstance() override;

    auto GetRegistryEventListener() -> SilKit::Core::IRegistryEventListener* override;

private:
    /*! Connects to the dashboard and starts the worker thread.
     *
     *  Deferred until OnRegistryUri because the REST client needs the logger from OnLoggerCreated
     *  and the events need the registry URI. Called once.
     */
    void StartWorker();

private: // SilKit::Core::IRegistryEventListener
    void OnLoggerCreated(SilKit::Services::Logging::ILoggerInternal* logger) override;
    void OnRegistryUri(const std::string& registryUri) override;
    void OnParticipantConnected(const std::string& simulationName, const std::string& participantName) override;
    void OnParticipantDisconnected(const std::string& simulationName, const std::string& participantName) override;
    void OnRequiredParticipantsUpdate(const std::string& simulationName, const std::string& participantName,
                                      SilKit::Util::Span<const std::string> requiredParticipantNames) override;
    void OnParticipantStatusUpdate(
        const std::string& simulationName, const std::string& participantName,
        const SilKit::Services::Orchestration::ParticipantStatus& participantStatus) override;
    void OnServiceDiscoveryEvent(const std::string& simulationName, const std::string& participantName,
                                 const SilKit::Core::Discovery::ServiceDiscoveryEvent& serviceDiscoveryEvent) override;
    void OnMetricsUpdate(const std::string& simulationName, const std::string& origin,
                         const VSilKit::MetricsUpdate& metricsUpdate) override;

private:
    const std::string _dashboardUri;

    /// Assigned in OnLoggerCreated
    SilKit::Services::Logging::ILoggerInternal* _logger{nullptr};
    /// Assigned in OnRegistryUri
    std::optional<SilKit::Core::Uri> _registryUri;

    std::unique_ptr<IRestClient> _dashboardRestClient;
    LockedQueue<SilKitEvent> _silKitEventQueue;

    std::thread _eventQueueWorkerThread;
    /// Read by the worker thread, set by the destructor.
    std::atomic<bool> _abortWorker{false};

    /// One tracker per live simulation, touched only from the registry's thread.
    std::unordered_map<std::string, SystemStateTracker> _systemStateTrackers;
};

} // namespace VSilKit
