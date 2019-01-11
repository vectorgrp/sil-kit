// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include <map>

#include "ib/mw/sync/ISystemMonitor.hpp"
#include "ib/mw/sync/IIbToSystemMonitor.hpp"

#include "ib/mw/IComAdapter.hpp"
#include "ib/cfg/Config.hpp"

namespace ib {
namespace mw {
namespace sync {


class SystemMonitor
    : public ISystemMonitor
    , public IIbToSystemMonitor
{
public:
    // ----------------------------------------
    // Public Data Types

public:
    // ----------------------------------------
    // Constructors, Destructor, and Assignment
    SystemMonitor() = default;
    SystemMonitor(IComAdapter* comAdapter, cfg::SimulationSetup simulationSetup);
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

    void ReceiveIbMessage(mw::EndpointAddress from, const sync::ParticipantStatus& msg) override;

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
    void UpdateSystemState(const sync::ParticipantStatus& newStatus, sync::ParticipantState oldState);
    inline void SetSystemState(sync::SystemState newState);

private:
    // ----------------------------------------
    // private members
    IComAdapter* _comAdapter{nullptr};
    mw::EndpointAddress _endpointAddress{};
    cfg::SimulationSetup _simulationSetup;

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

} // namespace sync
} // namespace mw
} // namespace ib
