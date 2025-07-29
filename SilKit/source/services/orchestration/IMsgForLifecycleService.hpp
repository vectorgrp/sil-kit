// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "OrchestrationDatatypes.hpp"
#include "IReceiver.hpp"
#include "ISender.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

class IMsgForLifecycleService
    : public Core::IReceiver<SystemCommand>
    , public Core::ISender<ParticipantStatus>
{
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
