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

#pragma once

#include <future>

#include "silkit/participant/IParticipant.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/logging/ILogger.hpp"
#include "ICachingSilKitEventHandler.hpp"

//forwards
namespace SilKit {
namespace Core {
class IParticipantInternal;
namespace Discovery {
class IServiceDiscovery;
} // Discovery
} // namespace Core
namespace Config {
class IParticipantConfiguration;
} // namespace Config
} // namespace SilKit

namespace SilKit {
namespace Dashboard {

class DashboardRetryPolicy;

class Dashboard 
{
public:
    Dashboard(std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfig,
              const std::string& registryUri);
    ~Dashboard();

private:
    void OnParticipantConnected(
        const Services::Orchestration::ParticipantConnectionInformation& participantInformation);
    void OnParticipantDisconnected(
        const Services::Orchestration::ParticipantConnectionInformation& participantInformation);
    void OnParticipantStatusChanged(const Services::Orchestration::ParticipantStatus& participantStatus);
    void OnSystemStateChanged(Services::Orchestration::SystemState systemState);
    void OnServiceDiscoveryEvent(Core::Discovery::ServiceDiscoveryEvent::Type discoveryType,
                                            const Core::ServiceDescriptor& serviceDescriptor);
    bool LastParticipantDisconnected(
        const Services::Orchestration::ParticipantConnectionInformation& participantInformation);

private:
    std::unique_ptr<SilKit::IParticipant> _dashboardParticipant;
    SilKit::Core::IParticipantInternal* _participantInternal{nullptr};

    SilKit::Services::Orchestration::ILifecycleService* _lifecycleService{nullptr};
    SilKit::Services::Orchestration::ISystemMonitor* _systemMonitor{ nullptr };
    SilKit::Core::Discovery::IServiceDiscovery* _serviceDiscovery{ nullptr };
    SilKit::Services::Logging::ILogger* _logger{ nullptr };
    std::shared_ptr<DashboardRetryPolicy> _retryPolicy;
    std::unique_ptr<ICachingSilKitEventHandler> _cachingEventHandler;
    std::mutex _connectedParticipantsMx;
    std::vector<std::string> _connectedParticipants;

    SilKit::Util::HandlerId _participantStatusHandlerId{};
    SilKit::Util::HandlerId _systemStateHandlerId{};

    std::future<SilKit::Services::Orchestration::ParticipantState> _lifecycleDone;
    std::promise<void> _runningReachedPromise;
};

} // namespace Dashboard
} // namespace SilKit
