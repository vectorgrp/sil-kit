// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <map>
#include <memory>
#include <unordered_set>
#include <unordered_map>

#include "silkit/services/orchestration/ISystemMonitor.hpp"

#include "IMsgForSystemMonitor.hpp"
#include "IParticipantInternal.hpp"
#include "IServiceEndpoint.hpp"
#include "SynchronizedHandlers.hpp"
#include "SystemStateTracker.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

class SystemMonitor
    : public ISystemMonitor
    , public IMsgForSystemMonitor
    , public Core::IServiceEndpoint
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors, Destructor, and Assignment
    SystemMonitor() = default;
    SystemMonitor(Core::IParticipantInternal* participant);
    SystemMonitor(const SystemMonitor& other) = delete;
    SystemMonitor(SystemMonitor&& other) = delete;
    SystemMonitor& operator=(const SystemMonitor& other) = delete;
    SystemMonitor& operator=(SystemMonitor&& other) = delete;

public:
    // ----------------------------------------
    // Public Interface Methods

    // ISystemMonitor
    auto AddSystemStateHandler(SystemStateHandler handler) -> HandlerId override;
    void RemoveSystemStateHandler(HandlerId handlerId) override;

    auto AddParticipantStatusHandler(ParticipantStatusHandler handler) -> HandlerId override;
    void RemoveParticipantStatusHandler(HandlerId handlerId) override;

    auto SystemState() const -> Orchestration::SystemState override;
    auto ParticipantStatus(const std::string& participantName) const
        -> const Orchestration::ParticipantStatus& override;

    void ReceiveMsg(const IServiceEndpoint* from, const Orchestration::ParticipantStatus& msg) override;
    void ReceiveMsg(const IServiceEndpoint* from, const Orchestration::WorkflowConfiguration& msg) override;

    void SetParticipantConnectedHandler(ParticipantConnectedHandler handler) override;
    void SetParticipantDisconnectedHandler(ParticipantDisconnectedHandler handler) override;

    auto IsParticipantConnected(const std::string& participantName) const -> bool override;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor& override;

public:
    // ----------------------------------------
    // Other Public Methods

    /*! \brief Get the current transition violation count
     *
     * \return The number of detected invalid participant state transitions.
     */
    inline auto InvalidTransitionCount() const -> unsigned int;

    void UpdateRequiredParticipantNames(const std::vector<std::string>& requiredParticipantNames);

    /*! \brief Invokes the handler set by \ref SetParticipantConnectedHandler
     *
     * @param participantConnectionInformation The struct that provides information about the connected participant
     */
    void OnParticipantConnected(const ParticipantConnectionInformation& participantConnectionInformation);

    /*! \brief Invokes the handler set by \ref SetParticipantDisconnectedHandler
     *
     * @param participantConnectionInformation The struct that provides information about the disconnected participant
     */
    void OnParticipantDisconnected(const ParticipantConnectionInformation& participantConnectionInformation);

private:
    // ----------------------------------------
    // private members
    Core::ServiceDescriptor _serviceDescriptor{};
    Services::Logging::ILoggerInternal* _logger{nullptr};
    Core::IParticipantInternal* _participant{nullptr};

    VSilKit::SystemStateTracker _systemStateTracker;

    unsigned int _invalidTransitionCount{0u};

    Util::SynchronizedHandlers<ParticipantStatusHandler> _participantStatusHandlers;
    Util::SynchronizedHandlers<SystemStateHandler> _systemStateHandlers;

    ParticipantConnectedHandler _participantConnectedHandler;
    ParticipantDisconnectedHandler _participantDisconnectedHandler;

    mutable std::mutex _connectedParticipantsMx;
    std::unordered_map<std::string, ParticipantConnectionInformation> _connectedParticipants;
};

// ================================================================================
//  Inline Implementations
// ================================================================================
auto SystemMonitor::InvalidTransitionCount() const -> unsigned int
{
    return _invalidTransitionCount;
}

void SystemMonitor::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto SystemMonitor::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
