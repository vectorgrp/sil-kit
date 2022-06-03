// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/mw/sync/SyncDatatypes.hpp"

#include <string>
#include <iomanip> //std::put_time
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

inline std::string to_string(const ParticipantCommand& command);
inline std::string to_string(const SystemCommand& command);
inline std::string to_string(const ParticipantStatus& status);
inline std::string to_string(const ExpectedParticipants& participantNames);

inline std::ostream& operator<<(std::ostream& out, ParticipantState state);
inline std::ostream& operator<<(std::ostream& out, SystemState state);
inline std::ostream& operator<<(std::ostream& out, ParticipantCommand::Kind command);
inline std::ostream& operator<<(std::ostream& out, SystemCommand::Kind command);

inline std::ostream& operator<<(std::ostream& out, const ParticipantCommand& command);
inline std::ostream& operator<<(std::ostream& out, const SystemCommand& command);
inline std::ostream& operator<<(std::ostream& out, const ParticipantStatus& status);
inline std::ostream& operator<<(std::ostream& out, const ExpectedParticipants& participantNames);

// ================================================================================
//  Inline Implementations
// ================================================================================
std::string to_string(ParticipantState state)
{
    switch (state)
    {
    case ParticipantState::Invalid:
        return "Invalid";
    case ParticipantState::ControllersCreated:
        return "ControllersCreated";
    case ParticipantState::CommunicationReady:
        return "CommunicationReady";
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
    case ParticipantState::Reinitializing:
        return "Reinitializing";
    }
    throw ib::TypeConversionError{};
}
    
std::string to_string(SystemState state)
{
    switch (state)
    {
    case SystemState::Invalid:
        return "Invalid";
    case SystemState::ControllersCreated:
        return "ControllersCreated";
    case SystemState::CommunicationReady:
        return "CommunicationReady";
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
    case SystemState::Reinitializing:
        return "Reinitializing";
    }
    throw ib::TypeConversionError{};
}

std::string to_string(ParticipantCommand::Kind command)
{
    switch (command)
    {
    case ParticipantCommand::Kind::Invalid:
        return "Invalid";
    case ParticipantCommand::Kind::Initialize: 
        return "Initialize";
    case ParticipantCommand::Kind::Reinitialize:
        return "Reinitialize";
    case ParticipantCommand::Kind::Shutdown:
        return "Shutdown";
    }
    throw ib::TypeConversionError{};
}
    
std::string to_string(SystemCommand::Kind command)
{
    switch (command)
    {
    case SystemCommand::Kind::Invalid:
        return "Invalid";
    case SystemCommand::Kind::Run:
        return "Run";
    case SystemCommand::Kind::Stop:
        return "Stop";
    case SystemCommand::Kind::Shutdown:
        return "Shutdown";
    case SystemCommand::Kind::PrepareColdswap:
        return "PrepareColdswap";
    case SystemCommand::Kind::ExecuteColdswap:
        return "ExecuteColdswap";
    case SystemCommand::Kind::AbortSimulation:
        return "AbortSimulation";
    case SystemCommand::Kind::EstablishCommunication:
        return "EstablishCommunication";
    case SystemCommand::Kind::CommunicationReady:
        return "CommunicationReady";
    }
    throw ib::TypeConversionError{};
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
    catch (const ib::TypeConversionError&)
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
    catch (const ib::TypeConversionError&)
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
    catch (const ib::TypeConversionError&)
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
    catch (const ib::TypeConversionError&)
    {
        return out << "SystemCommand::Kind{" << static_cast<uint32_t>(command) << "}";
    }
}

std::ostream& operator<<(std::ostream& out, const ParticipantCommand& command)
{
    out << "sync::ParticipantCommand{" << command.kind << "}";
    return out;
}

std::ostream& operator<<(std::ostream& out, const SystemCommand& command)
{
    out << "sync::SystemCommand{" << command.kind << "}";
    return out;
}

std::ostream& operator<<(std::ostream& out, const ParticipantStatus& status)
{
    std::time_t enterTime = std::chrono::system_clock::to_time_t(status.enterTime);
    std::tm tmBuffer;
#if defined(_WIN32)
    localtime_s(&tmBuffer, &enterTime);
#else
    localtime_r(&enterTime, &tmBuffer);
#endif

    out << "sync::ParticipantStatus{" << status.participantName
        << ", State=" << status.state
        << ", Reason=" << status.enterReason
        << ", Time=" << std::put_time(&tmBuffer, "%FT%T")
        << "}";

    return out;
}

std::string to_string(const ExpectedParticipants& participants)
{
    std::stringstream outStream;
    outStream << participants;
    return outStream.str();
}

std::ostream& operator<<(std::ostream& out, const ExpectedParticipants& participants)
{
    out << "sync::ExpectedParticipants{";
    for (auto&& p : participants.names)
    {
        out << p << ", ";
    }
    out << "}";

    return out;
}

} // namespace sync
} // namespace mw
} // namespace ib
