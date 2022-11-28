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

#include "ILogger.hpp"
#include "SetThreadName.hpp"

namespace SilKit {
namespace Dashboard {

CachingSilKitEventHandler::CachingSilKitEventHandler(Services::Logging::ILogger* logger,
                                                     const std::string& participantName,
                                                     std::shared_ptr<ISilKitEventHandler> eventHandler)
    : _logger(logger)
    , _participantName(participantName)
    , _eventHandler(eventHandler)
{
}

CachingSilKitEventHandler::~CachingSilKitEventHandler()
{
    if (_simulationCreationThread.joinable())
    {
        _simulationCreationThread.join();
    }
}

std::future<bool> CachingSilKitEventHandler::OnStart(const std::string& connectUri, uint64_t time)
{
    _simulationCreationThread = std::thread{[this, &connectUri, time]() {
        SilKit::Util::SetThreadName("SK-Dash-Caching");
        Run(connectUri, time);
    }};
    return _simulationCreatedPromise.get_future();
}

void CachingSilKitEventHandler::Run(const std::string& connectUri, uint64_t time)
{
    auto simulationCreated = _eventHandler->OnStart(connectUri, time);
    std::future_status simulationCreatedStatus;
    do
    {
        simulationCreatedStatus = simulationCreated.wait_for(std::chrono::seconds{1});
    } while (_state != Disabled && simulationCreatedStatus != std::future_status::ready);
    if (simulationCreatedStatus != std::future_status::ready)
    {
        _logger->Warn("Dashboard: already disabled");
        _simulationCreatedPromise.set_value(false);
        return;
    }
    if (simulationCreated.get())
    {
        auto expectedState = Caching;
        if (_state.compare_exchange_strong(expectedState, Sending))
        {
            _logger->Info("Dashboard: notifying cached events");
            NotifyCachedEvents();
            _simulationCreatedPromise.set_value(true);
            return;
        }
        _logger->Warn("Dashboard: already disabled");
        _simulationCreatedPromise.set_value(false);
        return;
    }
    _logger->Warn("Dashboard: simulation creation failed, disabling caching");
    _state = Disabled;
    _simulationCreatedPromise.set_value(false);
    return;
}

void CachingSilKitEventHandler::OnShutdown(uint64_t time)
{
    auto expectedState = Sending;
    if (_state.compare_exchange_strong(expectedState, Disabled))
    {
        _eventHandler->OnShutdown(time);
        return;
    }
    _logger->Warn("Dashboard: not sending, skipping setting an end");
    _state = Disabled;
}

void CachingSilKitEventHandler::OnParticipantConnected(
    const Services::Orchestration::ParticipantConnectionInformation& participantInformation)
{
    if (participantInformation.participantName == _participantName)
    {
        return;
    }
    switch (_state)
    {
    case Disabled: return;
    case Caching: _dataCache.Insert(participantInformation); return;
    case Sending: _eventHandler->OnParticipantConnected(participantInformation);
    }
}

void CachingSilKitEventHandler::OnSystemStateChanged(Services::Orchestration::SystemState systemState)
{
    switch (_state)
    {
    case Disabled: return;
    case Caching: _dataCache.Insert(systemState); return;
    case Sending: _eventHandler->OnSystemStateChanged(systemState);
    }
}

void CachingSilKitEventHandler::OnParticipantStatusChanged(
    const Services::Orchestration::ParticipantStatus& participantStatus)
{
    if (participantStatus.participantName == _participantName)
    {
        return;
    }
    switch (_state)
    {
    case Disabled: return;
    case Caching: _dataCache.Insert(participantStatus); return;
    case Sending: _eventHandler->OnParticipantStatusChanged(participantStatus);
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
    if (serviceDescriptor.GetParticipantName() == _participantName)
    {
        return;
    }
    if (ShouldSkipServiceDiscoveryEvent(discoveryType, serviceDescriptor))
    {
        return;
    }
    switch (_state)
    {
    case Disabled: return;
    case Caching: _dataCache.Insert(ServiceData{discoveryType, serviceDescriptor}); return;
    case Sending: _eventHandler->OnServiceDiscoveryEvent(discoveryType, serviceDescriptor);
    }
}

void CachingSilKitEventHandler::NotifyCachedEvents()
{
    {
        auto&& cacheCopy = _dataCache.GetAndClear<Services::Orchestration::ParticipantConnectionInformation>();
        for (auto&& pci : cacheCopy)
        {
            if (_state == Disabled)
            {
                return;
            }
            _eventHandler->OnParticipantConnected(pci);
        }
    }
    {
        auto&& cacheCopy = _dataCache.GetAndClear<ServiceData>();
        for (auto&& descr : cacheCopy)
        {
            if (_state == Disabled)
            {
                return;
            }
            _eventHandler->OnServiceDiscoveryEvent(descr.discoveryType, descr.serviceDescriptor);
        }
    }
    {
        auto&& cacheCopy = _dataCache.GetAndClear<Services::Orchestration::ParticipantStatus>();
        for (auto&& status : cacheCopy)
        {
            if (_state == Disabled)
            {
                return;
            }
            _eventHandler->OnParticipantStatusChanged(status);
        }
    }
    {
        auto&& cacheCopy = _dataCache.GetAndClear<Services::Orchestration::SystemState>();
        for (auto&& status : cacheCopy)
        {
            if (_state == Disabled)
            {
                return;
            }
            _eventHandler->OnSystemStateChanged(status);
        }
    }
}

} // namespace Dashboard
} // namespace SilKit
