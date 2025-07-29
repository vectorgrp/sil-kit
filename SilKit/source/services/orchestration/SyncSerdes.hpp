// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "OrchestrationDatatypes.hpp"
#include "MessageBuffer.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

void Serialize(SilKit::Core::MessageBuffer& buffer, const SystemCommand& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const ParticipantStatus& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const WorkflowConfiguration& msg);
void Serialize(SilKit::Core::MessageBuffer& buffer, const NextSimTask& msg);

void Deserialize(SilKit::Core::MessageBuffer& buffer, SystemCommand& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, ParticipantStatus& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, WorkflowConfiguration& out);
void Deserialize(SilKit::Core::MessageBuffer& buffer, NextSimTask& out);

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
