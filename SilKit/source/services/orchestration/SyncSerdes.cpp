// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "InternalSerdes.hpp"
#include "SyncSerdes.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer,
                                               const SilKit::Services::Orchestration::NextSimTask& task)
{
    buffer << task.timePoint << task.duration;
    return buffer;
}
inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer,
                                               SilKit::Services::Orchestration::NextSimTask& task)
{
    buffer >> task.timePoint >> task.duration;
    return buffer;
}


inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer,
                                               const SilKit::Services::Orchestration::SystemCommand& cmd)
{
    buffer << cmd.kind;
    return buffer;
}
inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer,
                                               SilKit::Services::Orchestration::SystemCommand& cmd)
{
    buffer >> cmd.kind;
    return buffer;
}


inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer,
                                               const SilKit::Services::Orchestration::ParticipantStatus& status)
{
    buffer << status.participantName << status.state << status.enterReason << status.enterTime << status.refreshTime;
    return buffer;
}
inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer,
                                               SilKit::Services::Orchestration::ParticipantStatus& status)
{
    buffer >> status.participantName >> status.state >> status.enterReason >> status.enterTime >> status.refreshTime;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator<<(
    SilKit::Core::MessageBuffer& buffer,
    const SilKit::Services::Orchestration::WorkflowConfiguration& workflowConfiguration)
{
    buffer << workflowConfiguration.requiredParticipantNames;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(
    SilKit::Core::MessageBuffer& buffer, SilKit::Services::Orchestration::WorkflowConfiguration& workflowConfiguration)
{
    buffer >> workflowConfiguration.requiredParticipantNames;
    return buffer;
}

void Serialize(SilKit::Core::MessageBuffer& buffer, const SystemCommand& msg)
{
    buffer << msg;
    return;
}
void Serialize(SilKit::Core::MessageBuffer& buffer, const ParticipantStatus& msg)
{
    buffer << msg;
    return;
}
void Serialize(SilKit::Core::MessageBuffer& buffer, const WorkflowConfiguration& msg)
{
    buffer << msg;
    return;
}
void Serialize(SilKit::Core::MessageBuffer& buffer, const NextSimTask& msg)
{
    buffer << msg;
    return;
}

void Deserialize(SilKit::Core::MessageBuffer& buffer, SystemCommand& out)
{
    buffer >> out;
}
void Deserialize(SilKit::Core::MessageBuffer& buffer, ParticipantStatus& out)
{
    buffer >> out;
}
void Deserialize(SilKit::Core::MessageBuffer& buffer, WorkflowConfiguration& out)
{
    buffer >> out;
}
void Deserialize(SilKit::Core::MessageBuffer& buffer, NextSimTask& out)
{
    buffer >> out;
}

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
