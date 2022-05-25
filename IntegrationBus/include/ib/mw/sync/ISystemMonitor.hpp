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

    /*! Callback type to indicate that a ParticipantStatus has been received.
    *  Cf., \ref RegisterParticipantStatusHandler(ParticipantStatusHandlerT);
    */
    using ParticipantStatusHandlerT = std::function<void(const ParticipantStatus&)>;

    /*! Callback type to indicate that a participant has been connected.
     * Cf., \ref SetParticipantConnectedHandler(ParticipantConnectedHandler);
     */
    using ParticipantConnectedHandler = std::function<void(const std::string& participantName)>;

    /*! Callback type to indicate that a participant has been disconnected.
     * Cf., \ref SetParticipantDisconnectedHandler(ParticipantDisconnectedHandler);
     */
    using ParticipantDisconnectedHandler = std::function<void(const std::string& participantName)>;

public:
    /*! \brief Register a callback for ::SystemState changes
     *
     * If the current SystemState is not \ref SystemState::Invalid,
     * the handler will be called immediately.
     */
    virtual void RegisterSystemStateHandler(SystemStateHandlerT handler) = 0;

    /*! \brief Register a callback for \ref ParticipantStatus changes
     *
     * The handler will be called immediately for any participant that is
     * not in \ref ParticipantState::Invalid.
     *
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

    /*! \brief Set a callback for participants being connected.
     *
     * @param handler The callback which overwrites any previously set callback
     */
    virtual void SetParticipantConnectedHandler(ParticipantConnectedHandler handler) = 0;

    /*! \brief Set a callback for participants being disconnected.
     *
     * @param handler The callback which overwrites any previously set callback
     */
    virtual void SetParticipantDisconnectedHandler(ParticipantDisconnectedHandler handler) = 0;

    /*! \brief Check if a participant identified by \ref participantName is present.
     *
     * @param participantName The name of the participant for which presence is queried.
     * @return true if the participant is present
     */
    virtual auto IsParticipantConnected(const std::string& participantName) const -> bool = 0;
};

} // namespace sync
} // namespace mw
} // namespace ib
