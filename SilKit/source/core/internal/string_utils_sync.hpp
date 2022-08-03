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
    out << "Orchestration::NextSimTask{tp=" << tp.count()
        << "ms, duration=" << duration.count()
        << "ms}";
    return out;
}

std::string to_string(SystemCommand::Kind command)
{
    switch (command)
    {
    case SystemCommand::Kind::Invalid: return "Invalid";
    case SystemCommand::Kind::AbortSimulation: return "AbortSimulation";
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
