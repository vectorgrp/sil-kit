// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "DashboardInstance.hpp"

#include "DashboardComponents.hpp"
#include "DashboardRetryPolicy.hpp"
#include "DashboardSystemApiClient.hpp"
#include "Client/DashboardSystemServiceClient.hpp"

#include "SilKitEvent.hpp"
#include "SilKitEventHandler.hpp"
#include "SilKitEventQueue.hpp"
#include "SilKitToOatppMapper.hpp"


namespace Log = SilKit::Services::Logging;


namespace {


uint64_t GetCurrentTime()
{
    auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}

bool ShouldSkipServiceDiscoveryEvent(const SilKit::Core::Discovery::ServiceDiscoveryEvent &serviceDiscoveryEvent)
{
    return serviceDiscoveryEvent.type != SilKit::Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated
           || (serviceDiscoveryEvent.serviceDescriptor.GetServiceType() != SilKit::Core::ServiceType::Controller
               && serviceDiscoveryEvent.serviceDescriptor.GetServiceType() != SilKit::Core::ServiceType::Link);
}


using SilKit::Dashboard::SilKitEvent;
using SilKit::Dashboard::SimulationStart;
using SilKit::Dashboard::SimulationEnd;
using SilKit::Dashboard::ServiceData;


} // namespace


namespace VSilKit {


DashboardInstance::DashboardInstance()
{
    oatpp::base::Environment::init();
}

DashboardInstance::~DashboardInstance()
{
    if (_retryPolicy != nullptr)
    {
        _retryPolicy->AbortAllRetries();
    }

    try
    {
        _eventQueueWorkerThreadAbort.set_value();
    }
    catch (...)
    {
        // ignored
    }

    if (_silKitEventQueue != nullptr)
    {
        _silKitEventQueue->Stop();
    }

    if (_eventQueueWorkerThread.joinable())
    {
        _eventQueueWorkerThread.join();
    }

    oatpp::base::Environment::destroy();
}

auto DashboardInstance::GetRegistryEventListener() -> SilKit::Core::IRegistryEventListener *
{
    return this;
}

void DashboardInstance::SetupDashboardConnection(std::string const &dashboardUri)
{
    auto uri = SilKit::Core::Uri::Parse(dashboardUri);
    SilKit::Dashboard::DashboardComponents dashboardComponents{uri.Host(), uri.Port()};

    _objectMapper = OATPP_GET_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>);

    _retryPolicy = std::make_shared<SilKit::Dashboard::DashboardRetryPolicy>(3);

    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ClientConnectionProvider>, connectionProvider);
    auto requestExecutor = oatpp::web::client::HttpRequestExecutor::createShared(connectionProvider, _retryPolicy);

    _apiClient = SilKit::Dashboard::DashboardSystemApiClient::createShared(requestExecutor, _objectMapper);
    _silKitToOatppMapper = std::make_shared<SilKit::Dashboard::SilKitToOatppMapper>();

    auto serviceClient =
        std::make_shared<SilKit::Dashboard::DashboardSystemServiceClient>(_logger, _apiClient, _objectMapper);

    _silKitEventHandler =
        std::make_shared<SilKit::Dashboard::SilKitEventHandler>(_logger, serviceClient, _silKitToOatppMapper);
    _silKitEventQueue = std::make_shared<SilKit::Dashboard::SilKitEventQueue>();

    RunEventQueueWorkerThread();
}

struct EventQueueWorkerThread
{
    SilKit::Services::Logging::ILogger *logger{nullptr};
    std::shared_ptr<SilKit::Dashboard::DashboardSystemApiClient> apiClient;
    std::shared_ptr<SilKit::Dashboard::ISilKitEventHandler> eventHandler;
    std::shared_ptr<SilKit::Dashboard::ISilKitEventQueue> eventQueue;
    std::future<void> abort;

    auto DetectBulkUpdate() const -> bool
    {
        bool bulkUpdateAvailable = false;

        auto bulkSimulationDto = SilKit::Dashboard::BulkSimulationDto::createShared();
        const auto response = apiClient->updateSimulation(oatpp::UInt64{std::uint64_t{0}}, bulkSimulationDto);

        if (response)
        {
            const auto statusCode = response->getStatusCode();
            bulkUpdateAvailable = 200 <= statusCode && statusCode < 300;
        }

        if (bulkUpdateAvailable)
        {
            logger->Debug("Dashboard bulk-updates are available");
        }
        else
        {
            logger->Debug("Dashboard bulk-updates are not available, falling back to individual requests");
        }

        return bulkUpdateAvailable;
    }

    void ProcessEventsWithBulkUpdates() const
    {
        using SilKitEventType = SilKit::Dashboard::SilKitEventType;

        std::unordered_map<std::string, uint64_t> simulationNameToId;
        std::unordered_map<uint64_t, SilKit::Dashboard::DashboardBulkUpdate> simulationBulkUpdates;

        std::vector<SilKit::Dashboard::SilKitEvent> events;
        while (eventQueue->DequeueAllInto(events))
        {
            const auto ProcessAllAccumulatedBulkUpdates = [this, &simulationBulkUpdates] {
                for (auto &pair : simulationBulkUpdates)
                {
                    const auto simulationId = pair.first;
                    auto &bulkUpdate = pair.second;

                    if (bulkUpdate.Empty())
                    {
                        continue;
                    }

                    eventHandler->OnBulkUpdate(simulationId, bulkUpdate);
                    bulkUpdate.Clear();
                }
            };

            for (const auto &event : events)
            {
                if (!abort.valid() || abort.wait_for(std::chrono::seconds{}) != std::future_status::timeout)
                {
                    return;
                }

                // process OnSimulationStart separately, which creates the simulation-id for a simulation name

                if (event.Type() == SilKitEventType::OnSimulationStart)
                {
                    ProcessAllAccumulatedBulkUpdates();

                    const auto it{simulationNameToId.find(event.GetSimulationName())};
                    if (it != simulationNameToId.end())
                    {
                        // it is possible that multiple SimulationStart events are created (due to the queuing)
                        Log::Debug(logger, "Dashboard: Simulation {} already has id {}", event.GetSimulationName(),
                                   it->second);
                        continue;
                    }

                    const auto &simulationStart = event.GetSimulationStart();
                    const auto simulationId =
                        eventHandler->OnSimulationStart(simulationStart.connectUri, simulationStart.time);

                    if (simulationId == 0)
                    {
                        Log::Warn(logger, "Dashboard: Simulation {} could not be created", event.GetSimulationName());
                        continue;
                    }

                    simulationNameToId.emplace(event.GetSimulationName(), simulationId);

                    continue;
                }

                // fetch the simulation id for the given name

                const auto it{simulationNameToId.find(event.GetSimulationName())};
                if (it == simulationNameToId.end())
                {
                    Log::Warn(logger, "Dashboard: Simulation {} is unknown", event.GetSimulationName());
                    continue;
                }

                const auto simulationId{it->second};
                auto &bulkUpdate{simulationBulkUpdates[simulationId]};

                // process all event types, except OnSimulationStart

                switch (event.Type())
                {
                case SilKitEventType::OnParticipantConnected:
                {
                    const auto &participantConnectionInformation = event.GetParticipantConnectionInformation();
                    bulkUpdate.participantConnectionInformations.emplace_back(participantConnectionInformation);
                }
                break;

                case SilKitEventType::OnSystemStateChanged:
                {
                    const auto &systemState = event.GetSystemState();
                    bulkUpdate.systemStates.emplace_back(systemState);
                }
                break;

                case SilKitEventType::OnParticipantStatusChanged:
                {
                    const auto &participantStatus = event.GetParticipantStatus();
                    bulkUpdate.participantStatuses.emplace_back(participantStatus);
                }
                break;

                case SilKitEventType::OnServiceDiscoveryEvent:
                {
                    const auto &serviceData = event.GetServiceData();
                    bulkUpdate.serviceDatas.emplace_back(serviceData);
                }
                break;

                case SilKitEventType::OnSimulationEnd:
                {
                    const auto &simulationEnd = event.GetSimulationEnd();
                    bulkUpdate.stopped = std::make_unique<uint64_t>(simulationEnd.time);

                    simulationNameToId.erase(it);
                }
                break;

                default:
                {
                    Log::Error(logger, "Dashboard: unexpected SilKitEventType");
                }
                break;
                }
            }

            events.clear();
            ProcessAllAccumulatedBulkUpdates();
        }
    }

    void ProcessEventsWithIndividualRequests() const
    {
        using SilKitEventType = SilKit::Dashboard::SilKitEventType;

        std::unordered_map<std::string, uint64_t> simulationNameToId;

        std::vector<SilKit::Dashboard::SilKitEvent> events;
        while (eventQueue->DequeueAllInto(events))
        {
            for (const auto &event : events)
            {
                if (!abort.valid() || abort.wait_for(std::chrono::seconds{}) != std::future_status::timeout)
                {
                    return;
                }

                // process OnSimulationStart separately, since it does

                if (event.Type() == SilKitEventType::OnSimulationStart)
                {
                    const auto it{simulationNameToId.find(event.GetSimulationName())};
                    if (it != simulationNameToId.end())
                    {
                        Log::Warn(logger, "Dashboard: Simulation {} already has id {}", event.GetSimulationName(),
                                  it->second);
                        continue;
                    }

                    const auto &simulationStart = event.GetSimulationStart();
                    const auto simulationId =
                        eventHandler->OnSimulationStart(simulationStart.connectUri, simulationStart.time);

                    if (simulationId == 0)
                    {
                        Log::Warn(logger, "Dashboard: Simulation {} could not be created", event.GetSimulationName());
                        continue;
                    }

                    simulationNameToId.emplace(event.GetSimulationName(), simulationId);

                    continue;
                }

                // fetch the simulation id for the given name

                const auto it{simulationNameToId.find(event.GetSimulationName())};
                if (it == simulationNameToId.end())
                {
                    Log::Warn(logger, "Dashboard: Simulation {} is unknown", event.GetSimulationName());
                    continue;
                }

                const auto simulationId{it->second};

                // process all event types, except OnSimulationStart

                switch (event.Type())
                {
                case SilKitEventType::OnParticipantConnected:
                {
                    const auto &participantConnectionInformation = event.GetParticipantConnectionInformation();
                    eventHandler->OnParticipantConnected(simulationId, participantConnectionInformation);
                }
                break;

                case SilKitEventType::OnSystemStateChanged:
                {
                    const auto &systemState = event.GetSystemState();
                    eventHandler->OnSystemStateChanged(simulationId, systemState);
                }
                break;

                case SilKitEventType::OnParticipantStatusChanged:
                {
                    const auto &participantStatus = event.GetParticipantStatus();
                    eventHandler->OnParticipantStatusChanged(simulationId, participantStatus);
                }
                break;

                case SilKitEventType::OnServiceDiscoveryEvent:
                {
                    const auto &serviceData = event.GetServiceData();
                    eventHandler->OnServiceDiscoveryEvent(simulationId, serviceData.discoveryType,
                                                          serviceData.serviceDescriptor);
                }
                break;

                case SilKitEventType::OnSimulationEnd:
                {
                    const auto &simulationEnd = event.GetSimulationEnd();
                    eventHandler->OnSimulationEnd(simulationId, simulationEnd.time);

                    simulationNameToId.erase(it);
                }
                break;

                default:
                {
                    Log::Error(logger, "Dashboard: unexpected SilKitEventType");
                }
                break;
                }
            }
            events.clear();
        }
    }

    void operator()() const
    try
    {
        SilKit::Util::SetThreadName("SK-Dash-Cons");

        const bool bulkUpdateAvailable = DetectBulkUpdate();

        if (bulkUpdateAvailable)
        {
            ProcessEventsWithBulkUpdates();
        }
        else
        {
            ProcessEventsWithIndividualRequests();
        }
    }
    catch (const std::exception &exception)
    {
        Log::Error(logger, "Dashboard: event queue worker failed: {}", exception.what());
    }
    catch (...)
    {
        Log::Error(logger, "Dashboard: event queue worker failed with unknown exception");
    }
};

void DashboardInstance::RunEventQueueWorkerThread()
{
    SILKIT_ASSERT(_eventQueueWorkerThread.get_id() == std::thread::id{});

    _eventQueueWorkerThreadAbort = std::promise<void>{};

    EventQueueWorkerThread workerThread;
    workerThread.logger = _logger;
    workerThread.apiClient = _apiClient;
    workerThread.eventHandler = _silKitEventHandler;
    workerThread.eventQueue = _silKitEventQueue;
    workerThread.abort = _eventQueueWorkerThreadAbort.get_future();

    _eventQueueWorkerThread = std::thread{std::move(workerThread)};
}

auto DashboardInstance::GetOrCreateSimulationData(const std::string &simulationName) -> SimulationData &
{
    auto &simulationDataRef{_simulationEventHandlers[simulationName]};
    simulationDataRef.systemStateTracker.SetLogger(_logger);

    return simulationDataRef;
}

void DashboardInstance::RemoveSimulationData(const std::string &simulationName)
{
    _simulationEventHandlers.erase(simulationName);
}


// SilKit::Core::IRegistryEventListener


void DashboardInstance::OnLoggerCreated(SilKit::Services::Logging::ILogger *logger)
{
    SILKIT_ASSERT(_logger == nullptr);
    _logger = logger;
}

void DashboardInstance::OnRegistryUri(const std::string &registryUri)
{
    Log::Debug(_logger, "DashboardInstance::OnRegistryUri: registryUri={}", registryUri);
    SILKIT_ASSERT(_registryUri == nullptr);
    _registryUri = std::make_unique<SilKit::Core::Uri>(registryUri);
}

void DashboardInstance::OnParticipantConnected(const std::string &simulationName, const std::string &participantName)
{
    Log::Trace(_logger, "DashboardInstance::OnParticipantConnected: simulationName={} participantName={}",
               simulationName, participantName);

    auto &simulationData{GetOrCreateSimulationData(simulationName)};

    if (simulationData.systemStateTracker.IsEmpty())
    {
        const auto connectUri{
            SilKit::Core::Uri::MakeSilKit(_registryUri->Host(), _registryUri->Port(), simulationName)};
        _silKitEventQueue->Enqueue(
            SilKitEvent{simulationName, SimulationStart{connectUri.EncodedString(), GetCurrentTime()}});
    }

    _silKitEventQueue->Enqueue(SilKitEvent{
        simulationName, SilKit::Services::Orchestration::ParticipantConnectionInformation{participantName}});
}

void DashboardInstance::OnParticipantDisconnected(const std::string &simulationName, const std::string &participantName)
{
    Log::Debug(_logger, "DashboardInstance::OnParticipantDisconnected: simulationName={} participantName={}",
               simulationName, participantName);

    bool isEmpty{false};

    {
        auto &simulationData{GetOrCreateSimulationData(simulationName)};

        const auto result{simulationData.systemStateTracker.RemoveParticipant(participantName)};
        isEmpty = simulationData.systemStateTracker.IsEmpty();

        if (result.systemStateChanged)
        {
            _silKitEventQueue->Enqueue(SilKitEvent{simulationName, simulationData.systemStateTracker.GetSystemState()});
        }
    }

    if (isEmpty)
    {
        _silKitEventQueue->Enqueue(SilKitEvent{simulationName, SimulationEnd{GetCurrentTime()}});
        RemoveSimulationData(simulationName);
    }
}

void DashboardInstance::OnRequiredParticipantsUpdate(const std::string &simulationName,
                                                     const std::string &participantName,
                                                     SilKit::Util::Span<const std::string> requiredParticipantNames)
{
    Log::Trace(_logger,
               "DashboardInstance::OnRequiredParticipantsUpdate: simulationName={} participantName={} "
               "requiredParticipantNames={}",
               simulationName, participantName, requiredParticipantNames.size());

    auto &simulationData{GetOrCreateSimulationData(simulationName)};
    const auto result{simulationData.systemStateTracker.UpdateRequiredParticipants(requiredParticipantNames)};

    if (result.systemStateChanged)
    {
        _silKitEventQueue->Enqueue(SilKitEvent{simulationName, simulationData.systemStateTracker.GetSystemState()});
    }
}

void DashboardInstance::OnParticipantStatusUpdate(
    const std::string &simulationName, const std::string &participantName,
    const SilKit::Services::Orchestration::ParticipantStatus &participantStatus)
{
    Log::Trace(_logger,
               "DashboardInstance::OnParticipantStatusUpdate: simulationName={} participantName={} "
               "participantState={}",
               simulationName, participantName, participantStatus.state);

    auto &simulationData{GetOrCreateSimulationData(simulationName)};
    const auto result{simulationData.systemStateTracker.UpdateParticipantStatus(participantStatus)};

    if (result.participantStateChanged)
    {
        _silKitEventQueue->Enqueue(SilKitEvent{simulationName, participantStatus});
    }

    if (result.systemStateChanged)
    {
        _silKitEventQueue->Enqueue(SilKitEvent{simulationName, simulationData.systemStateTracker.GetSystemState()});
    }
}

void DashboardInstance::OnServiceDiscoveryEvent(
    const std::string &simulationName, const std::string &participantName,
    const SilKit::Core::Discovery::ServiceDiscoveryEvent &serviceDiscoveryEvent)
{
    Log::Trace(_logger,
               "DashboardInstance::OnServiceDiscoveryEvent: simulationName={} participantName={} serviceName={}",
               simulationName, participantName, serviceDiscoveryEvent.serviceDescriptor.GetServiceName());

    if (ShouldSkipServiceDiscoveryEvent(serviceDiscoveryEvent))
    {
        return;
    }

    _silKitEventQueue->Enqueue(
        SilKitEvent{simulationName, ServiceData{serviceDiscoveryEvent.type, serviceDiscoveryEvent.serviceDescriptor}});
}


} // namespace VSilKit