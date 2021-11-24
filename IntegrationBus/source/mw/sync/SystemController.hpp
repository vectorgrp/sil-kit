// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/sync/ISystemController.hpp"


#include <cassert>

#include "IIbToSystemController.hpp"
#include "IComAdapterInternal.hpp"

namespace ib {
namespace mw {
namespace sync {

class SystemController
    : public ISystemController
    , public IIbToSystemController
    , public mw::IIbServiceEndpoint
{
    public:
    // ----------------------------------------
    // Public Data Types
    
public:
    // ----------------------------------------
    // Constructors, Destructor, and Assignment
    SystemController() = default;
    SystemController(IComAdapterInternal* comAdapter);
    SystemController(const SystemController& other) = default;
    SystemController(SystemController&& other) = default;
    SystemController& operator=(const SystemController& other) = default;
    SystemController& operator=(SystemController&& other) = default;
    
public:
    // ----------------------------------------
    // Public Methods
    // ISystemController
    void Initialize(ParticipantId participantId) const override;
    void ReInitialize(ParticipantId participantId) const override;
    void Run() const override;
    void Stop() const override;
    void Shutdown() const override;
    void PrepareColdswap() const override;
    void ExecuteColdswap() const override;

    // IIbToSystemMonitor
    void SetEndpointAddress(const mw::EndpointAddress& addr) override;
    auto EndpointAddress() const -> const mw::EndpointAddress& override;


    // IIbServiceEndpoint
    inline void SetServiceId(const mw::ServiceId& serviceId) override;
    inline auto GetServiceId() const -> const mw::ServiceId & override;

private:
    // ----------------------------------------
    // private methods
    template <class MsgT>
    inline void SendIbMessage(MsgT&& msg) const;

    inline void SendParticipantCommand(ParticipantId participantId, ParticipantCommand::Kind kind) const;
    inline void SendSystemCommand(SystemCommand::Kind kind) const;
    
private:
    // ----------------------------------------
    // private members
    IComAdapterInternal* _comAdapter{nullptr};
    mw::ServiceId _serviceId{};
};

// ================================================================================
// Inline Implementations
// ================================================================================
template <class MsgT>
void SystemController::SendIbMessage(MsgT&& msg) const
{
    assert(_comAdapter);
    _comAdapter->SendIbMessage(this, std::forward<MsgT>(msg));
}

void SystemController::SendParticipantCommand(ParticipantId participantId, ParticipantCommand::Kind kind) const
{
    ParticipantCommand cmd;
    cmd.participant = participantId;
    cmd.kind = kind;

    SendIbMessage(std::move(cmd));
}
    
void SystemController::SendSystemCommand(SystemCommand::Kind kind) const
{
    SystemCommand cmd;
    cmd.kind = kind;

    SendIbMessage(std::move(cmd));
}

// ================================================================================
//  Inline Implementations
// ================================================================================

void SystemController::SetServiceId(const mw::ServiceId& serviceId)
{
    _serviceId = serviceId;
}

auto SystemController::GetServiceId() const -> const mw::ServiceId&
{
    return _serviceId;
}

} // namespace sync
} // namespace mw
} // namespace ib
