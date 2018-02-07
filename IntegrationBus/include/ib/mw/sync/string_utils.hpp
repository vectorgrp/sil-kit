// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include "SyncDatatypes.hpp"

#include <string>
#include <ostream>

#include "ib/exception.hpp"

namespace ib {
namespace mw {
namespace sync {

inline std::string to_string(ParticipantState state);
inline std::string to_string(SystemState state);
inline std::string to_string(ParticipantCommand::Kind command);
inline std::string to_string(SystemCommand::Kind command);
inline std::string to_string(QuantumRequestStatus status);

inline std::ostream& operator<<(std::ostream& out, ParticipantState state);
inline std::ostream& operator<<(std::ostream& out, SystemState state);
inline std::ostream& operator<<(std::ostream& out, ParticipantCommand::Kind command);
inline std::ostream& operator<<(std::ostream& out, SystemCommand::Kind command);

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
    case ParticipantState::Stopped:
        return "Stopped";
    case ParticipantState::Error:
        return "Error";
    case ParticipantState::Shutdown:
        return "Shutdown";
    default:
        throw ib::type_conversion_error{};
    }
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
    case SystemState::Error:
        return "Error";
    case SystemState::ShuttingDown:
        return "ShuttingDown";
    case SystemState::Shutdown:
        return "Shutdown";
    default:
        throw ib::type_conversion_error{};
    }
}

std::string to_string(ParticipantCommand::Kind command)
{
    switch (command)
    {
    case ParticipantCommand::Kind::Initialize:
        return "Initialize";
    case ParticipantCommand::Kind::ReInitialize:
        return "ReInitialize";
    default:
        throw ib::type_conversion_error{};
    }
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
    default:
        throw ib::type_conversion_error{};
    }
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
    default:
        throw ib::type_conversion_error{};
    }
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

inline std::ostream& operator<<(std::ostream& out, ParticipantCommand::Kind command)
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

inline std::ostream& operator<<(std::ostream& out, SystemCommand::Kind command)
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

} // namespace sync
} // namespace mw
} // namespace ib
