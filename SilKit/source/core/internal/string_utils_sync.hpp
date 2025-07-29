// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/services/orchestration/string_utils.hpp"

#include "OrchestrationDatatypes.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

inline std::string to_string(const NextSimTask& nextTask);
inline std::string to_string(SystemCommand::Kind command);
inline std::string to_string(const SystemCommand& command);

inline std::ostream& operator<<(std::ostream& out, const NextSimTask& nextTask);
inline std::ostream& operator<<(std::ostream& out, SystemCommand::Kind command);
inline std::ostream& operator<<(std::ostream& out, const SystemCommand& command);

// ================================================================================
//  Inline Implementations
// ================================================================================

std::string to_string(const NextSimTask& nextTask)
{
    std::stringstream outStream;
    outStream << nextTask;
    return outStream.str();
}

std::ostream& operator<<(std::ostream& out, const NextSimTask& nextTask)
{
    auto tp = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(nextTask.timePoint);
    auto duration = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(nextTask.duration);
    out << "Orchestration::NextSimTask{tp=" << tp.count() << "ms, duration=" << duration.count() << "ms}";
    return out;
}

std::string to_string(SystemCommand::Kind command)
{
    switch (command)
    {
    case SystemCommand::Kind::Invalid:
        return "Invalid";
    case SystemCommand::Kind::AbortSimulation:
        return "AbortSimulation";
    }
    throw SilKit::TypeConversionError{};
}

std::string to_string(const SystemCommand& command)
{
    std::stringstream outStream;
    outStream << command;
    return outStream.str();
}

std::ostream& operator<<(std::ostream& out, SystemCommand::Kind command)
{
    try
    {
        return out << to_string(command);
    }
    catch (const SilKit::TypeConversionError&)
    {
        return out << "SystemCommand::Kind{" << static_cast<uint32_t>(command) << "}";
    }
}

std::ostream& operator<<(std::ostream& out, const SystemCommand& command)
{
    out << "Orchestration::SystemCommand{" << command.kind << "}";
    return out;
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
