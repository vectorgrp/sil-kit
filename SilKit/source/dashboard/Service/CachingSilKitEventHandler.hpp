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

#include "ISilKitEventHandler.hpp"

#include <atomic>
#include <memory>
#include <thread>

#include "silkit/services/logging/ILogger.hpp"

#include "CachedData.hpp"

namespace SilKit {
namespace Dashboard {

enum CachingSilKitEventHandlerState
{
    Caching,
    Sending,
    Disabled
};   

// Filters own events and caches SIL Kit Event as long as the dashboard has not allocated a simulation id
class CachingSilKitEventHandler : public ISilKitEventHandler
{
public:
    CachingSilKitEventHandler(Services::Logging::ILogger* logger, const std::string& participantName,
                              std::shared_ptr<ISilKitEventHandler> eventHandler);
    ~CachingSilKitEventHandler();

public: //methods

    std::future<bool> OnStart(const std::string& connectUri, uint64_t time) override;
    void OnShutdown(uint64_t time) override;
    void OnParticipantConnected(
        const Services::Orchestration::ParticipantConnectionInformation& participantInformation) override;
    void OnParticipantStatusChanged(const Services::Orchestration::ParticipantStatus& participantStatus) override;
    void OnSystemStateChanged(Services::Orchestration::SystemState systemState) override;
    void OnServiceDiscoveryEvent(Core::Discovery::ServiceDiscoveryEvent::Type discoveryType,
                                 const Core::ServiceDescriptor& serviceDescriptor) override;

private: //methods
    void Run(const std::string& connectUri, uint64_t time);
    void NotifyCachedEvents();

private: //member
    std::thread _simulationCreationThread;
    std::promise<bool> _simulationCreatedPromise;
    std::atomic<CachingSilKitEventHandlerState> _state{Caching};

    Services::Logging::ILogger* _logger;
    std::string _participantName;
    std::shared_ptr<ISilKitEventHandler> _eventHandler;

    CachedData _dataCache;

    friend class TestDashboardCachingSilKitEventHandler;
};

} // namespace Dashboard
} // namespace SilKit
