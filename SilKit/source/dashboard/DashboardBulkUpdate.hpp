// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "silkit/services/orchestration/OrchestrationDatatypes.hpp"

#include "SilKitEvent.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace SilKit {
namespace Dashboard {

struct DashboardBulkUpdate
{
    using ParticipantConnectionInformation = SilKit::Services::Orchestration::ParticipantConnectionInformation;
    using SystemState = SilKit::Services::Orchestration::SystemState;
    using ParticipantStatus = SilKit::Services::Orchestration::ParticipantStatus;

    std::unique_ptr<uint64_t> stopped;
    std::vector<SystemState> systemStates;
    std::vector<ParticipantConnectionInformation> participantConnectionInformations;
    std::vector<ParticipantStatus> participantStatuses;
    std::vector<ServiceData> serviceDatas;

    auto Empty() const -> bool
    {
        return !stopped && systemStates.empty() && participantConnectionInformations.empty()
               && participantStatuses.empty() && serviceDatas.empty();
    }

    void Clear()
    {
        stopped.reset();
        participantConnectionInformations.clear();
        systemStates.clear();
        participantStatuses.clear();
        serviceDatas.clear();
    }
};

} // namespace Dashboard
} // namespace SilKit