/* Copyright (c) 2022 Vector Informatik GmbH
 
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "DashboardParticipant.hpp"

#include "silkit/SilKit.hpp"
#include "silkit/config/IParticipantConfiguration.hpp"
#include "LoggerMessage.hpp"
#include "IParticipantInternal.hpp"
#include "IServiceDiscovery.hpp"

#include "CreateParticipantImpl.hpp"

#include "CachingSilKitEventHandler.hpp"
#include "SilKitEventQueue.hpp"
#include "DashboardSystemServiceClient.hpp"
#include "SilKitEventHandler.hpp"
#include "SilKitToOatppMapper.hpp"

using namespace std::chrono_literals;

namespace SilKit {
namespace Dashboard {

DashboardParticipant::DashboardParticipant(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                                           const std::string& registryUri,
                                           std::shared_ptr<oatpp::data::mapping::ObjectMapper> objectMapper,
                                           std::shared_ptr<DashboardSystemApiClient> apiClient,
                                           std::shared_ptr<ISilKitToOatppMapper> silKitToOatppMapper)
{
    _participant = SilKit::CreateParticipantImpl(participantConfig, _participantName, registryUri);
    auto participantInternal = dynamic_cast<Core::IParticipantInternal*>(_participant.get());
    _lifecycleService = _participant->CreateLifecycleService({Services::Orchestration::OperationMode::Autonomous});
    _systemMonitor = _participant->CreateSystemMonitor();
    _serviceDiscovery = participantInternal->GetServiceDiscovery();
    _logger = participantInternal->GetLogger();
    auto serviceClient = std::make_shared<DashboardSystemServiceClient>(_logger, apiClient, objectMapper);
    auto eventHandler = std::make_shared<SilKitEventHandler>(_logger, serviceClient, silKitToOatppMapper);
    auto eventQueue = std::make_shared<SilKitEventQueue>();
    _cachingEventHandler = std::make_unique<CachingSilKitEventHandler>(registryUri, _logger, eventHandler, eventQueue);

    _systemMonitor->SetParticipantConnectedHandler(
        [this](auto&& participantInformation) { OnParticipantConnected(participantInformation); });
    _systemMonitor->SetParticipantDisconnectedHandler(
        [this](auto&& participantInformation) { OnParticipantDisconnected(participantInformation); });
    _participantStatusHandlerId = _systemMonitor->AddParticipantStatusHandler(
        [this](auto&& participantStatus) { OnParticipantStatusChanged(participantStatus); });
    _systemStateHandlerId =
        _systemMonitor->AddSystemStateHandler([this](auto&& systemState) { OnSystemStateChanged(systemState); });
    _serviceDiscovery->RegisterServiceDiscoveryHandler([this](auto&& discoveryType, auto&& serviceDescriptor) {
        OnServiceDiscoveryEvent(discoveryType, serviceDescriptor);
    });
}

DashboardParticipant::~DashboardParticipant()
{
    _systemMonitor->RemoveParticipantStatusHandler(_participantStatusHandlerId);
    _systemMonitor->RemoveSystemStateHandler(_systemStateHandlerId);
}

void DashboardParticipant::Run()
{
    auto lifecycleDone = _lifecycleService->StartLifecycle();
    lifecycleDone.get();
    if (!_shutdown && !NoParticipantConnected())
    {
        // abort
        _cachingEventHandler->OnLastParticipantDisconnected();
    }
}

void DashboardParticipant::Shutdown()
{
    _shutdown = true;
    auto timeout = 10s;
    if (std::future_status::ready == _runningReachedPromise.get_future().wait_for(timeout))
    {
        _lifecycleService->Stop("Stop");
    }
}

void DashboardParticipant::OnParticipantConnected(
    const Services::Orchestration::ParticipantConnectionInformation& participantInformation)
{
    if (participantInformation.participantName == _participantName)
    {
        return;
    }
    {
        std::lock_guard<decltype(_connectedParticipantsMx)> lock(_connectedParticipantsMx);
        _connectedParticipants.insert(participantInformation.participantName);
    }
    _cachingEventHandler->OnParticipantConnected(participantInformation);
}

void DashboardParticipant::OnParticipantDisconnected(
    const Services::Orchestration::ParticipantConnectionInformation& participantInformation)
{
    if (participantInformation.participantName == _participantName)
    {
        return;
    }
    if (LastParticipantDisconnected(participantInformation))
    {
        _cachingEventHandler->OnLastParticipantDisconnected();
    }
}

void DashboardParticipant::OnParticipantStatusChanged(
    const Services::Orchestration::ParticipantStatus& participantStatus)
{
    if (participantStatus.participantName == _participantName)
    {
        if (participantStatus.state == Services::Orchestration::ParticipantState::Running)
        {
            _runningReachedPromise.set_value();
        }
        return;
    }
    _cachingEventHandler->OnParticipantStatusChanged(participantStatus);
}

void DashboardParticipant::OnSystemStateChanged(Services::Orchestration::SystemState systemState)
{
    _cachingEventHandler->OnSystemStateChanged(systemState);
}

void DashboardParticipant::OnServiceDiscoveryEvent(Core::Discovery::ServiceDiscoveryEvent::Type discoveryType,
                                                   const Core::ServiceDescriptor& serviceDescriptor)
{
    if (serviceDescriptor.GetParticipantName() == _participantName)
    {
        return;
    }
    _cachingEventHandler->OnServiceDiscoveryEvent(discoveryType, serviceDescriptor);
}

bool DashboardParticipant::LastParticipantDisconnected(
    const Services::Orchestration::ParticipantConnectionInformation& participantInformation)
{
    std::lock_guard<decltype(_connectedParticipantsMx)> lock(_connectedParticipantsMx);
    _connectedParticipants.erase(participantInformation.participantName);
    Services::Logging::Debug(_logger, "Dashboard: {} connected participant(s)", _connectedParticipants.size());
    return _connectedParticipants.empty();
}

bool DashboardParticipant::NoParticipantConnected()
{
    std::lock_guard<decltype(_connectedParticipantsMx)> lock(_connectedParticipantsMx);
    Services::Logging::Debug(_logger, "Dashboard: {} connected participant(s)", _connectedParticipants.size());
    return _connectedParticipants.empty();
}

} // namespace Dashboard
} // namespace SilKit
