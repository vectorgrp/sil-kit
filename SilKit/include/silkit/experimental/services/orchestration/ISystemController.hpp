/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

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
