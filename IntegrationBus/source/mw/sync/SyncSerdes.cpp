// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "InternalSerdes.hpp"
#include "SyncSerdes.hpp"

namespace ib {
namespace mw {
namespace sync {

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const ib::mw::sync::NextSimTask& task)
{
    buffer << task.timePoint
           << task.duration;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, ib::mw::sync::NextSimTask& task)
{
    buffer >> task.timePoint
           >> task.duration;
    return buffer;
}

    
inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const ib::mw::sync::ParticipantCommand& cmd)
{
    buffer << cmd.participant
           << cmd.kind;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, ib::mw::sync::ParticipantCommand& cmd)
{
    buffer >> cmd.participant
           >> cmd.kind;
    return buffer;
}

    
inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const ib::mw::sync::SystemCommand& cmd)
{
    buffer << cmd.kind;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, ib::mw::sync::SystemCommand& cmd)
{
    buffer >> cmd.kind;
    return buffer;
}
    

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer, const ib::mw::sync::ParticipantStatus& status)
{
    buffer << status.participantName
           << status.state
           << status.enterReason
           << status.enterTime
           << status.refreshTime;
    return buffer;
}
inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer, ib::mw::sync::ParticipantStatus& status)
{
    buffer >> status.participantName
           >> status.state
           >> status.enterReason
           >> status.enterTime
           >> status.refreshTime;
    return buffer;
}

inline ib::mw::MessageBuffer& operator<<(ib::mw::MessageBuffer& buffer,
                                         const ib::mw::sync::WorkflowConfiguration& workflowConfiguration)
{
    buffer << workflowConfiguration.requiredParticipantNames;
    return buffer;
}

inline ib::mw::MessageBuffer& operator>>(ib::mw::MessageBuffer& buffer,
                                         ib::mw::sync::WorkflowConfiguration& workflowConfiguration)
{
    buffer >> workflowConfiguration.requiredParticipantNames;
    return buffer;
}

void Serialize(ib::mw::MessageBuffer& buffer, const ParticipantCommand& msg)
{
    buffer << msg;
    return;
}
void Serialize(ib::mw::MessageBuffer& buffer, const SystemCommand& msg)
{
    buffer << msg;
    return;
}
void Serialize(ib::mw::MessageBuffer& buffer, const ParticipantStatus& msg)
{
    buffer << msg;
    return;
}
void Serialize(ib::mw::MessageBuffer& buffer, const WorkflowConfiguration& msg)
{
    buffer << msg;
    return;
}
void Serialize(ib::mw::MessageBuffer& buffer, const NextSimTask& msg)
{
    buffer << msg;
    return;
}

void Deserialize(ib::mw::MessageBuffer& buffer, ParticipantCommand& out)
{
    buffer >> out;
}
void Deserialize(ib::mw::MessageBuffer& buffer, SystemCommand& out)
{
    buffer >> out;
}
void Deserialize(ib::mw::MessageBuffer& buffer, ParticipantStatus& out)
{
    buffer >> out;
}
void Deserialize(ib::mw::MessageBuffer& buffer, WorkflowConfiguration& out)
{
    buffer >> out;
}
void Deserialize(ib::mw::MessageBuffer& buffer, NextSimTask& out)
{
    buffer >> out;
}

} // namespace sync    
} // namespace mw
} // namespace ib
