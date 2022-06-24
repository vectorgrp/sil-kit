// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <map>
#include <memory>
#include <unordered_set>

#include "ib/mw/sync/ISystemMonitor.hpp"

#include "IIbToSystemMonitor.hpp"
#include "IParticipantInternal.hpp"
#include "IIbServiceEndpoint.hpp"
#include "SynchronizedHandlers.hpp"

namespace ib {
namespace mw {
namespace sync {

class SystemMonitor
    : public ISystemMonitor
    , public IIbToSystemMonitor
    , public mw::IIbServiceEndpoint
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

    auto SystemState() const -> sync::SystemState override;
    auto ParticipantStatus(const std::string& participantName) const -> const sync::ParticipantStatus& override;

    void ReceiveIbMessage(const IIbServiceEndpoint* from, const sync::ParticipantStatus& msg) override;
    void ReceiveIbMessage(const IIbServiceEndpoint* from, const sync::ExpectedParticipants& msg) override;

    void SetParticipantConnectedHandler(ParticipantConnectedHandler handler) override;
    void SetParticipantDisconnectedHandler(ParticipantDisconnectedHandler handler) override;

    auto IsParticipantConnected(const std::string& participantName) const -> bool override;

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;

public:
    // ----------------------------------------
    // Other Public Methods

    const sync::ExpectedParticipants& GetExpectedParticipants() const;

    /*! \brief Get the current transition violation count
     *
     * \return The number of detected invalid participant state transitions.
     */
    inline auto InvalidTransitionCount() const -> unsigned int;

    void UpdateExpectedParticipantNames(const ExpectedParticipants& expectedParticipants);

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
    bool AllParticipantsInState(sync::ParticipantState state) const;
    bool AllParticipantsInState(std::initializer_list<sync::ParticipantState> acceptedStates) const;
    void ValidateParticipantStatusUpdate(const sync::ParticipantStatus& newStatus, sync::ParticipantState oldState);
    void UpdateSystemState(const sync::ParticipantStatus& newStatus);

    inline void SetSystemState(sync::SystemState newState);

private:
    // ----------------------------------------
    // private members
    mw::ServiceDescriptor _serviceDescriptor{};
    logging::ILogger* _logger{nullptr};

    ExpectedParticipants _expectedParticipants{};
    std::map<std::string, sync::ParticipantStatus> _participantStatus;
    sync::SystemState _systemState{sync::SystemState::Invalid};

    unsigned int _invalidTransitionCount{0u};

    util::SynchronizedHandlers<ParticipantStatusHandlerT> _participantStatusHandlers;
    util::SynchronizedHandlers<SystemStateHandlerT> _systemStateHandlers;

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

void SystemMonitor::SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto SystemMonitor::GetServiceDescriptor() const -> const mw::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace sync
} // namespace mw
} // namespace ib
