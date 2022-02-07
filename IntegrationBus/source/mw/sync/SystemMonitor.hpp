// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <map>
#include <memory>

#include "ib/cfg/Config.hpp"
#include "ib/mw/sync/ISystemMonitor.hpp"

#include "IIbToSystemMonitor.hpp"
#include "IComAdapterInternal.hpp"
#include "IIbServiceEndpoint.hpp"

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
    SystemMonitor(IComAdapterInternal* comAdapter, cfg::SimulationSetup simulationSetup);
    SystemMonitor(const SystemMonitor& other) = default;
    SystemMonitor(SystemMonitor&& other) = default;
    SystemMonitor& operator=(const SystemMonitor& other) = default;
    SystemMonitor& operator=(SystemMonitor&& other) = default;

public:
    // ----------------------------------------
    // Public Interface Methods
    // ISystemMonitor
    void RegisterSystemStateHandler(SystemStateHandlerT handler) override;
    void RegisterParticipantStateHandler(ParticipantStateHandlerT handler) override;
    void RegisterParticipantStatusHandler(ParticipantStatusHandlerT handler) override;

    auto SystemState() const -> sync::SystemState override;
    auto ParticipantStatus(const std::string& participantId) const -> const sync::ParticipantStatus& override;

    void ReceiveIbMessage(const IIbServiceEndpoint* from, const sync::ParticipantStatus& msg) override;

    // IIbServiceEndpoint
    inline void SetServiceDescriptor(const mw::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const mw::ServiceDescriptor & override;

public:
    // ----------------------------------------
    // Other Public Methods

    /*! \brief Get the current transition violation count
     *
     * \return The number of detected invalid participant state transitions.
     */
    inline auto InvalidTransitionCount() const -> unsigned int;

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
    IComAdapterInternal* _comAdapter{nullptr};
    mw::ServiceDescriptor _serviceDescriptor{};
    cfg::SimulationSetup _simulationSetup;
    logging::ILogger* _logger{nullptr};

    std::map<std::string, sync::ParticipantStatus> _participantStatus;
    sync::SystemState _systemState{sync::SystemState::Invalid};

    unsigned int _invalidTransitionCount{0u};

    std::vector<ParticipantStateHandlerT> _participantStateHandlers;
    std::vector<ParticipantStatusHandlerT> _participantStatusHandlers;
    std::vector<SystemStateHandlerT> _systemStateHandlers;
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
