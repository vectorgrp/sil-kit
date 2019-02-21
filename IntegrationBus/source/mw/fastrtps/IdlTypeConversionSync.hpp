// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "idl/MiddlewareTopics.h"

#include "IdlTypeConversionMw.hpp"

namespace ib {
namespace mw {
namespace sync {

inline auto to_idl(const QuantumRequest& msg) -> idl::QuantumRequest;
inline auto to_idl(QuantumRequestStatus status) -> idl::QuantumRequestStatus;
inline auto to_idl(const QuantumGrant& msg) -> idl::QuantumGrant;
inline auto to_idl(const Tick& msg) -> idl::Tick;
inline auto to_idl(const TickDone& msg) -> idl::TickDone;

inline auto to_idl(ParticipantCommand::Kind kind) -> idl::ParticipantCommandKind;
inline auto to_idl(const ParticipantCommand& msg) -> idl::ParticipantCommand;
inline auto to_idl(SystemCommand::Kind kind) -> idl::SystemCommandKind;
inline auto to_idl(const SystemCommand& msg) -> idl::SystemCommand;
inline auto to_idl(ParticipantState state) -> idl::ParticipantState;
inline auto to_idl(const ParticipantStatus& msg) -> idl::ParticipantStatus;
inline auto to_idl(SystemState state) -> idl::SystemState;

namespace idl {

inline auto from_idl(QuantumRequest&& msg) -> sync::QuantumRequest;
inline auto from_idl(QuantumRequestStatus status) -> sync::QuantumRequestStatus;
inline auto from_idl(QuantumGrant&& msg) -> sync::QuantumGrant;
inline auto from_idl(Tick&& idl) -> sync::Tick;
inline auto from_idl(TickDone&& idl) -> sync::TickDone;

inline auto from_idl(ParticipantCommandKind kind) -> sync::ParticipantCommand::Kind;
inline auto from_idl(ParticipantCommand&& msg) -> sync::ParticipantCommand;
inline auto from_idl(SystemCommandKind kind) -> sync::SystemCommand::Kind;
inline auto from_idl(SystemCommand&& msg) -> sync::SystemCommand;
inline auto from_idl(ParticipantState state) -> sync::ParticipantState;
inline auto from_idl(ParticipantStatus&& msg) -> sync::ParticipantStatus;
inline auto from_idl(SystemState state) -> sync::SystemState;

} // namespace idl


// ================================================================================
//  Inline Implementations
// ================================================================================
auto to_idl(const Tick& msg) -> idl::Tick
{
    idl::Tick idl;
    idl.nowNs(msg.now.count());
    idl.durationNs(msg.duration.count());

    return idl;
}

auto idl::from_idl(idl::Tick&& idl) -> ::ib::mw::sync::Tick
{
    ::ib::mw::sync::Tick msg;

    msg.now = std::chrono::nanoseconds{idl.nowNs()};
    msg.duration = std::chrono::nanoseconds{idl.durationNs()};

    return msg;
}

auto to_idl(const TickDone& msg) -> idl::TickDone
{
    idl::TickDone idl;

    idl.finishedTick(to_idl(msg.finishedTick));

    return idl;
}

auto idl::from_idl(idl::TickDone&& idl) -> ::ib::mw::sync::TickDone
{
    ::ib::mw::sync::TickDone msg;

    msg.finishedTick = from_idl(std::move(idl.finishedTick()));

    return msg;
}

auto to_idl(ParticipantCommand::Kind kind) -> idl::ParticipantCommandKind
{
    switch (kind)
    {
    case ParticipantCommand::Kind::Invalid:
        return idl::ParticipantCommandKind::PC_Invalid;
    case ParticipantCommand::Kind::Initialize:
        return idl::ParticipantCommandKind::PC_Initialize;
    case ParticipantCommand::Kind::ReInitialize:
        return idl::ParticipantCommandKind::PC_ReInitialize;
    }
    return idl::ParticipantCommandKind::PC_Invalid;
}

auto idl::from_idl(idl::ParticipantCommandKind kind) -> sync::ParticipantCommand::Kind
{
    switch (kind)
    {
    case idl::ParticipantCommandKind::PC_Invalid:
        return sync::ParticipantCommand::Kind::Invalid;
    case idl::ParticipantCommandKind::PC_Initialize:
        return sync::ParticipantCommand::Kind::Initialize;
    case idl::ParticipantCommandKind::PC_ReInitialize:
        return sync::ParticipantCommand::Kind::ReInitialize;
    }
    return sync::ParticipantCommand::Kind::Invalid;
}

auto to_idl(const ParticipantCommand& msg) -> idl::ParticipantCommand
{
    idl::ParticipantCommand idl;
    idl.participant(msg.participant);
    idl.kind(to_idl(msg.kind));
    return idl;
}

auto idl::from_idl(idl::ParticipantCommand&& idl) -> sync::ParticipantCommand
{
    sync::ParticipantCommand msg;
    msg.participant = idl.participant();
    msg.kind = from_idl(idl.kind());
    return msg;
}

auto to_idl(SystemCommand::Kind kind) -> idl::SystemCommandKind
{
    switch (kind)
    {
    case SystemCommand::Kind::Invalid:
        return idl::SystemCommandKind::SC_Invalid;
    case SystemCommand::Kind::Run:
        return idl::SystemCommandKind::SC_Run;
    case SystemCommand::Kind::Stop:
        return idl::SystemCommandKind::SC_Stop;
    case SystemCommand::Kind::Shutdown:
        return idl::SystemCommandKind::SC_Shutdown;
    case SystemCommand::Kind::PrepareColdswap:
        return idl::SystemCommandKind::SC_PrepareColdswap;
    case SystemCommand::Kind::ExecuteColdswap:
        return idl::SystemCommandKind::SC_ExecuteColdswap;
    }
    return idl::SystemCommandKind::SC_Invalid;
}

auto idl::from_idl(idl::SystemCommandKind kind) -> sync::SystemCommand::Kind
{
    switch (kind)
    {
    case idl::SystemCommandKind::SC_Invalid:
        return sync::SystemCommand::Kind::Invalid;
    case idl::SystemCommandKind::SC_Run:
        return sync::SystemCommand::Kind::Run;
    case idl::SystemCommandKind::SC_Stop:
        return sync::SystemCommand::Kind::Stop;
    case idl::SystemCommandKind::SC_Shutdown:
        return sync::SystemCommand::Kind::Shutdown;
    case idl::SystemCommandKind::SC_PrepareColdswap:
        return sync::SystemCommand::Kind::PrepareColdswap;
    case idl::SystemCommandKind::SC_ExecuteColdswap:
        return sync::SystemCommand::Kind::ExecuteColdswap;
    }
    return sync::SystemCommand::Kind::Invalid;
}

auto to_idl(const SystemCommand& msg) -> idl::SystemCommand
{
    idl::SystemCommand idl;
    idl.kind(to_idl(msg.kind));
    return idl;
}

auto idl::from_idl(idl::SystemCommand&& idl) -> sync::SystemCommand
{
    sync::SystemCommand msg;
    msg.kind = from_idl(idl.kind());
    return msg;
}

auto to_idl(ParticipantState state) -> idl::ParticipantState
{
    switch (state)
    {
    case ParticipantState::Invalid:
        return idl::ParticipantState::PS_Invalid;
    case ParticipantState::Idle:
        return idl::ParticipantState::PS_Idle;
    case ParticipantState::Initializing:
        return idl::ParticipantState::PS_Initializing;
    case ParticipantState::Initialized:
        return idl::ParticipantState::PS_Initialized;
    case ParticipantState::Running:
        return idl::ParticipantState::PS_Running;
    case ParticipantState::Paused:
        return idl::ParticipantState::PS_Paused;
    case ParticipantState::Stopping:
        return idl::ParticipantState::PS_Stopping;
    case ParticipantState::Stopped:
        return idl::ParticipantState::PS_Stopped;
    case ParticipantState::ColdswapPrepare:
        return idl::ParticipantState::PS_ColdswapPrepare;
    case ParticipantState::ColdswapReady:
        return idl::ParticipantState::PS_ColdswapReady;
    case ParticipantState::ColdswapShutdown:
        return idl::ParticipantState::PS_ColdswapShutdown;
    case ParticipantState::ColdswapIgnored:
        return idl::ParticipantState::PS_ColdswapIgnored;
    case ParticipantState::Error:
        return idl::ParticipantState::PS_Error;
    case ParticipantState::Shutdown:
        return idl::ParticipantState::PS_Shutdown;
    case ParticipantState::ShuttingDown:
        return idl::ParticipantState::PS_ShuttingDown;
    }
    return idl::ParticipantState::PS_Invalid;
}

auto idl::from_idl(idl::ParticipantState state) -> sync::ParticipantState
{
    switch (state)
    {
    case idl::ParticipantState::PS_Invalid:
        return sync::ParticipantState::Invalid;
    case idl::ParticipantState::PS_Idle:
        return sync::ParticipantState::Idle;
    case idl::ParticipantState::PS_Initializing:
        return sync::ParticipantState::Initializing;
    case idl::ParticipantState::PS_Initialized:
        return sync::ParticipantState::Initialized;
    case idl::ParticipantState::PS_Running:
        return sync::ParticipantState::Running;
    case idl::ParticipantState::PS_Paused:
        return sync::ParticipantState::Paused;
    case idl::ParticipantState::PS_Stopping:
        return sync::ParticipantState::Stopping;
    case idl::ParticipantState::PS_Stopped:
        return sync::ParticipantState::Stopped;
    case idl::ParticipantState::PS_ColdswapPrepare:
        return sync::ParticipantState::ColdswapPrepare;
    case idl::ParticipantState::PS_ColdswapReady:
        return sync::ParticipantState::ColdswapReady;
    case idl::ParticipantState::PS_ColdswapShutdown:
        return sync::ParticipantState::ColdswapShutdown;
    case idl::ParticipantState::PS_ColdswapIgnored:
        return sync::ParticipantState::ColdswapIgnored;
    case idl::ParticipantState::PS_Error:
        return sync::ParticipantState::Error;
    case idl::ParticipantState::PS_ShuttingDown:
        return sync::ParticipantState::ShuttingDown;
    case idl::ParticipantState::PS_Shutdown:
        return sync::ParticipantState::Shutdown;
    }
    return sync::ParticipantState::Invalid;
}

auto to_idl(const ParticipantStatus& msg) -> idl::ParticipantStatus
{
    idl::ParticipantStatus idl;
    idl.participantName(msg.participantName);
    idl.state(to_idl(msg.state));
    idl.enterReason(msg.enterReason);
    idl.enterTimeUs(std::chrono::duration_cast<std::chrono::microseconds>(msg.enterTime.time_since_epoch()).count());
    idl.refreshTimeUs(std::chrono::duration_cast<std::chrono::microseconds>(msg.refreshTime.time_since_epoch()).count());
    return idl;
}

auto idl::from_idl(idl::ParticipantStatus&& idl) -> sync::ParticipantStatus
{
    sync::ParticipantStatus msg;

    msg.participantName = std::move(idl.participantName());
    msg.state = from_idl(idl.state());
    msg.enterReason = std::move(idl.enterReason());
    msg.enterTime = std::chrono::system_clock::time_point(std::chrono::microseconds{idl.enterTimeUs()});
    msg.refreshTime = std::chrono::system_clock::time_point(std::chrono::microseconds{idl.refreshTimeUs()});

    return msg;
}

auto to_idl(SystemState state) -> idl::SystemState
{
    switch (state)
    {
    case SystemState::Invalid:
        return idl::SystemState::SS_Invalid;
    case SystemState::Idle:
        return idl::SystemState::SS_Idle;
    case SystemState::Initializing:
        return idl::SystemState::SS_Initializing;
    case SystemState::Initialized:
        return idl::SystemState::SS_Initialized;
    case SystemState::Running:
        return idl::SystemState::SS_Running;
    case SystemState::Paused:
        return idl::SystemState::SS_Paused;
    case SystemState::Stopping:
        return idl::SystemState::SS_Stopping;
    case SystemState::Stopped:
        return idl::SystemState::SS_Stopped;
    case SystemState::ColdswapPrepare:
        return idl::SystemState::SS_ColdswapPrepare;
    case SystemState::ColdswapReady:
        return idl::SystemState::SS_ColdswapReady;
    case SystemState::ColdswapPending:
        return idl::SystemState::SS_ColdswapPending;
    case SystemState::ColdswapDone:
        return idl::SystemState::SS_ColdswapDone;
    case SystemState::Error:
        return idl::SystemState::SS_Error;
    case SystemState::ShuttingDown:
        return idl::SystemState::SS_ShuttingDown;
    case SystemState::Shutdown:
        return idl::SystemState::SS_Shutdown;
    }
    return idl::SystemState::SS_Invalid;
}

auto idl::from_idl(idl::SystemState state) -> sync::SystemState
{
    switch (state)
    {
    case idl::SystemState::SS_Invalid:
        return sync::SystemState::Invalid;
    case idl::SystemState::SS_Idle:
        return sync::SystemState::Idle;
    case idl::SystemState::SS_Initializing:
        return sync::SystemState::Initializing;
    case idl::SystemState::SS_Initialized:
        return sync::SystemState::Initialized;
    case idl::SystemState::SS_Running:
        return sync::SystemState::Running;
    case idl::SystemState::SS_Paused:
        return sync::SystemState::Paused;
    case idl::SystemState::SS_Stopping:
        return sync::SystemState::Stopping;
    case idl::SystemState::SS_Stopped:
        return sync::SystemState::Stopped;
    case idl::SystemState::SS_ColdswapPrepare:
        return sync::SystemState::ColdswapPrepare;
    case idl::SystemState::SS_ColdswapReady:
        return sync::SystemState::ColdswapReady;
    case idl::SystemState::SS_ColdswapPending:
        return sync::SystemState::ColdswapPending;
    case idl::SystemState::SS_ColdswapDone:
        return sync::SystemState::ColdswapDone;
    case idl::SystemState::SS_Error:
        return sync::SystemState::Error;
    case idl::SystemState::SS_ShuttingDown:
        return sync::SystemState::ShuttingDown;
    case idl::SystemState::SS_Shutdown:
        return sync::SystemState::Shutdown;
    }
    return sync::SystemState::Invalid;
}

auto to_idl(const QuantumRequest& msg) -> idl::QuantumRequest
{
    idl::QuantumRequest idl;
    idl.nowNs(std::chrono::duration_cast<std::chrono::nanoseconds>(msg.now).count());
    idl.durationNs(std::chrono::duration_cast<std::chrono::nanoseconds>(msg.duration).count());
    return idl;
}

auto idl::from_idl(idl::QuantumRequest&& idl) -> sync::QuantumRequest
{
    sync::QuantumRequest msg;
    msg.now = std::chrono::nanoseconds{idl.nowNs()};
    msg.duration = std::chrono::nanoseconds{idl.durationNs()};
    return msg;
}

auto to_idl(QuantumRequestStatus status) -> idl::QuantumRequestStatus
{
    switch (status)
    {
    case QuantumRequestStatus::Invalid:
        return idl::QuantumRequestStatus::QR_Invalid;
    case QuantumRequestStatus::Granted:
        return idl::QuantumRequestStatus::QR_Granted;
    case QuantumRequestStatus::Rejected:
        return idl::QuantumRequestStatus::QR_Rejected;
    }
    return idl::QuantumRequestStatus::QR_Invalid;
}

auto idl::from_idl(idl::QuantumRequestStatus status) -> sync::QuantumRequestStatus
{
    switch (status)
    {
    case idl::QuantumRequestStatus::QR_Invalid:
        return sync::QuantumRequestStatus::Invalid;
    case idl::QuantumRequestStatus::QR_Granted:
        return sync::QuantumRequestStatus::Granted;
    case idl::QuantumRequestStatus::QR_Rejected:
        return sync::QuantumRequestStatus::Rejected;
    }
    return sync::QuantumRequestStatus::Invalid;
}

auto to_idl(const QuantumGrant& msg) -> idl::QuantumGrant
{
    idl::QuantumGrant idl;
    idl.grantee(to_idl(msg.grantee));
    idl.nowNs(std::chrono::duration_cast<std::chrono::nanoseconds>(msg.now).count());
    idl.durationNs(std::chrono::duration_cast<std::chrono::nanoseconds>(msg.duration).count());
    idl.status(to_idl(msg.status));
    return idl;
}

auto idl::from_idl(idl::QuantumGrant&& idl) -> sync::QuantumGrant
{
    sync::QuantumGrant msg;
    msg.grantee = from_idl(idl.grantee());
    msg.now = std::chrono::nanoseconds{idl.nowNs()};
    msg.duration = std::chrono::nanoseconds{idl.durationNs()};
    msg.status = from_idl(idl.status());
    return msg;
}


} // namespace sync
} // namespace mw
} // namespace ib
