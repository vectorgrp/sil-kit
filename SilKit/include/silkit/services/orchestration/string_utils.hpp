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

#include "silkit/services/orchestration/OrchestrationDatatypes.hpp"

#include <string>
#include <iomanip> //std::put_time
#include <ostream>
#include <sstream>
#include <ctime>

#include "silkit/participant/exception.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

inline std::string to_string(ParticipantState state);
inline std::string to_string(SystemState state);

inline std::string to_string(const ParticipantStatus& status);
inline std::string to_string(const WorkflowConfiguration& participantNames);

inline std::ostream& operator<<(std::ostream& out, ParticipantState state);
inline std::ostream& operator<<(std::ostream& out, SystemState state);

inline std::ostream& operator<<(std::ostream& out, const ParticipantStatus& status);
inline std::ostream& operator<<(std::ostream& out, const WorkflowConfiguration& workflowConfiguration);

// ================================================================================
//  Inline Implementations
// ================================================================================
std::string to_string(ParticipantState state)
{
    switch (state)
    {
    case ParticipantState::Invalid:
        return "Invalid";
    case ParticipantState::ServicesCreated:
        return "ServicesCreated";
    case ParticipantState::CommunicationInitializing:
        return "CommunicationInitializing";
    case ParticipantState::CommunicationInitialized:
        return "CommunicationInitialized";
    case ParticipantState::ReadyToRun:
        return "ReadyToRun";
    case ParticipantState::Running:
        return "Running";
    case ParticipantState::Paused:
        return "Paused";
    case ParticipantState::Stopping:
        return "Stopping";
    case ParticipantState::Stopped:
        return "Stopped";
    case ParticipantState::Error:
        return "Error";
    case ParticipantState::ShuttingDown:
        return "ShuttingDown";
    case ParticipantState::Shutdown:
        return "Shutdown";
    case ParticipantState::Aborting:
        return "Aborting";
    }
    throw SilKit::TypeConversionError{};
}
    
std::string to_string(SystemState state)
{
    switch (state)
    {
    case SystemState::Invalid:
        return "Invalid";
    case SystemState::ServicesCreated:
        return "ServicesCreated";
    case SystemState::CommunicationInitializing:
        return "CommunicationInitializing";
    case SystemState::CommunicationInitialized:
        return "CommunicationInitialized";
    case SystemState::ReadyToRun:
        return "ReadyToRun";
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
    case SystemState::Aborting:
        return "Aborting";
    }
    throw SilKit::TypeConversionError{};
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
    catch (const SilKit::TypeConversionError&)
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
    catch (const SilKit::TypeConversionError&)
    {
        return out << "SystemState{" << static_cast<uint32_t>(state) << "}";
    }
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

    out << "Orchestration::ParticipantStatus{" << status.participantName
        << ", State=" << status.state
        << ", Reason=" << status.enterReason
        << ", Time=" << std::put_time(&tmBuffer, "%FT%T")
        << "}";

    return out;
}

std::string to_string(const WorkflowConfiguration& participants)
{
    std::stringstream outStream;
    outStream << participants;
    return outStream.str();
}

std::ostream& operator<<(std::ostream& out, const WorkflowConfiguration& workflowConfiguration)
{
    out << "Orchestration::WorkflowConfiguration{requiredParticipantNames=";
    bool first = true;
    for (auto&& p : workflowConfiguration.requiredParticipantNames)
    {
        if (!first) out << ", ";
        out << p;
        first = false;
    }
    out << "}";

    return out;
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
