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

#include "SystemController.hpp"
#include "LifecycleService.hpp"
#include "Hash.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

SystemController::SystemController(Core::IParticipantInternal* participant)
    : _participant{participant}
{
}

void SystemController::AbortSimulation() const
{
    // AbortSimulation is directly processed in the lifecycle so that it can be called in user callbacks.
    // Self delivery of the SystemCommand is forbidden via traits.
    auto* lifecycleService = _participant->GetLifecycleService();
    dynamic_cast<SilKit::Services::Orchestration::LifecycleService*>(lifecycleService)
        ->AbortSimulation("SystemController requested AbortSimulation");

    SendSystemCommand(SystemCommand::Kind::AbortSimulation);
}

void SystemController::SetWorkflowConfiguration(const WorkflowConfiguration& workflowConfiguration)
{
    //  Distribute to SystemMonitors (including self delivery) 
    _participant->SendMsg(this, workflowConfiguration);
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
