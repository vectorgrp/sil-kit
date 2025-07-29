// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/services/orchestration/OrchestrationDatatypes.hpp"

namespace SilKit {
namespace Experimental {
namespace Services {
namespace Orchestration {

//! SystemController interface for configuration of the simulation flow and system-wide commands.
class ISystemController
{
public:
    virtual ~ISystemController() = default;

    /*! \brief Sends a SilKit::Services::Orchestration::SystemCommand::Kind::AbortSimulation to all participants
    *
    *  The abort simulation command signals all participants to terminate their
    *  lifecycle, regardless of their current state.
    *
    *  The command is allowed at any time.
    */
    virtual void AbortSimulation() const = 0;

    /*! \brief Configures details of the simulation workflow regarding lifecycle and participant coordination.
    *
    * Only the required participant defined in the \ref SilKit::Services::Orchestration::WorkflowConfiguration are taken into account to define the 
    * system state. Further, the simulation time propagation also relies on the required participants.
    * The \ref SilKit::Services::Orchestration::WorkflowConfiguration is distributed to other participants, so it must only be set once by a single 
    * member of the simulation.
    * 
    *  \param workflowConfiguration The desired configuration, currently containing a list of required participants
    */
    virtual void SetWorkflowConfiguration(
        const SilKit::Services::Orchestration::WorkflowConfiguration& workflowConfiguration) = 0;
};

} // namespace Orchestration
} // namespace Services
} // namespace Experimental
} // namespace SilKit
