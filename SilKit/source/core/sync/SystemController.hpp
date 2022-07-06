// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <cassert>

#include "silkit/core/sync/ISystemController.hpp"

#include "IMsgForSystemController.hpp"
#include "IParticipantInternal.hpp"

namespace SilKit {
namespace Core {
namespace Orchestration {

class SystemController
    : public ISystemController
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
    SystemController(IParticipantInternal* participant);
    SystemController(const SystemController& other) = default;
    SystemController(SystemController&& other) = default;
    SystemController& operator=(const SystemController& other) = default;
    SystemController& operator=(SystemController&& other) = default;
    
public:
    // ----------------------------------------
    // Public Methods
    // ISystemController
    void Restart(const std::string& participantName) const override;
    void Run() const override;
    void Stop() const override;
    void Shutdown(const std::string& participantName) const override;
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

    inline void SendParticipantCommand(ParticipantId participantId, ParticipantCommand::Kind kind) const;
    inline void SendSystemCommand(SystemCommand::Kind kind) const;
    
private:
    // ----------------------------------------
    // private members
    IParticipantInternal* _participant{nullptr};
    Core::ServiceDescriptor _serviceDescriptor;
};

// ================================================================================
// Inline Implementations
// ================================================================================
template <class MsgT>
void SystemController::SendMsg(MsgT&& msg) const
{
    assert(_participant);
    _participant->SendMsg(this, std::forward<MsgT>(msg));
}

void SystemController::SendParticipantCommand(ParticipantId participantId, ParticipantCommand::Kind kind) const
{
    ParticipantCommand cmd;
    cmd.participant = participantId;
    cmd.kind = kind;

    SendMsg(std::move(cmd));
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
} // namespace Core
} // namespace SilKit
