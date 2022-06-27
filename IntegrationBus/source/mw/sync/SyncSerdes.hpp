
// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once


#include "SyncDatatypes.hpp"
#include "MessageBuffer.hpp"

namespace ib {
namespace mw {
namespace sync {
void Serialize(ib::mw::MessageBuffer& buffer, const ParticipantCommand& msg);
void Serialize(ib::mw::MessageBuffer& buffer, const SystemCommand& msg);
void Serialize(ib::mw::MessageBuffer& buffer, const ParticipantStatus& msg);
void Serialize(ib::mw::MessageBuffer& buffer, const WorkflowConfiguration& msg);
void Serialize(ib::mw::MessageBuffer& buffer, const NextSimTask& msg);

void Deserialize(ib::mw::MessageBuffer& buffer, ParticipantCommand& out);
void Deserialize(ib::mw::MessageBuffer& buffer, SystemCommand& out);
void Deserialize(ib::mw::MessageBuffer& buffer, ParticipantStatus& out);
void Deserialize(ib::mw::MessageBuffer& buffer, WorkflowConfiguration& out);
void Deserialize(ib::mw::MessageBuffer& buffer, NextSimTask& out);
} // namespace sync    
} // namespace mw
} // namespace ib
