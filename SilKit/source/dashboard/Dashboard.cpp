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

#include "Dashboard.hpp"

#include <chrono>

#include "oatpp/core/macro/component.hpp"
#include "oatpp/web/client/HttpRequestExecutor.hpp"
#include "oatpp/network/ConnectionProvider.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"

#include "silkit/SilKit.hpp"
#include "silkit/config/IParticipantConfiguration.hpp"
#include "ILogger.hpp"
#include "IParticipantInternal.hpp"
#include "IServiceDiscovery.hpp"
#include "SetThreadName.hpp"

#include "ParticipantConfiguration.hpp"
#include "CreateParticipantImpl.hpp"

#include "CachingSilKitEventHandler.hpp"
#include "DashboardRetryPolicy.hpp"
#include "DashboardSystemServiceClient.hpp"
#include "SilKitEventHandler.hpp"
#include "SilKitToOatppMapper.hpp"

using namespace std::chrono_literals;

namespace SilKit {
namespace Dashboard {

uint64_t GetCurrentTime()
{
    auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}

Dashboard::Dashboard(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
                     const std::string& registryUri)
{
    _dashboardParticipant = SilKit::CreateParticipantImpl(participantConfig, "__SilKitDashboard", registryUri);
    _participantInternal = dynamic_cast<Core::IParticipantInternal*>(_dashboardParticipant.get());
    _retryPolicy = std::make_shared<DashboardRetryPolicy>(3);
    OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, objectMapper);
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ClientConnectionProvider>, connectionProvider);
    auto requestExecutor = oatpp::web::client::HttpRequestExecutor::createShared(connectionProvider, _retryPolicy);
    auto apiClient = DashboardSystemApiClient::createShared(requestExecutor, objectMapper);
    auto silKitToOatppMapper = std::make_shared<SilKitToOatppMapper>();
    _asyncExecutor = std::make_shared<oatpp::async::Executor>(1, 1, 1);
    auto serviceClient = std::make_shared<DashboardSystemServiceClient>(_participantInternal->GetLogger(), apiClient,
                                                                        objectMapper, _asyncExecutor);
    auto eventHandler =
        std::make_shared<SilKitEventHandler>(_participantInternal->GetLogger(), serviceClient, silKitToOatppMapper);
    _cachingEventHandler = std::make_shared<CachingSilKitEventHandler>(
        _participantInternal->GetLogger(), _participantInternal->GetParticipantName(), eventHandler);

    _systemMonitor = _participantInternal->CreateSystemMonitor();
    _systemMonitor->SetParticipantConnectedHandler([this](auto&& participantInformation) {
        _cachingEventHandler->OnParticipantConnected(participantInformation);
    });
    _participantStatusHandlerId = _systemMonitor->AddParticipantStatusHandler([this](auto&& participantStatus) {
        _cachingEventHandler->OnParticipantStatusChanged(participantStatus);
    });
    _systemStateHandlerId = _systemMonitor->AddSystemStateHandler([this](auto&& systemState) {
        _cachingEventHandler->OnSystemStateChanged(systemState);
    });
    _serviceDiscovery = _participantInternal->GetServiceDiscovery();
    _serviceDiscovery->RegisterServiceDiscoveryHandler([this](auto&& discoveryType, auto&& serviceDescriptor) {
        _cachingEventHandler->OnServiceDiscoveryEvent(discoveryType, serviceDescriptor);
    });

    _cachingEventHandler->OnStart(_participantInternal->GetRegistryUri(), GetCurrentTime());
    _dashboardThread = std::thread{[this]() {
        SilKit::Util::SetThreadName("SK-Dashboard");
        auto dashboardStop = _dashboardStopPromise.get_future();
        Run(std::move(dashboardStop));
        _cachingEventHandler->OnShutdown(GetCurrentTime());
    }};
}

Dashboard::~Dashboard()
{
    _dashboardStopPromise.set_value();
    if (_dashboardThread.joinable())
    {
        _dashboardThread.join();
    }
    _systemMonitor->RemoveParticipantStatusHandler(_participantStatusHandlerId);
    _systemMonitor->RemoveSystemStateHandler(_systemStateHandlerId);
    _retryPolicy->AbortAllRetries();
    Services::Logging::Info(_participantInternal->GetLogger(), "Dashboard: {} executor task(s) still running",
                            _asyncExecutor->getTasksCount());
    _asyncExecutor->waitTasksFinished(10s);
    Services::Logging::Info(_participantInternal->GetLogger(), "Dashboard: executor tasks finished");
    _asyncExecutor->stop();
    _asyncExecutor->join();
}

void Dashboard::Run(std::future<void> stopSignal)
{
    auto lifecycleService =
        _participantInternal->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Autonomous});
    auto finalStatePromise = lifecycleService->StartLifecycle();
    stopSignal.wait();
    finalStatePromise.wait_for(2s);
}

} // namespace Dashboard
} // namespace SilKit