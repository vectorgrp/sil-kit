// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>
#include <string>

#include "OrchestrationDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

class ISystemMonitor
{
public:
    virtual ~ISystemMonitor() = default;

public:
    /*! Callback type to indicate that a \ref SilKit::Services::Orchestration::SystemState has been received.
    *  Cf., \ref AddSystemStateHandler(SystemStateHandler);
    */
    using SystemStateHandler = std::function<void(SystemState)>;

    /*! Callback type to indicate that a ParticipantStatus has been received.
    *  Cf., \ref AddParticipantStatusHandler(ParticipantStatusHandler);
    */
    using ParticipantStatusHandler = std::function<void(const ParticipantStatus&)>;

    /*! Callback type to indicate that a participant has been connected.
     * Cf., \ref SetParticipantConnectedHandler(ParticipantConnectedHandler);
     */
    using ParticipantConnectedHandler =
        std::function<void(const ParticipantConnectionInformation& participantInformation)>;

    /*! Callback type to indicate that a participant has been disconnected.
     * Cf., \ref SetParticipantDisconnectedHandler(ParticipantDisconnectedHandler);
     */
    using ParticipantDisconnectedHandler =
        std::function<void(const ParticipantConnectionInformation& participantInformation)>;

public:
    /*! \brief Register a callback for \ref SilKit::Services::Orchestration::SystemState changes
     *
     * If the current system state is not \ref SilKit::Services::Orchestration::SystemState::Invalid,
     * the handler will be called immediately.
     *
     * \return Returns a \ref SilKit::Util::HandlerId that can be used to remove the callback.
     */
    virtual auto AddSystemStateHandler(SystemStateHandler handler) -> HandlerId = 0;

    /*! \brief Remove a SystemStateHandler by \ref SilKit::Util::HandlerId on this monitor
     *
     * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
     */
    virtual void RemoveSystemStateHandler(HandlerId handlerId) = 0;

    /*! \brief Register a callback for \ref SilKit::Services::Orchestration::ParticipantStatus changes
     *
     * The handler will be called immediately for any participant that is
     * not in \ref SilKit::Services::Orchestration::ParticipantState::Invalid.
     *
     * \return Returns a \ref SilKit::Util::HandlerId that can be used to remove the callback.
     */
    virtual auto AddParticipantStatusHandler(ParticipantStatusHandler handler) -> HandlerId = 0;

    /*! \brief Remove a ParticipantStatusHandler by \ref SilKit::Util::HandlerId on this monitor
     *
     * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
     */
    virtual void RemoveParticipantStatusHandler(HandlerId handlerId) = 0;

    //! \brief Get the current \ref SilKit::Services::Orchestration::SystemState
    virtual auto SystemState() const -> Orchestration::SystemState = 0;

    /*! \brief Get the current \ref ParticipantStatus of specific participant
     *
     * \param participantName The name of the participant for which the status is queried (UTF-8).
     * \throw SilKit::SilKitError If the participant name does not
     *        identify a participant that participates in synchronization.
     */
    virtual auto ParticipantStatus(const std::string& participantName) const
        -> const Orchestration::ParticipantStatus& = 0;

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

    /*! \brief Check if a participant identified by the participantName is present.
     *
     * @param participantName The name of the participant for which presence is queried (UTF-8).
     * @return true if the participant is present
     */
    virtual auto IsParticipantConnected(const std::string& participantName) const -> bool = 0;
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
