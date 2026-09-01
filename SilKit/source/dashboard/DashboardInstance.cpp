// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "dashboard/DashboardInstance.hpp"

#include <chrono>
#include <future>
#include <utility>

#include "dashboard/EventQueueWorkerThread.hpp"
#include "dashboard/service/DashboardRestClient.hpp"
#include "services/logging/LoggerMessage.hpp"
#include "util/Assert.hpp"
#include "util/SetThreadName.hpp"
#include "util/Uri.hpp"


namespace {


/// How long the destructor lets an in-flight dashboard request finish before aborting it.
constexpr auto kShutdownGracePeriod = std::chrono::seconds{5};


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

using namespace SilKit::Services;
using namespace SilKit::Services::Logging;
using SilKit::Dashboard::DashboardBulkUpdate;

DashboardInstance::DashboardInstance(const std::string& dashboardUri)
    : _dashboardUri{dashboardUri}
{
    // Parse eagerly so that a malformed --dashboard-uri is reported when the instance is created,
    // rather than later on the registry's thread. The result is discarded; DashboardRestClient
    // parses it again once it is built.
    (void)SilKit::Core::Uri::Parse(dashboardUri);
}

DashboardInstance::~DashboardInstance()
{
    _abortWorker.store(true, std::memory_order_release);
    _silKitEventQueue.Stop();

    if (!_eventQueueWorkerThread.joinable())
    {
        return;
    }

    /* The worker may be blocked in an HTTP request. Give it a grace period to finish flushing, then
     * abort the transport so shutdown stays bounded even when the dashboard server accepts
     * connections but never answers. std::thread has no timed join, hence the watchdog. */
    std::promise<void> workerFinished;
    auto workerFinishedFuture = workerFinished.get_future();
    auto watchdog = std::async(std::launch::async, [this, &workerFinishedFuture] {
        if (workerFinishedFuture.wait_for(kShutdownGracePeriod) == std::future_status::timeout
            && _dashboardRestClient != nullptr)
        {
            _dashboardRestClient->Abort();
        }
    });

    _eventQueueWorkerThread.join();

    workerFinished.set_value();
    watchdog.wait();
}

auto DashboardInstance::GetRegistryEventListener() -> SilKit::Core::IRegistryEventListener*
{
    return this;
}

void DashboardInstance::StartWorker()
{
    SILKIT_ASSERT(_eventQueueWorkerThread.get_id() == std::thread::id{});

    _dashboardRestClient = std::make_unique<SilKit::Dashboard::DashboardRestClient>(_logger, _dashboardUri);

    EventQueueWorkerThread worker{_logger, _dashboardRestClient.get(), &_silKitEventQueue, &_abortWorker};
    _eventQueueWorkerThread = std::thread{[worker] {
        SilKit::Util::SetThreadName("SK-Dash-Cons");
        worker();
    }};
}

void DashboardInstance::OnLoggerCreated(SilKit::Services::Logging::ILoggerInternal* logger)
{
    SILKIT_ASSERT(_logger == nullptr);
    _logger = logger;
}

void DashboardInstance::OnRegistryUri(const std::string& registryUri)
{
    _logger->MakeMessage(Level::Debug, TopicOf(*this))
        .SetMessage("DashboardInstance::OnRegistryUri: registryUri={}", registryUri)
        .Dispatch();
    SILKIT_ASSERT(!_registryUri.has_value());
    _registryUri = SilKit::Core::Uri{registryUri};

    // Both prerequisites are now in place: the logger and the registry URI.
    StartWorker();
}

void DashboardInstance::OnParticipantConnected(const std::string& simulationName, const std::string& participantName)
{
    _logger->MakeMessage(Level::Trace, TopicOf(*this))
        .SetMessage("DashboardInstance::OnParticipantConnected: simulationName={} participantName={}",
                    simulationName, participantName)
        .Dispatch();

    auto& systemStateTracker{_systemStateTrackers[simulationName]};

    if (systemStateTracker.IsEmpty())
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
    _logger->MakeMessage(Level::Debug, TopicOf(*this))
        .SetMessage("DashboardInstance::OnParticipantDisconnected: simulationName={} participantName={}",
                    simulationName, participantName)
        .Dispatch();

    bool isEmpty{false};

    {
        auto& systemStateTracker{_systemStateTrackers[simulationName]};

        const auto result{systemStateTracker.RemoveParticipant(participantName)};
        isEmpty = systemStateTracker.IsEmpty();

        if (result.systemStateChanged)
        {
            _silKitEventQueue.Enqueue(SilKitEvent{simulationName, systemStateTracker.GetSystemState()});
        }
    }

    if (isEmpty)
    {
        _silKitEventQueue.Enqueue(SilKitEvent{simulationName, SimulationEnd{GetCurrentSystemTime()}});
        _systemStateTrackers.erase(simulationName);
    }
}

void DashboardInstance::OnRequiredParticipantsUpdate(const std::string& simulationName,
                                                     const std::string& participantName,
                                                     SilKit::Util::Span<const std::string> requiredParticipantNames)
{
    _logger->MakeMessage(Level::Trace, TopicOf(*this))
        .SetMessage("DashboardInstance::OnRequiredParticipantsUpdate: simulationName={} participantName={} "
                    "requiredParticipantNames={}",
                    simulationName, participantName, requiredParticipantNames.size())
        .Dispatch();

    auto& systemStateTracker{_systemStateTrackers[simulationName]};
    const auto result{systemStateTracker.UpdateRequiredParticipants(requiredParticipantNames)};

    if (result.systemStateChanged)
    {
        _silKitEventQueue.Enqueue(SilKitEvent{simulationName, systemStateTracker.GetSystemState()});
    }
}

void DashboardInstance::OnParticipantStatusUpdate(
    const std::string& simulationName, const std::string& participantName,
    const SilKit::Services::Orchestration::ParticipantStatus& participantStatus)
{
    _logger->MakeMessage(Level::Trace, TopicOf(*this))
        .SetMessage("DashboardInstance::OnParticipantStatusUpdate: simulationName={} participantName={} "
                    "participantState={}",
                    simulationName, participantName, participantStatus.state)
        .Dispatch();

    auto& systemStateTracker{_systemStateTrackers[simulationName]};
    const auto result{systemStateTracker.UpdateParticipantStatus(participantStatus)};

    if (result.participantStateChanged)
    {
        _silKitEventQueue.Enqueue(SilKitEvent{simulationName, participantStatus});
    }

    if (result.systemStateChanged)
    {
        _silKitEventQueue.Enqueue(SilKitEvent{simulationName, systemStateTracker.GetSystemState()});
    }
}

void DashboardInstance::OnServiceDiscoveryEvent(
    const std::string& simulationName, const std::string& participantName,
    const SilKit::Core::Discovery::ServiceDiscoveryEvent& serviceDiscoveryEvent)
{
    if (ShouldSkipServiceDiscoveryEvent(serviceDiscoveryEvent))
    {
        return;
    }

    _logger->MakeMessage(Level::Trace, TopicOf(*this))
        .SetMessage("DashboardInstance::OnServiceDiscoveryEvent: simulationName={} participantName={} serviceName={}",
                    simulationName, participantName, serviceDiscoveryEvent.serviceDescriptor.GetServiceName())
        .Dispatch();

    _silKitEventQueue.Enqueue(
        SilKitEvent{simulationName, ServiceData{serviceDiscoveryEvent.type, serviceDiscoveryEvent.serviceDescriptor}});
}

void DashboardInstance::OnMetricsUpdate(const std::string& simulationName, const std::string& origin,
                                        const VSilKit::MetricsUpdate& metricsUpdate)
{
    _logger->MakeMessage(Level::Trace, TopicOf(*this))
        .SetMessage("DashboardInstance::OnMetricsUpdate: simulationName={} origin={} metricsUpdate={}",
                    simulationName, origin, metricsUpdate)
        .Dispatch();

    _silKitEventQueue.Enqueue(SilKitEvent{simulationName, MetricsUpdatePair{origin, metricsUpdate}});
}


} // namespace VSilKit
