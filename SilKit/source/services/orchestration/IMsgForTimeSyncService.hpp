// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "core/internal/OrchestrationDatatypes.hpp"
#include "core/internal/IReceiver.hpp"
#include "core/internal/ISender.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

class IMsgForTimeSyncService
    : public Core::IReceiver<NextSimTask>
    , public Core::ISender<ParticipantStatus, NextSimTask>
{
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
