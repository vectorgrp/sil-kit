// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <map>
#include <memory>

#include "ib/cfg/Config.hpp"
#include "ib/mw/sync/ISystemMonitor.hpp"

#include "IIbToSystemMonitor.hpp"
#include "IComAdapterInternal.hpp"
#include "IServiceId.hpp"

namespace ib {
namespace mw {
namespace sync {

class SystemMonitor
    : public ISystemMonitor
    , public IIbToSystemMonitor
    , public mw::IServiceId
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
    auto ParticipantState(ParticipantId participantId) const -> sync::ParticipantState override;
    auto ParticipantStatus(ParticipantId participantId) const -> const sync::ParticipantStatus& override;

    // IIbToSystemMonitor
    void SetEndpointAddress(const mw::EndpointAddress& addr) override;
    auto EndpointAddress() const -> const mw::EndpointAddress& override;

    void ReceiveIbMessage(const IServiceId* from, const sync::ParticipantStatus& msg) override;

    // IServiceId
    inline void SetServiceId(const mw::ServiceId& serviceId) override;
    inline auto GetServiceId() const -> const mw::ServiceId & override;

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
    mw::ServiceId _serviceId{};
    cfg::SimulationSetup _simulationSetup;
    logging::ILogger* _logger{nullptr};

    std::map<mw::ParticipantId, sync::ParticipantStatus> _participantStatus;
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

void SystemMonitor::SetServiceId(const mw::ServiceId& serviceId)
{
    _serviceId = serviceId;
}

auto SystemMonitor::GetServiceId() const -> const mw::ServiceId&
{
    return _serviceId;
}

} // namespace sync
} // namespace mw
} // namespace ib
