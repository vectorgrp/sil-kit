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
     *  The command is only allowed if the participant is in ParticipantState::ServicesCreated.
     *
     *  \param participantName identifies the participant to be initialized
     *
     *  NB: Parametrization is yet to be determined.
     */
    virtual void Initialize(const std::string& participantName) const = 0;

    /*! \brief Send \ref ParticipantCommand::Kind::Restart to a specific participant
     *
     *  The command is only allowed if the participant is in the
     *  ParticipantState::Stopped or ParticipantState::Error state.
     *
     *  \param participantName identifies the participant to be initialized
     *
     *  NB:
     *   - Parametrization is yet to be determined.
     *   - Restart is still subject to changed! It might be changed to
     *     a SystemCommand to Restart all participants without sending
     *     new parameters.
     */
    virtual void Restart(const std::string& participantName) const = 0;

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
     *  \param participantName identifies the participant to be initialized
     */
    virtual void Shutdown(const std::string& participantName) const = 0;

    /*! \brief Send \ref SystemCommand::Kind::AbortSimulation to all participants
    *
    *  The abort simulation command signals all participants to terminate their
    *  lifecycle, regardless of their current state.
    *
    *  The command is allowed at any time.
    */
    virtual void AbortSimulation() const = 0;

    /*! \brief Configures details of the simulation workflow regarding lifecycle and participant coordination.
    *
    * Only the required participant defined in the \ref WorkflowConfiguration are taken into account to define the 
    * system state. Further, the simulation time propagation also relies on the required participants.
    * The \ref WorkflowConfiguration is distributed to other participants, so it must only be set once by a single 
    * member of the simulation.
    * 
    *  \param workflowConfiguration The desired configuration, currently containing a list of required participants
    */
    virtual void SetWorkflowConfiguration(const WorkflowConfiguration& workflowConfiguration) = 0;
};

} // namespace sync
} // namespace mw
} // namespace ib
