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

#include <algorithm>
#include <ctime>
#include <iomanip> //std:put_time

#include "silkit/services/orchestration/string_utils.hpp"

#include "SystemMonitor.hpp"
#include "IServiceDiscovery.hpp"
#include "ILogger.hpp"
#include "LifecycleService.hpp"
#include "OrchestrationDatatypes.hpp"
#include "VAsioConstants.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

SystemMonitor::SystemMonitor(Core::IParticipantInternal* participant)
    : _logger{participant->GetLogger()}
    , _participant{participant}
{
    _systemStateTracker.SetLogger(_logger);
}

void SystemMonitor::ReceiveMsg(const IServiceEndpoint* /*from*/,
                               const Orchestration::WorkflowConfiguration& workflowConfiguration)
{
    UpdateRequiredParticipantNames(workflowConfiguration.requiredParticipantNames);
    dynamic_cast<LifecycleService*>(_participant->GetLifecycleService())
        ->SetWorkflowConfiguration(workflowConfiguration);
}

void SystemMonitor::UpdateRequiredParticipantNames(const std::vector<std::string>& requiredParticipantNames)
{
    const auto result{_systemStateTracker.UpdateRequiredParticipants(requiredParticipantNames)};

    if (result.systemStateChanged)
    {
        _systemStateHandlers.InvokeAll(SystemState());
    }
}

auto SystemMonitor::AddSystemStateHandler(SystemStateHandler handler) -> HandlerId
{
    handler(SystemState());

    return _systemStateHandlers.Add(std::move(handler));
}

void SystemMonitor::RemoveSystemStateHandler(HandlerId handlerId)
{
    if (!_systemStateHandlers.Remove(handlerId))
    {
        _logger->Warn("RemoveSystemStateHandler failed: Unknown HandlerId.");
    }
}

auto SystemMonitor::AddParticipantStatusHandler(ParticipantStatusHandler handler) -> HandlerId
{
    {
        std::lock_guard<decltype(_connectedParticipantsMx)> lock{_connectedParticipantsMx};

        for (const auto& participantConnectionInformation : _connectedParticipants)
        {
            const auto& participantName{participantConnectionInformation.second.participantName};
            const auto* const participantStatus{_systemStateTracker.GetParticipantStatus(participantName)};

            if (participantStatus == nullptr || participantStatus->state == ParticipantState::Invalid)
            {
                continue;
            }

            handler(*participantStatus);
        }
    }

    return _participantStatusHandlers.Add(std::move(handler));
}

void SystemMonitor::RemoveParticipantStatusHandler(HandlerId handlerId)
{
    if (!_participantStatusHandlers.Remove(handlerId))
    {
        _logger->Warn("RemoveParticipantStatusHandler failed: Unknown HandlerId.");
    }
}

auto SystemMonitor::SystemState() const -> Orchestration::SystemState
{
    return _systemStateTracker.GetSystemState();
}

auto SystemMonitor::ParticipantStatus(const std::string& participantName) const
    -> const SilKit::Services::Orchestration::ParticipantStatus&
{
    const auto* const participantStatus{_systemStateTracker.GetParticipantStatus(participantName)};

    if (participantStatus == nullptr)
    {
        throw SilKitError{"Unknown participant name"};
    }

    return *participantStatus;
}

void SystemMonitor::ReceiveMsg(const IServiceEndpoint* /*from*/,
                               const Orchestration::ParticipantStatus& newParticipantStatus)
{
    const auto result{_systemStateTracker.UpdateParticipantStatus(newParticipantStatus)};

    if (result.participantStateChanged)
    {
        _participantStatusHandlers.InvokeAll(newParticipantStatus);
    }

    if (result.systemStateChanged)
    {
        _systemStateHandlers.InvokeAll(SystemState());
    }
}

void SystemMonitor::SetParticipantConnectedHandler(ParticipantConnectedHandler handler)
{
    _participantConnectedHandler = std::move(handler);
}

void SystemMonitor::SetParticipantDisconnectedHandler(ParticipantDisconnectedHandler handler)
{
    _participantDisconnectedHandler = std::move(handler);
}

auto SystemMonitor::IsParticipantConnected(const std::string& participantName) const -> bool
{
    std::lock_guard<decltype(_connectedParticipantsMx)> lock{_connectedParticipantsMx};
    const auto it{_connectedParticipants.find(participantName)};
    return it != _connectedParticipants.end();
}

void SystemMonitor::OnParticipantConnected(const ParticipantConnectionInformation& participantConnectionInformation)
{
    {
        std::unique_lock<decltype(_connectedParticipantsMx)> lock{_connectedParticipantsMx};

        // Add the participant name to the map of connected participant names/connections
        _connectedParticipants.emplace(participantConnectionInformation.participantName,
                                       participantConnectionInformation);
    }

    // Call the handler if set
    if (_participantConnectedHandler)
    {
        _participantConnectedHandler(participantConnectionInformation);
    }
}

void SystemMonitor::OnParticipantDisconnected(const ParticipantConnectionInformation& participantConnectionInformation)
{
    const auto& participantName = participantConnectionInformation.participantName;
    const auto* const participantStatus{_systemStateTracker.GetParticipantStatus(participantName)};

    if (participantStatus != nullptr)
    {
        const auto participantState = participantStatus->state;

        if (participantState == Orchestration::ParticipantState::Shutdown)
        {
            // If current known participant state is ParticipantState::Shutdown, we do not bother changing state
            Logging::Info(_logger, "Participant \'{}\' has disconnected after gracefully shutting down",
                          participantName);
        }
        else if (participantState != ParticipantState::Invalid)
        {
            // If participant has any other state except ParticipantState::Invalid and ParticipantState::Shutdown,
            // he has a started lifecycle that was not shut down gracefully.

            // Update disconnected participant to ParticipantState::Error
            auto status = Orchestration::ParticipantStatus{};
            status.participantName = participantName;
            status.state = SilKit::Services::Orchestration::ParticipantState::Error;
            status.enterReason = "Connection Lost";
            status.enterTime = std::chrono::system_clock::now();
            status.refreshTime = std::chrono::system_clock::now();

            ReceiveMsg(nullptr, status);

            Logging::Error(_logger, "Participant \'{}\' has disconnected without gracefully shutting down.",
                           participantName);
        }
    }
    else if (participantName == VSilKit::REGISTRY_PARTICIPANT_NAME)
    {
        Logging::Error(_logger,
                       "Connection to SIL Kit Registry was lost - no new participant connections can be established.");
    }
    else
    {
        // This disconnecting participant is not the SIL Kit Registry and has no lifecycle.
        Logging::Info(_logger, "Participant \'{}\' has disconnected.",
                      participantConnectionInformation.participantName);
    }

    // Erase participant from connectedParticipant map
    {
        std::unique_lock<decltype(_connectedParticipantsMx)> lock{_connectedParticipantsMx};
        auto it = _connectedParticipants.find(participantConnectionInformation.participantName);
        if (it != _connectedParticipants.end())
        {
            _connectedParticipants.erase(it);
        }
    }

    // Remove the participant from the state tracker and process any resulting system state changes
    {
        const auto result{_systemStateTracker.RemoveParticipant(participantName)};

        if (result.systemStateChanged)
        {
            _systemStateHandlers.InvokeAll(SystemState());
        }
    }

    // Call the handler if set
    if (_participantDisconnectedHandler)
    {
        _participantDisconnectedHandler(participantConnectionInformation);
    }
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
