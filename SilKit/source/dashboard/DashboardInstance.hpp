// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "IDashboardInstance.hpp"
#include "VAsioRegistry.hpp"

#include "LoggerMessage.hpp"

#include "silkit/config/IParticipantConfiguration.hpp"

#include "CachingSilKitEventHandler.hpp"
#include "IDashboard.hpp"
#include "OatppHeaders.hpp"
#include "Client/DashboardSystemApiClient.hpp"
#include "Service/ISilKitToOatppMapper.hpp"
#include "SystemStateTracker.hpp"
#include "DashboardRetryPolicy.hpp"

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
    void SetupDashboardConnection(std::string const& dashboardUri) override;

private:
    auto GetOrCreateSimulationData(const std::string& simulationName) -> SimulationData&;
    void RemoveSimulationData(const std::string& simulationName);

private:
    void RunEventQueueWorkerThread();
    void RunBulkUpdateEventQueueWorkerThread();

private: // SilKit::Core::IRegistryEventListener
    void OnLoggerInternalCreated(SilKit::Services::Logging::ILoggerInternal* logger) override;
    void OnRegistryUri(const std::string& registryUri) override;
    void OnParticipantConnected(std::string const& simulationName, std::string const& participantName) override;
    void OnParticipantDisconnected(std::string const& simulationName, std::string const& participantName) override;
    void OnRequiredParticipantsUpdate(const std::string& simulationName, const std::string& participantName,
                                      SilKit::Util::Span<const std::string> requiredParticipantNames) override;
    void OnParticipantStatusUpdate(
        std::string const& simulationName, std::string const& participantName,
        SilKit::Services::Orchestration::ParticipantStatus const& participantStatus) override;
    void OnServiceDiscoveryEvent(std::string const& simulationName, std::string const& participantName,
                                 SilKit::Core::Discovery::ServiceDiscoveryEvent const& serviceDiscoveryEvent) override;

private:
    /// Assigned in OnLoggerCreated
    SilKit::Services::Logging::ILoggerInternal* _logger{nullptr};
    /// Assigned in OnRegistryUri
    std::unique_ptr<SilKit::Core::Uri> _registryUri;

    std::shared_ptr<oatpp::data::mapping::ObjectMapper> _objectMapper;
    std::shared_ptr<SilKit::Dashboard::DashboardRetryPolicy> _retryPolicy;
    std::shared_ptr<SilKit::Dashboard::DashboardSystemApiClient> _apiClient;
    std::shared_ptr<SilKit::Dashboard::ISilKitToOatppMapper> _silKitToOatppMapper;
    std::shared_ptr<SilKit::Dashboard::ISilKitEventHandler> _silKitEventHandler;
    std::shared_ptr<SilKit::Dashboard::ISilKitEventQueue> _silKitEventQueue;

    std::thread _eventQueueWorkerThread;
    std::promise<void> _eventQueueWorkerThreadAbort;

    std::unordered_map<std::string, SimulationData> _simulationEventHandlers;
};

} // namespace VSilKit
