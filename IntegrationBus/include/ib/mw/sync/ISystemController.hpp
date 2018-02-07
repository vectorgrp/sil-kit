// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "SyncDatatypes.hpp"

namespace ib {
namespace mw {
namespace sync {

class ISystemController
{
public:
    /*! \brief Send ParticipantCommand::Initialize to a specific participant
     *
     *  The command is only allowed if the participant is in the \ref
     *  ParticipantState::Idle
     *
     *  \param participantId identifies the participant to be initialized
     *
     *  NB: Parametrization is yet to be determined.
     */
    virtual void Initialize(ParticipantId participantId) const = 0;

    /*! \brief Send ParticipantCommand::ReInitialize to a specific participant
     *
     *  The command is only allowed if the participant is in the \ref
     *  ParticipantState::Stopped or ParticipantState::Error
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

    /*! \brief Send SystemCommand::Run to all participants
     *
     *  The command is only allowed if system is in \ref
     *  SystemState::Initialized.
     */
    virtual void Run() const = 0;


    /*! \brief Send SystemCommand::Stop to all participants
     *
     *  The command is only allowed if system is in \ref
     *  SystemState::Running.
     */
    virtual void Stop() const = 0;

    /*! \brief Send SystemCommand::Shutdown to all participants
     *
     *  The command is only allowed if system is in \ref
     *  SystemState::Stopped or SystemState::Error.
     */
    virtual void Shutdown() const = 0;
};

} // namespace sync
} // namespace mw
} // namespace ib
