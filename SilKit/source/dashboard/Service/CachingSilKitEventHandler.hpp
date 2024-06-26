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

#include "ICachingSilKitEventHandler.hpp"

#include <atomic>
#include <future>
#include <memory>

#include "silkit/services/logging/ILogger.hpp"

#include "ISilKitEventQueue.hpp"
#include "ISilKitEventHandler.hpp"


namespace SilKit {
namespace Dashboard {

// Filters own events and process others using a queue
class CachingSilKitEventHandler : public ICachingSilKitEventHandler
{
public:
    CachingSilKitEventHandler(const std::string& connectUri, Services::Logging::ILogger* logger,
                              std::shared_ptr<ISilKitEventHandler> eventHandler,
                              std::shared_ptr<ISilKitEventQueue> eventQueue);
    ~CachingSilKitEventHandler();

public: //methods
    void OnLastParticipantDisconnected() override;
    void OnParticipantConnected(
        const Services::Orchestration::ParticipantConnectionInformation& participantInformation) override;
    void OnParticipantStatusChanged(const Services::Orchestration::ParticipantStatus& participantStatus) override;
    void OnSystemStateChanged(Services::Orchestration::SystemState systemState) override;
    void OnServiceDiscoveryEvent(Core::Discovery::ServiceDiscoveryEvent::Type discoveryType,
                                 const Core::ServiceDescriptor& serviceDescriptor) override;

private: //methods
    void StartSimulationIfNeeded();

private: //member
    std::string _connectUri;
    Services::Logging::ILogger* _logger;
    std::shared_ptr<ISilKitEventHandler> _eventHandler;
    std::shared_ptr<ISilKitEventQueue> _eventQueue;

    std::atomic<bool> _simulationRunning{false};
    std::future<void> _done;
    std::atomic<bool> _abort{false};
};

} // namespace Dashboard
} // namespace SilKit
