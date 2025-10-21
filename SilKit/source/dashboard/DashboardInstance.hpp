// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IDashboardInstance.hpp"
#include "VAsioRegistry.hpp"

#include "LoggerMessage.hpp"

#include "client/DashboardSystemApiClient.hpp"
#include "service/ISilKitToOatppMapper.hpp"
#include "SystemStateTracker.hpp"
#include "IRestClient.hpp"

#include "LockedQueue.hpp"
#include "SilKitEvent.hpp"

#include <chrono>
#include <string>
#include <memory>
#include <thread>
#include <unordered_map>


namespace VSilKit {

class DashboardInstance final
    : public IDashboardInstance
    , private SilKit::Core::IRegistryEventListener
{
    struct SimulationData
    {
        SystemStateTracker systemStateTracker;
    };

public:
    explicit DashboardInstance();

    DashboardInstance(const DashboardInstance&) = delete;
    DashboardInstance(DashboardInstance&&) = delete;

    DashboardInstance& operator=(const DashboardInstance&) = delete;
    DashboardInstance& operator=(DashboardInstance&&) = delete;

    ~DashboardInstance() override;

    auto GetRegistryEventListener() -> SilKit::Core::IRegistryEventListener* override;
    void SetupDashboardConnection(const std::string& dashboardUri) override;

private:
    auto GetOrCreateSimulationData(const std::string& simulationName) -> SimulationData&;
    void RemoveSimulationData(const std::string& simulationName);

private:
    void RunEventQueueWorkerThread();

private: // SilKit::Core::IRegistryEventListener
    void OnLoggerCreated(SilKit::Services::Logging::ILogger* logger) override;
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
    /// Assigned in OnLoggerCreated
    SilKit::Services::Logging::ILogger* _logger{nullptr};
    /// Assigned in OnRegistryUri
    std::unique_ptr<SilKit::Core::Uri> _registryUri;

    std::shared_ptr<IRestClient> _dashboardRestClient;
    LockedQueue<SilKitEvent> _silKitEventQueue;

    std::thread _eventQueueWorkerThread;
    std::promise<void> _eventQueueWorkerThreadAbort;

    std::unordered_map<std::string, SimulationData> _simulationEventHandlers;
};

} // namespace VSilKit
