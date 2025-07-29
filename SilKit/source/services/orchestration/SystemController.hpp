// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

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
    inline auto GetServiceDescriptor() const -> const Core::ServiceDescriptor& override;

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
