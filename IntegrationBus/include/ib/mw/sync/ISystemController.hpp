// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "SyncDatatypes.hpp"

namespace ib {
namespace mw {
namespace sync {

class ISystemController
{
public:
    /*! \brief Send \ref ParticipantCommand::Kind::Initialize to a specific participant
     *
     *  The command is only allowed if the participant is in ParticipantState::Idle.
     *
     *  \param participantId identifies the participant to be initialized
     *
     *  NB: Parametrization is yet to be determined.
     */
    virtual void Initialize(ParticipantId participantId) const = 0;

    /*! \brief Send \ref ParticipantCommand::Kind::ReInitialize to a specific participant
     *
     *  The command is only allowed if the participant is in the
     *  ParticipantState::Stopped or ParticipantState::Error state.
     *
     *  \param participantId identifies the participant to be initialized
     *
     *  NB:
     *   - Parametrization is yet to be determined.
     *   - ReInitialize is still subject to changed! It might be changed to
     *     a SystemCommand to ReInitialize all participants without sending
     *     new parameters.
     */
    virtual void ReInitialize(ParticipantId participantId) const = 0;

    /*! \brief Send \ref SystemCommand::Kind::Run to all participants
     *
     *  The command is only allowed if system is in SystemState::Initialized.
     */
    virtual void Run() const = 0;

    /*! \brief Send \ref SystemCommand::Kind::Stop to all participants
     *
     *  The command is only allowed if system is in SystemState::Running.
     */
    virtual void Stop() const = 0;

    /*! \brief Send \ref SystemCommand::Kind::Shutdown to all participants
     *
     *  The command is only allowed if system is in
     *  SystemState::Stopped or SystemState::Error.
     */
    virtual void Shutdown() const = 0;

    /*! \brief Send \ref SystemCommand::Kind::PrepareColdswap to all participants
    *
    *  The coldswap process is used to restart a simulation but allow
    *  to swap out one or more participants. The PrepareColdswap()
    *  command brings the system into a safe state such that the actual
    *  coldswap can be performed without loss of data.
    * 
    *  The command is only allowed if system is in
    *  SystemState::Stopped or SystemState::Error.
    */
    virtual void PrepareColdswap() const = 0;

    /*! \brief Send \ref SystemCommand::Kind::ExecuteColdswap to all participants
    *
    *  The coldswap process is used to restart a simulation but allow
    *  to swap out one or more participants. Once the system is ready
    *  to perform a coldswap, the actual coldswap can be initiated with
    *  the ExecuteColdswap() command.
    *
    *  The command is only allowed if system is in SystemState::ColdswapReady
    */
    virtual void ExecuteColdswap() const = 0;
};

} // namespace sync
} // namespace mw
} // namespace ib
