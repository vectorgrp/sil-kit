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

#include "CachingSilKitEventHandler.hpp"

#include <chrono>

#include "LoggerMessage.hpp"
#include "SetThreadName.hpp"

namespace SilKit {
namespace Dashboard {

uint64_t GetCurrentTime()
{
    auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}

CachingSilKitEventHandler::CachingSilKitEventHandler(const std::string& connectUri, Services::Logging::ILogger* logger,
                                                     std::shared_ptr<ISilKitEventHandler> eventHandler,
                                                     std::shared_ptr<ISilKitEventQueue> eventQueue)
    : _connectUri(connectUri)
    , _logger(logger)
    , _eventHandler(eventHandler)
    , _eventQueue(eventQueue)
{
    _done = std::async(std::launch::async, [this]() {
        SilKit::Util::SetThreadName("SK-Dash-Cons");
        uint64_t simulationId = 0;
        std::vector<SilKitEvent> events;
        while (_eventQueue->DequeueAllInto(events))
        {
            for (SilKitEvent& evt : events)
            {
                if (_abort)
                {
                    return;
                }
                switch (evt.Type())
                {
                case SilKitEventType::OnSimulationStart:
                {
                    const SimulationStart& simulationStart = evt.GetSimulationStart();
                    simulationId = _eventHandler->OnSimulationStart(simulationStart.connectUri, simulationStart.time);
                }
                break;

                case SilKitEventType::OnParticipantConnected:
                    if (simulationId > 0)
                    {
                        const Services::Orchestration::ParticipantConnectionInformation&
                            participantConnectionInformation = evt.GetParticipantConnectionInformation();
                        _eventHandler->OnParticipantConnected(simulationId, participantConnectionInformation);
                    }
                    break;

                case SilKitEventType::OnSystemStateChanged:
                    if (simulationId > 0)
                    {
                        const Services::Orchestration::SystemState& systemState = evt.GetSystemState();
                        _eventHandler->OnSystemStateChanged(simulationId, systemState);
                    }
                    break;

                case SilKitEventType::OnParticipantStatusChanged:
                    if (simulationId > 0)
                    {
                        const Services::Orchestration::ParticipantStatus& participantStatus =
                            evt.GetParticipantStatus();
                        _eventHandler->OnParticipantStatusChanged(simulationId, participantStatus);
                    }
                    break;

                case SilKitEventType::OnServiceDiscoveryEvent:
                    if (simulationId > 0)
                    {
                        const ServiceData& serviceData = evt.GetServiceData();
                        _eventHandler->OnServiceDiscoveryEvent(simulationId, serviceData.discoveryType,
                                                               serviceData.serviceDescriptor);
                    }
                    break;

                case SilKitEventType::OnSimulationEnd:
                    if (simulationId > 0)
                    {
                        const SimulationEnd& simulationEnd = evt.GetSimulationEnd();
                        _eventHandler->OnSimulationEnd(simulationId, simulationEnd.time);
                    }
                    simulationId = 0;
                    break;
                default:
                    _logger->Error("Dashboard: unexpected SilKitEventType");
                }
            }
            events.clear();
        }
    });
}

CachingSilKitEventHandler::~CachingSilKitEventHandler()
{
    _eventQueue->Stop();
    auto status = _done.wait_for(std::chrono::seconds{2});
    if (status != std::future_status::ready)
    {
        _logger->Warn("Dashboard: aborting");
        _abort = true;
    }
    _done.wait();
}

void CachingSilKitEventHandler::OnLastParticipantDisconnected()
{
    if (_simulationRunning)
    {
        _eventQueue->Enqueue(SilKitEvent({}, SimulationEnd{GetCurrentTime()}));
        _simulationRunning = false;
    }
}

void CachingSilKitEventHandler::OnParticipantConnected(
    const Services::Orchestration::ParticipantConnectionInformation& participantInformation)
{
    StartSimulationIfNeeded();
    _eventQueue->Enqueue(SilKitEvent({}, participantInformation));
}

void CachingSilKitEventHandler::OnSystemStateChanged(Services::Orchestration::SystemState systemState)
{
    if (_simulationRunning)
    {
        _eventQueue->Enqueue(SilKitEvent({}, systemState));
    }
}

void CachingSilKitEventHandler::OnParticipantStatusChanged(
    const Services::Orchestration::ParticipantStatus& participantStatus)
{
    if (_simulationRunning)
    {
        _eventQueue->Enqueue(SilKitEvent({}, participantStatus));
    }
}

bool ShouldSkipServiceDiscoveryEvent(Core::Discovery::ServiceDiscoveryEvent::Type discoveryType,
                                     const Core::ServiceDescriptor& serviceDescriptor)
{
    return discoveryType != Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated
           || (serviceDescriptor.GetServiceType() != Core::ServiceType::Controller
               && serviceDescriptor.GetServiceType() != Core::ServiceType::Link);
}

void CachingSilKitEventHandler::OnServiceDiscoveryEvent(Core::Discovery::ServiceDiscoveryEvent::Type discoveryType,
                                                        const Core::ServiceDescriptor& serviceDescriptor)
{
    if (_simulationRunning)
    {
        if (ShouldSkipServiceDiscoveryEvent(discoveryType, serviceDescriptor))
        {
            return;
        }
        _eventQueue->Enqueue(SilKitEvent({}, ServiceData{discoveryType, serviceDescriptor}));
    }
}

void CachingSilKitEventHandler::StartSimulationIfNeeded()
{
    bool simulationRunning = false;
    if (_simulationRunning.compare_exchange_strong(simulationRunning, true))
    {
        _eventQueue->Enqueue(SilKitEvent({}, SimulationStart{_connectUri, GetCurrentTime()}));
    }
}

} // namespace Dashboard
} // namespace SilKit
