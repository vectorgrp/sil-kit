// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/services/orchestration/OrchestrationDatatypes.hpp"

#include "core/internal/IReceiver.hpp"
#include "core/internal/ISender.hpp"
#include "core/internal/string_utils_sync.hpp"

namespace SilKit {
namespace Services {
namespace Orchestration {

class IMsgForSystemController
    : public Core::IReceiver<>
    , public Core::ISender<SystemCommand, WorkflowConfiguration>
{
};

} // namespace Orchestration
} // namespace Services
} // namespace SilKit
