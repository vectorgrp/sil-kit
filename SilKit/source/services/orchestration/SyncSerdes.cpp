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

#include "InternalSerdes.hpp"
#include "SyncSerdes.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const SilKit::Services::Orchestration::NextSimTask& task)
{
    buffer << task.timePoint
           << task.duration;
    return buffer;
}
inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, SilKit::Services::Orchestration::NextSimTask& task)
{
    buffer >> task.timePoint
           >> task.duration;
    return buffer;
}

    
inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const SilKit::Services::Orchestration::SystemCommand& cmd)
{
    buffer << cmd.kind;
    return buffer;
}
inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, SilKit::Services::Orchestration::SystemCommand& cmd)
{
    buffer >> cmd.kind;
    return buffer;
}


inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const SilKit::Services::Orchestration::ParticipantStatus& status)
{
    buffer << status.participantName
           << status.state
           << status.enterReason
           << status.enterTime
           << status.refreshTime;
    return buffer;
}
inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, SilKit::Services::Orchestration::ParticipantStatus& status)
{
    buffer >> status.participantName
           >> status.state
           >> status.enterReason
           >> status.enterTime
           >> status.refreshTime;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer,
                                         const SilKit::Services::Orchestration::WorkflowConfiguration& workflowConfiguration)
{
    buffer << workflowConfiguration.requiredParticipantNames;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer,
                                         SilKit::Services::Orchestration::WorkflowConfiguration& workflowConfiguration)
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
