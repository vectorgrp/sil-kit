// Copyright (c) Vector Informatik GmbH. All rights reserved.

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
    /*! Callback type to indicate that a ::SystemState has been received.
    *  Cf., \ref RegisterSystemStateHandler(SystemStateHandlerT);
    */
    using SystemStateHandlerT = std::function<void(SystemState)>;

    /*! Callback type to indicate that a ::ParticipantState has been received.
    *  Cf., \ref RegisterParticipantStateHandler(ParticipantStateHandlerT);
    */
    using ParticipantStateHandlerT = std::function<void(ParticipantState)>;

    /*! Callback type to indicate that a ParticipantStatus has been received.
    *  Cf., \ref RegisterParticipantStatusHandler(ParticipantStatusHandlerT);
    */
    using ParticipantStatusHandlerT = std::function<void(const ParticipantStatus&)>;

public:
    /*! \brief Register a callback for ::SystemState changes
     *
     * If the current SystemState is not \ref SystemState::Invalid,
     * the handler will be called immediately.
     */
    virtual void RegisterSystemStateHandler(SystemStateHandlerT handler) = 0;

    /*! \brief Register a callback for ::ParticipantState changes
     *
     * The handler will be called immediately for any participant that is
     * not in \ref ParticipantState::Invalid.
     *
     * NB: ParticipantStatusHandlers and ParticipantStateHandlers are
     * always called in combination.
     */
    virtual void RegisterParticipantStateHandler(ParticipantStateHandlerT handler) = 0;

    /*! \brief Register a callback for \ref ParticipantStatus changes
     *
     * The handler will be called immediately for any participant that is
     * not in \ref ParticipantState::Invalid.
     *
     * NB: ParticipantStatusHandlers and ParticipantStateHandlers are
     * always called in combination.
     */
    virtual void RegisterParticipantStatusHandler(ParticipantStatusHandlerT handler) = 0;

    //! \brief Get the current ::SystemState
    virtual auto SystemState() const -> sync::SystemState = 0;

    /*! \brief Get the current \ref ParticipantStatus of specific participant
     *
     * \param participantName The name of the participant for which the status is queried.
     * \throw std::runtime_error If the participantId does not
     *        identify a participant that participates in synchronization.
     */
    virtual auto ParticipantStatus(const std::string& participantName) const -> const sync::ParticipantStatus& = 0;
};

} // namespace sync
} // namespace mw
} // namespace ib
