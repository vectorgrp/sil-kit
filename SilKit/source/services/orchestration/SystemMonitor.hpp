// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <map>
#include <memory>
#include <unordered_set>

#include "silkit/core/sync/ISystemMonitor.hpp"

#include "IMsgForSystemMonitor.hpp"
#include "IParticipantInternal.hpp"
#include "IServiceEndpoint.hpp"
#include "SynchronizedHandlers.hpp"

namespace SilKit {
namespace Core {
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
    SystemMonitor(IParticipantInternal* participant);
    SystemMonitor(const SystemMonitor& other) = delete;
    SystemMonitor(SystemMonitor&& other) = delete;
    SystemMonitor& operator=(const SystemMonitor& other) = delete;
    SystemMonitor& operator=(SystemMonitor&& other) = delete;

public:
    // ----------------------------------------
    // Public Interface Methods

    // ISystemMonitor
    auto AddSystemStateHandler(SystemStateHandlerT handler) -> HandlerId override;
    void RemoveSystemStateHandler(HandlerId handlerId) override;

    auto AddParticipantStatusHandler(ParticipantStatusHandlerT handler) -> HandlerId override;
    void RemoveParticipantStatusHandler(HandlerId handlerId) override;

    auto SystemState() const -> Orchestration::SystemState override;
    auto ParticipantStatus(const std::string& participantName) const -> const Orchestration::ParticipantStatus& override;

    void ReceiveSilKitMessage(const IServiceEndpoint* from, const Orchestration::ParticipantStatus& msg) override;
    void ReceiveSilKitMessage(const IServiceEndpoint* from, const Orchestration::WorkflowConfiguration& msg) override;

    void SetParticipantConnectedHandler(ParticipantConnectedHandler handler) override;
    void SetParticipantDisconnectedHandler(ParticipantDisconnectedHandler handler) override;

    auto IsParticipantConnected(const std::string& participantName) const -> bool override;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor & override;

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
     * @param participantName The name of participant that connected
     */
    void OnParticipantConnected(const std::string& participantName);

    /*! \brief Invokes the handler set by \ref SetParticipantDisconnectedHandler
     *
     * @param participantName The name of participant that disconnected
     */
    void OnParticipantDisconnected(const std::string& participantName);

private:
    // ----------------------------------------
    // private methods
    bool AllRequiredParticipantsInState(std::initializer_list<Orchestration::ParticipantState> acceptedStates) const;
    void ValidateParticipantStatusUpdate(const Orchestration::ParticipantStatus& newStatus, Orchestration::ParticipantState oldState);
    void UpdateSystemState(const Orchestration::ParticipantStatus& newStatus);

    inline void SetSystemState(Orchestration::SystemState newState);

private:
    // ----------------------------------------
    // private members
    Core::ServiceDescriptor _serviceDescriptor{};
    Services::Logging::ILogger* _logger{nullptr};

    std::vector<std::string> _requiredParticipantNames{};
    std::map<std::string, Orchestration::ParticipantStatus> _participantStatus;
    Orchestration::SystemState _systemState{Orchestration::SystemState::Invalid};

    unsigned int _invalidTransitionCount{0u};

    Util::SynchronizedHandlers<ParticipantStatusHandlerT> _participantStatusHandlers;
    Util::SynchronizedHandlers<SystemStateHandlerT> _systemStateHandlers;

    ParticipantConnectedHandler _participantConnectedHandler;
    ParticipantDisconnectedHandler _participantDisconnectedHandler;
    std::unordered_set<std::string> _connectedParticipantNames;
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
} // namespace Core
} // namespace SilKit
