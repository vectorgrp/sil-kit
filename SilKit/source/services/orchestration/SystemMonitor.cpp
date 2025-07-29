// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <algorithm>
#include <ctime>
#include <iomanip> //std:put_time

#include "silkit/services/orchestration/string_utils.hpp"

#include "SystemMonitor.hpp"
#include "IServiceDiscovery.hpp"
#include "LoggerMessage.hpp"
#include "LifecycleService.hpp"
#include "OrchestrationDatatypes.hpp"
#include "VAsioConstants.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

SystemMonitor::SystemMonitor(Core::IParticipantInternal* participant)
    : _logger{participant->GetLoggerInternal()}
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
            struct ParticipantStatus participantStatus;
            const bool present = _systemStateTracker.GetParticipantStatus(participantName, participantStatus);

            if (!present || participantStatus.state == ParticipantState::Invalid)
            {
                continue;
            }

            handler(participantStatus);
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
    {
        std::lock_guard<decltype(_connectedParticipantsMx)> lock{_connectedParticipantsMx};

        for (const auto& participantConnectionInformation : _connectedParticipants)
        {
            ParticipantConnectionInformation participantConnectionInfoCopy{
                participantConnectionInformation.second.participantName};
            handler(participantConnectionInfoCopy);
        }
    }
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
    bool hasInserted = false;
    {
        std::unique_lock<decltype(_connectedParticipantsMx)> lock{_connectedParticipantsMx};

        // Add the participant name to the map of connected participant names/connections
        const auto pair = _connectedParticipants.emplace(participantConnectionInformation.participantName,
                                                         participantConnectionInformation);
        hasInserted = pair.second;
    }

    // Call the handler if set
    if (_participantConnectedHandler && hasInserted)
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
