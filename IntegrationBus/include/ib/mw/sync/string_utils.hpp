// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "SyncDatatypes.hpp"

#include <string>
#include <ostream>
#include <sstream>
#include <ctime>

#include "ib/exception.hpp"

namespace ib {
namespace mw {
namespace sync {

inline std::string to_string(ParticipantState state);
inline std::string to_string(SystemState state);
inline std::string to_string(ParticipantCommand::Kind command);
inline std::string to_string(SystemCommand::Kind command);
inline std::string to_string(QuantumRequestStatus status);

inline std::string to_string(const QuantumRequest& request);
inline std::string to_string(const QuantumGrant& grant);
inline std::string to_string(const Tick& tick);
inline std::string to_string(const TickDone& tickDone);
inline std::string to_string(const ParticipantCommand& command);
inline std::string to_string(const SystemCommand& command);
inline std::string to_string(const ParticipantStatus& status);

inline std::ostream& operator<<(std::ostream& out, ParticipantState state);
inline std::ostream& operator<<(std::ostream& out, SystemState state);
inline std::ostream& operator<<(std::ostream& out, ParticipantCommand::Kind command);
inline std::ostream& operator<<(std::ostream& out, SystemCommand::Kind command);
inline std::ostream& operator<<(std::ostream& out, QuantumRequestStatus status);

inline std::ostream& operator<<(std::ostream& out, const QuantumRequest& request);
inline std::ostream& operator<<(std::ostream& out, const QuantumGrant& grant);
inline std::ostream& operator<<(std::ostream& out, const Tick& tick);
inline std::ostream& operator<<(std::ostream& out, const TickDone& tickDone);
inline std::ostream& operator<<(std::ostream& out, const ParticipantCommand& command);
inline std::ostream& operator<<(std::ostream& out, const SystemCommand& command);
inline std::ostream& operator<<(std::ostream& out, const ParticipantStatus& status);

// ================================================================================
//  Inline Implementations
// ================================================================================
std::string to_string(ParticipantState state)
{
    switch (state)
    {
    case ParticipantState::Invalid:
        return "Invalid";
    case ParticipantState::Idle:
        return "Idle";
    case ParticipantState::Initializing:
        return "Initializing";
    case ParticipantState::Initialized:
        return "Initialized";
    case ParticipantState::Running:
        return "Running";
    case ParticipantState::Paused:
        return "Paused";
    case ParticipantState::Stopping:
        return "Stopping";
    case ParticipantState::Stopped:
        return "Stopped";
    case ParticipantState::ColdswapPrepare:
        return "ColdswapPrepare";
    case ParticipantState::ColdswapReady:
        return "ColdswapReady";
    case ParticipantState::ColdswapShutdown:
        return "ColdswapShutdown";
    case ParticipantState::ColdswapIgnored:
        return "ColdswapIgnored";
    case ParticipantState::Error:
        return "Error";
    case ParticipantState::ShuttingDown:
        return "ShuttingDown";
    case ParticipantState::Shutdown:
        return "Shutdown";
    }
    throw ib::type_conversion_error{};
}
    
std::string to_string(SystemState state)
{
    switch (state)
    {
    case SystemState::Invalid:
        return "Invalid";
    case SystemState::Idle:
        return "Idle";
    case SystemState::Initializing:
        return "Initializing";
    case SystemState::Initialized:
        return "Initialized";
    case SystemState::Running:
        return "Running";
    case SystemState::Paused:
        return "Paused";
    case SystemState::Stopping:
        return "Stopping";
    case SystemState::Stopped:
        return "Stopped";
    case SystemState::ColdswapPrepare:
        return "ColdswapPrepare";
    case SystemState::ColdswapReady:
        return "ColdswapReady";
    case SystemState::ColdswapPending:
        return "ColdswapPending";
    case SystemState::ColdswapDone:
        return "ColdswapDone";
    case SystemState::Error:
        return "Error";
    case SystemState::ShuttingDown:
        return "ShuttingDown";
    case SystemState::Shutdown:
        return "Shutdown";
    }
    throw ib::type_conversion_error{};
}

std::string to_string(ParticipantCommand::Kind command)
{
    switch (command)
    {
    case ParticipantCommand::Kind::Initialize:
        return "Initialize";
    case ParticipantCommand::Kind::ReInitialize:
        return "ReInitialize";
    }
    throw ib::type_conversion_error{};
}
    
std::string to_string(SystemCommand::Kind command)
{
    switch (command)
    {
    case SystemCommand::Kind::Run:
        return "Run";
    case SystemCommand::Kind::Stop:
        return "Stop";
    case SystemCommand::Kind::Shutdown:
        return "Shutdown";
    }
    throw ib::type_conversion_error{};
}

inline std::string to_string(QuantumRequestStatus status)
{
    switch (status)
    {
    case QuantumRequestStatus::Granted:
        return "Granted";
    case QuantumRequestStatus::Rejected:
        return "Rejected";
    case QuantumRequestStatus::Invalid:
        return "Invalid";
    }
    throw ib::type_conversion_error{};
}


std::string to_string(const QuantumRequest& request)
{
    std::stringstream outStream;
    outStream << request;
    return outStream.str();
}

std::string to_string(const QuantumGrant& grant)
{
    std::stringstream outStream;
    outStream << grant;
    return outStream.str();
}

std::string to_string(const Tick& tick)
{
    std::stringstream outStream;
    outStream << tick;
    return outStream.str();
}

std::string to_string(const TickDone& tickDone)
{
    std::stringstream outStream;
    outStream << tickDone;
    return outStream.str();
}

std::string to_string(const ParticipantCommand& command)
{
    std::stringstream outStream;
    outStream << command;
    return outStream.str();
}

std::string to_string(const SystemCommand& command)
{
    std::stringstream outStream;
    outStream << command;
    return outStream.str();
}

std::string to_string(const ParticipantStatus& status)
{
    std::stringstream outStream;
    outStream << status;
    return outStream.str();
}

std::ostream& operator<<(std::ostream& out, ParticipantState state)
{
    try
    {
        return out << to_string(state);
    }
    catch (const ib::type_conversion_error&)
    {
        return out << "ParticipantState{" << static_cast<uint32_t>(state) << "}";
    }
}

std::ostream& operator<<(std::ostream& out, SystemState state)
{
    try
    {
        return out << to_string(state);
    }
    catch (const ib::type_conversion_error&)
    {
        return out << "SystemState{" << static_cast<uint32_t>(state) << "}";
    }
}

std::ostream& operator<<(std::ostream& out, ParticipantCommand::Kind command)
{
    try
    {
        return out << to_string(command);
    }
    catch (const ib::type_conversion_error&)
    {
        return out << "ParticipantCommand::Kind{" << static_cast<uint32_t>(command) << "}";
    }
}

std::ostream& operator<<(std::ostream& out, SystemCommand::Kind command)
{
    try
    {
        return out << to_string(command);
    }
    catch (const ib::type_conversion_error&)
    {
        return out << "SystemCommand::Kind{" << static_cast<uint32_t>(command) << "}";
    }
}

std::ostream& operator<<(std::ostream& out, QuantumRequestStatus status)
{
    try
    {
        return out << to_string(status);
    }
    catch (const ib::type_conversion_error&)
    {
        return out << "QuantumRequestStatus{" << static_cast<uint32_t>(status) << "}";
    }
}

std::ostream& operator<<(std::ostream& out, const QuantumRequest& request)
{
    auto now = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(request.now);
    auto duration = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(request.duration);

    out << "QuantumRequest{now=" << now.count()
        << "ms, duration=" << duration.count()
        << "ms}";
    return out;
}

std::ostream& operator<<(std::ostream& out, const QuantumGrant& grant)
{
    auto now = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(grant.now);
    auto duration = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(grant.duration);

    out << "QuantumGrant{grantee={" << grant.grantee.participant << "," << grant.grantee.endpoint
        << "}, status=" << grant.status
        << "ms, duration=" << duration.count()
        << "ms}";
    return out;
}

std::ostream& operator<<(std::ostream& out, const Tick& tick)
{
    auto now = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(tick.now);
    out << "Tick{now=" << now.count() << "ms}";
    return out;
}

std::ostream& operator<<(std::ostream& out, const TickDone&)
{
    out << "TickDone{}";
    return out;
}

std::ostream& operator<<(std::ostream& out, const ParticipantCommand& command)
{
    out << "ParticipantCommand{" << command.kind << "}";
    return out;
}

std::ostream& operator<<(std::ostream& out, const SystemCommand& command)
{
    out << "SystemCommand{" << command.kind << "}";
    return out;
}

std::ostream& operator<<(std::ostream& out, const ParticipantStatus& status)
{
    std::time_t enterTime = std::chrono::system_clock::to_time_t(status.enterTime);
    std::tm tmBuffer;
#if defined(_MSC_VER)
    localtime_s(&tmBuffer, &enterTime);
#else
    localtime_r(&enterTime, &tmBuffer);
#endif

    char timeString[32];
    std::strftime(timeString, sizeof(timeString), "%FT%T", &tmBuffer);

    out << "ParticipantStatus{" << status.participantName
        << ", State=" << status.state
        << ", Reason=" << status.enterReason
        << ", Time=" << timeString
        << "}";

    return out;
}

} // namespace sync
} // namespace mw
} // namespace ib
