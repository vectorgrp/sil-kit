// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once


#include "silkit/services/orchestration/OrchestrationDatatypes.hpp"
#include "silkit/services/logging/ILogger.hpp"
#include "silkit/util/Span.hpp"

#include <unordered_map>
#include <unordered_set>
#include <mutex>


namespace VSilKit {


class SystemStateTracker
{
    using ParticipantState = SilKit::Services::Orchestration::ParticipantState;
    using ParticipantStatus = SilKit::Services::Orchestration::ParticipantStatus;

    using SystemState = SilKit::Services::Orchestration::SystemState;

public:
    struct UpdateRequiredParticipantsResult
    {
        bool systemStateChanged{false};
    };

    struct UpdateParticipantStatusResult
    {
        bool participantStateChanged{false};
        bool systemStateChanged{false};
    };

    struct RemoveParticipantResult
    {
        bool systemStateChanged{false};
    };

public:
    void SetLogger(SilKit::Services::Logging::ILogger* logger);

    auto IsEmpty() const -> bool;

    auto UpdateRequiredParticipants(SilKit::Util::Span<const std::string> requiredParticipantNames)
        -> UpdateRequiredParticipantsResult;

    auto UpdateParticipantStatus(const ParticipantStatus& newParticipantStatus) -> UpdateParticipantStatusResult;

    auto RemoveParticipant(const std::string& participantName) -> RemoveParticipantResult;

    auto IsRequiredParticipant(const std::string& participantName) const -> bool;
    auto GetParticipantStatus(const std::string& participantName) const -> const ParticipantStatus*;
    auto GetSystemState() const -> SystemState;

private:
    auto GetOrCreateParticipantStatus(const std::string& participantName) -> const ParticipantStatus&;
    void SetParticipantStatus(const std::string& participantName, const ParticipantStatus& participantStatus);
    auto GetAnyRequiredParticipantState() const -> ParticipantState;
    auto ComputeSystemState(ParticipantState newParticipantState) const -> SystemState;

private:
    mutable std::recursive_mutex _mutex;

    SilKit::Services::Logging::ILogger* _logger{nullptr};
    std::unordered_set<std::string> _requiredParticipants;
    SystemState _systemState{};

    /// Mutable because GetParticipantStatus is allowed to insert the default (invalid) value.
    mutable std::unordered_map<std::string, ParticipantStatus> _participantStatusCache;
};


} // namespace VSilKit
