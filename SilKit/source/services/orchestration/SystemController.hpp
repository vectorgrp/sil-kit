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


#include "silkit/experimental/services/orchestration/ISystemController.hpp"

#include "IMsgForSystemController.hpp"
#include "IParticipantInternal.hpp"
#include "Assert.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

class SystemController
    : public Experimental::Services::Orchestration::ISystemController
    , public IMsgForSystemController
    , public Core::IServiceEndpoint
{
    public:
    // ----------------------------------------
    // Public Data Types
    
public:
    // ----------------------------------------
    // Constructors, Destructor, and Assignment
    SystemController() = default;
    SystemController(Core::IParticipantInternal* participant);
    SystemController(const SystemController& other) = default;
    SystemController(SystemController&& other) = default;
    SystemController& operator=(const SystemController& other) = default;
    SystemController& operator=(SystemController&& other) = default;
    
public:
    // ----------------------------------------
    // Public Methods
    // ISystemController
    void AbortSimulation() const override;
    void SetWorkflowConfiguration(const WorkflowConfiguration& workflowConfiguration) override;

    // IServiceEndpoint
    inline void SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor) override;
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor & override;

private:
    // ----------------------------------------
    // private methods
    template <class MsgT>
    inline void SendMsg(MsgT&& msg) const;

    inline void SendSystemCommand(SystemCommand::Kind kind) const;
    
private:
    // ----------------------------------------
    // private members
    Core::IParticipantInternal* _participant{nullptr};
    Core::ServiceDescriptor _serviceDescriptor;
};

// ================================================================================
// Inline Implementations
// ================================================================================
template <class MsgT>
void SystemController::SendMsg(MsgT&& msg) const
{
    SILKIT_ASSERT(_participant);
    _participant->SendMsg(this, std::forward<MsgT>(msg));
}
    
void SystemController::SendSystemCommand(SystemCommand::Kind kind) const
{
    SystemCommand cmd;
    cmd.kind = kind;

    SendMsg(std::move(cmd));
}

// ================================================================================
//  Inline Implementations
// ================================================================================

void SystemController::SetServiceDescriptor(const Core::ServiceDescriptor& serviceDescriptor)
{
    _serviceDescriptor = serviceDescriptor;
}

auto SystemController::GetServiceDescriptor() const -> const Core::ServiceDescriptor&
{
    return _serviceDescriptor;
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
