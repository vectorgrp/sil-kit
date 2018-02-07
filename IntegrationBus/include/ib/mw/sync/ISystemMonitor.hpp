// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include <functional>
#include <future>
#include <string>

#include "SyncDatatypes.hpp"

namespace ib {
namespace mw {
namespace sync {

class ISystemMonitor
{
public:
    using SystemStateHandlerT = std::function<void(SystemState)>;
    using ParticipantStateHandlerT = std::function<void(ParticipantState)>;
    using ParticipantStatusHandlerT = std::function<void(ParticipantStatus)>;

public:
    //! \brief Register a callback for SystemState changes
    virtual void RegisterSystemStateHandler(SystemStateHandlerT handler) = 0;

    /*! \brief Register a callback for ParticipantState changes
     *
     *  NB: ParticipantStatusHandlers and ParticipantStateHandlers are
     *  always called in combination.
     */
    virtual void RegisterParticipantStateHandler(ParticipantStateHandlerT handler) = 0;

    /*! \brief Register a callback for ParticipantStatus changes
     *
     *  NB: ParticipantStatusHandlers and ParticipantStateHandlers are
     *  always called in combination.
     */
    virtual void RegisterParticipantStatusHandler(ParticipantStatusHandlerT handler) = 0;

    //! \brief Get the current SystemState
    virtual auto SystemState() const -> sync::SystemState = 0;

    /*! \brief Get the current ParticipantState of specific participant
     *
     * \param participantId The participant for which the state is queried
     * \throw std::runtime_error if the participantId does not
     *        identify a participant that participates in synchronization
     */
    virtual auto ParticipantState(ParticipantId participantId) const -> sync::ParticipantState = 0;

    /*! \brief Get the current ParticipantStatus of specific participant
     *
     * \param participantId The participant for which the status is queried
     * \throw std::runtime_error if the participantId does not
     *        identify a participant that participates in synchronization
     */
    virtual auto ParticipantStatus(ParticipantId participantId) const -> const sync::ParticipantStatus& = 0;
};

} // namespace sync
} // namespace mw
} // namespace ib
