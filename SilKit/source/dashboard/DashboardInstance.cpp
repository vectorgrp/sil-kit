// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "DashboardInstance.hpp"
#include "SilKitEvent.hpp"
#include "LockedQueue.hpp"
#include "SilKitToOatppMapper.hpp"
#include "DashboardRestClient.hpp"


namespace Log = SilKit::Services::Logging;


namespace {


uint64_t GetCurrentSystemTime()
{
    auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}

bool ShouldSkipServiceDiscoveryEvent(const SilKit::Core::Discovery::ServiceDiscoveryEvent& serviceDiscoveryEvent)
{
    return serviceDiscoveryEvent.type != SilKit::Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated
           || (serviceDiscoveryEvent.serviceDescriptor.GetServiceType() != SilKit::Core::ServiceType::Controller
               && serviceDiscoveryEvent.serviceDescriptor.GetServiceType() != SilKit::Core::ServiceType::Link);
}


using VSilKit::SilKitEvent;
using VSilKit::SimulationStart;
using VSilKit::SimulationEnd;
using VSilKit::ServiceData;


} // namespace


namespace VSilKit {


DashboardInstance::DashboardInstance() {}

DashboardInstance::~DashboardInstance()
{
    try
    {
        _eventQueueWorkerThreadAbort.set_value();
    }
    catch (...)
    {
        // ignored
    }

    _silKitEventQueue.Stop();

    if (_eventQueueWorkerThread.joinable())
    {
        _eventQueueWorkerThread.join();
    }
}

auto DashboardInstance::GetRegistryEventListener() -> SilKit::Core::IRegistryEventListener*
{
    return this;
}

void DashboardInstance::SetupDashboardConnection(const std::string& dashboardUri)
{
    _dashboardRestClient = std::make_shared<SilKit::Dashboard::DashboardRestClient>(_logger, dashboardUri);
    RunEventQueueWorkerThread();
}

using namespace SilKit::Services;
using namespace SilKit::Services::Logging;
using namespace SilKit::Dashboard;
class EventQueueWorkerThread
{
    ILogger* _logger{nullptr};
    IRestClient* _dashboardRestClient{nullptr};
    LockedQueue<SilKitEvent>* _eventQueue{nullptr};
    std::future<void> _abort;

public: //CTor
    EventQueueWorkerThread(ILogger* logger, IRestClient* dashboardRestClient, LockedQueue<SilKitEvent>* eventQueue,
                           std::future<void> abort)
        : _logger{logger}
        , _dashboardRestClient{dashboardRestClient}
        , _eventQueue{eventQueue}
        , _abort{std::move(abort)}
    {
    }

    auto DetectBulkUpdate() const -> bool
    {
        auto bulkUpdateAvailable = _dashboardRestClient->IsBulkUpdateSupported();
        if (bulkUpdateAvailable)
        {
            _logger->Debug("Dashboard bulk-updates are available");
        }
        else
        {
            _logger->Debug("Dashboard bulk-updates are not available, falling back to individual requests");
        }

        return bulkUpdateAvailable;
    }

    void ProcessEventsWithBulkUpdates() const
    {
        std::unordered_map<std::string, uint64_t> simulationNameToId;
        std::unordered_map<uint64_t, SilKit::Dashboard::DashboardBulkUpdate> simulationBulkUpdates;

        std::vector<SilKitEvent> events;
        while (_eventQueue->DequeueAllInto(events))
        {
            const auto ProcessAllAccumulatedBulkUpdates = [this, &simulationBulkUpdates] {
                for (auto& pair : simulationBulkUpdates)
                {
                    const auto simulationId = pair.first;
                    auto& bulkUpdate = pair.second;

                    if (bulkUpdate.Empty())
                    {
                        continue;
                    }

                    _dashboardRestClient->OnBulkUpdate(simulationId, bulkUpdate);
                    bulkUpdate.Clear();
                }
            };

            for (const auto& event : events)
            {
                if (!_abort.valid() || _abort.wait_for(std::chrono::seconds{}) != std::future_status::timeout)
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
                        Log::Debug(_logger, "Dashboard: Simulation {} already has id {}", event.GetSimulationName(),
                                   it->second);
                        continue;
                    }

                    const auto& simulationStart = event.GetSimulationStart();
                    const auto simulationId =
                        _dashboardRestClient->OnSimulationStart(simulationStart.connectUri, simulationStart.time);

                    if (simulationId == 0)
                    {
                        Log::Warn(_logger, "Dashboard: Simulation {} could not be created", event.GetSimulationName());
                        continue;
                    }

                    simulationNameToId.emplace(event.GetSimulationName(), simulationId);

                    continue;
                }

                // fetch the simulation id for the given name

                const auto it{simulationNameToId.find(event.GetSimulationName())};
                if (it == simulationNameToId.end())
                {
                    Log::Warn(_logger, "Dashboard: Simulation {} is unknown", event.GetSimulationName());
                    continue;
                }

                const auto simulationId{it->second};
                auto& bulkUpdate{simulationBulkUpdates[simulationId]};

                // process all event types, except OnSimulationStart

                switch (event.Type())
                {
                case SilKitEventType::OnParticipantConnected:
                {
                    const auto& participantConnectionInformation = event.GetParticipantConnectionInformation();
                    bulkUpdate.participantConnectionInformations.emplace_back(participantConnectionInformation);
                }
                break;

                case SilKitEventType::OnSystemStateChanged:
                {
                    const auto& systemState = event.GetSystemState();
                    bulkUpdate.systemStates.emplace_back(systemState);
                }
                break;

                case SilKitEventType::OnParticipantStatusChanged:
                {
                    const auto& participantStatus = event.GetParticipantStatus();
                    bulkUpdate.participantStatuses.emplace_back(participantStatus);
                }
                break;

                case SilKitEventType::OnServiceDiscoveryEvent:
                {
                    const auto& serviceData = event.GetServiceData();
                    bulkUpdate.serviceDatas.emplace_back(serviceData);
                }
                break;

                case SilKitEventType::OnSimulationEnd:
                {
                    const auto& simulationEnd = event.GetSimulationEnd();
                    bulkUpdate.stopped = std::make_unique<uint64_t>(simulationEnd.time);

                    simulationNameToId.erase(it);
                }
                break;

                case SilKitEventType::OnMetricUpdate:
                {
                    const auto& data = event.GetMetricsUpdate();
                    _dashboardRestClient->OnMetricsUpdate(simulationId, data.first, data.second);
                }
                break;

                default:
                {
                    Log::Error(_logger, "Dashboard: unexpected SilKitEventType");
                }
                break;
                }
            }

            events.clear();
            ProcessAllAccumulatedBulkUpdates();
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
            throw SilKit::SilKitError{"Bulk update for REST API is required!"};
        }
    }
    catch (const std::exception& exception)
    {
        Log::Error(_logger, "Dashboard: event queue worker failed: {}", exception.what());
    }
    catch (...)
    {
        Log::Error(_logger, "Dashboard: event queue worker failed with unknown exception");
    }
};

void DashboardInstance::RunEventQueueWorkerThread()
{
    SILKIT_ASSERT(_eventQueueWorkerThread.get_id() == std::thread::id{});

    _eventQueueWorkerThreadAbort = std::promise<void>{};

    EventQueueWorkerThread workerThread{_logger, _dashboardRestClient.get(), &_silKitEventQueue,
                                        _eventQueueWorkerThreadAbort.get_future()};

    _eventQueueWorkerThread = std::thread{std::move(workerThread)};
}

auto DashboardInstance::GetOrCreateSimulationData(const std::string& simulationName) -> SimulationData&
{
    auto& simulationDataRef{_simulationEventHandlers[simulationName]};
    //vikabgm: only used for debugging? simulationDataRef.systemStateTracker.SetLogger(_logger);

    return simulationDataRef;
}

void DashboardInstance::RemoveSimulationData(const std::string& simulationName)
{
    _simulationEventHandlers.erase(simulationName);
}

void DashboardInstance::OnLoggerCreated(SilKit::Services::Logging::ILogger* logger)
{
    SILKIT_ASSERT(_logger == nullptr);
    _logger = logger;
}

void DashboardInstance::OnRegistryUri(const std::string& registryUri)
{
    Log::Debug(_logger, "DashboardInstance::OnRegistryUri: registryUri={}", registryUri);
    SILKIT_ASSERT(_registryUri == nullptr);
    _registryUri = std::make_unique<SilKit::Core::Uri>(registryUri);
}

void DashboardInstance::OnParticipantConnected(const std::string& simulationName, const std::string& participantName)
{
    Log::Trace(_logger, "DashboardInstance::OnParticipantConnected: simulationName={} participantName={}",
               simulationName, participantName);

    auto& simulationData{GetOrCreateSimulationData(simulationName)};

    if (simulationData.systemStateTracker.IsEmpty())
    {
        const auto connectUri{
            SilKit::Core::Uri::MakeSilKit(_registryUri->Host(), _registryUri->Port(), simulationName)};
        _silKitEventQueue.Enqueue(
            SilKitEvent{simulationName, SimulationStart{connectUri.EncodedString(), GetCurrentSystemTime()}});
    }

    _silKitEventQueue.Enqueue(SilKitEvent{
        simulationName, SilKit::Services::Orchestration::ParticipantConnectionInformation{participantName}});
}

void DashboardInstance::OnParticipantDisconnected(const std::string& simulationName, const std::string& participantName)
{
    Log::Debug(_logger, "DashboardInstance::OnParticipantDisconnected: simulationName={} participantName={}",
               simulationName, participantName);

    bool isEmpty{false};

    {
        auto& simulationData{GetOrCreateSimulationData(simulationName)};

        const auto result{simulationData.systemStateTracker.RemoveParticipant(participantName)};
        isEmpty = simulationData.systemStateTracker.IsEmpty();

        if (result.systemStateChanged)
        {
            _silKitEventQueue.Enqueue(SilKitEvent{simulationName, simulationData.systemStateTracker.GetSystemState()});
        }
    }

    if (isEmpty)
    {
        _silKitEventQueue.Enqueue(SilKitEvent{simulationName, SimulationEnd{GetCurrentSystemTime()}});
        RemoveSimulationData(simulationName);
    }
}

void DashboardInstance::OnRequiredParticipantsUpdate(const std::string& simulationName,
                                                     const std::string& participantName,
                                                     SilKit::Util::Span<const std::string> requiredParticipantNames)
{
    Log::Trace(_logger,
               "DashboardInstance::OnRequiredParticipantsUpdate: simulationName={} participantName={} "
               "requiredParticipantNames={}",
               simulationName, participantName, requiredParticipantNames.size());

    auto& simulationData{GetOrCreateSimulationData(simulationName)};
    const auto result{simulationData.systemStateTracker.UpdateRequiredParticipants(requiredParticipantNames)};

    if (result.systemStateChanged)
    {
        _silKitEventQueue.Enqueue(SilKitEvent{simulationName, simulationData.systemStateTracker.GetSystemState()});
    }
}

void DashboardInstance::OnParticipantStatusUpdate(
    const std::string& simulationName, const std::string& participantName,
    const SilKit::Services::Orchestration::ParticipantStatus& participantStatus)
{
    Log::Trace(_logger,
               "DashboardInstance::OnParticipantStatusUpdate: simulationName={} participantName={} "
               "participantState={}",
               simulationName, participantName, participantStatus.state);

    auto& simulationData{GetOrCreateSimulationData(simulationName)};
    const auto result{simulationData.systemStateTracker.UpdateParticipantStatus(participantStatus)};

    if (result.participantStateChanged)
    {
        _silKitEventQueue.Enqueue(SilKitEvent{simulationName, participantStatus});
    }

    if (result.systemStateChanged)
    {
        _silKitEventQueue.Enqueue(SilKitEvent{simulationName, simulationData.systemStateTracker.GetSystemState()});
    }
}

void DashboardInstance::OnServiceDiscoveryEvent(
    const std::string& simulationName, const std::string& participantName,
    const SilKit::Core::Discovery::ServiceDiscoveryEvent& serviceDiscoveryEvent)
{
    Log::Trace(_logger,
               "DashboardInstance::OnServiceDiscoveryEvent: simulationName={} participantName={} serviceName={}",
               simulationName, participantName, serviceDiscoveryEvent.serviceDescriptor.GetServiceName());

    if (ShouldSkipServiceDiscoveryEvent(serviceDiscoveryEvent))
    {
        return;
    }

    _silKitEventQueue.Enqueue(
        SilKitEvent{simulationName, ServiceData{serviceDiscoveryEvent.type, serviceDiscoveryEvent.serviceDescriptor}});
}

void DashboardInstance::OnMetricsUpdate(const std::string& simulationName, const std::string& origin,
                                        const VSilKit::MetricsUpdate& metricsUpdate)
{
    Log::Trace(_logger, "DashboardInstance::OnMetricsUpdate: simulationName={} origin={} metricsUpdate={}",
               simulationName, origin, metricsUpdate);

    std::pair<std::string, VSilKit::MetricsUpdate> data{origin, metricsUpdate};

    _silKitEventQueue.Enqueue(SilKitEvent{simulationName, std::move(data)});
}


} // namespace VSilKit