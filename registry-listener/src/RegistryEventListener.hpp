// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include "core/vasio/IRegistryEventListener.hpp"

class RegistryEventListener final : public SilKit::Core::IRegistryEventListener
{
public:
    void OnLoggerCreated(SilKit::Services::Logging::ILoggerInternal* logger) override;
    void OnRegistryUri(const std::string& registryUri) override;
    void OnGenerationCreated(const std::string& simulationName, uint64_t generation) override;
    void OnGenerationStateChanged(const std::string& simulationName, uint64_t generation,
                                   SilKit::Services::Orchestration::SystemState state) override;
    void OnGenerationRemoved(const std::string& simulationName, uint64_t generation) override;
    void OnParticipantConnected(const std::string& simulationName, const std::string& participantName,
                                 std::optional<uint64_t> generation) override;
    void OnParticipantDisconnected(const std::string& simulationName, const std::string& participantName,
                                    std::optional<uint64_t> generation) override;
    void OnRequiredParticipantsUpdate(const std::string& simulationName, const std::string& participantName,
                                      SilKit::Util::Span<const std::string> requiredParticipantNames,
                                      std::optional<uint64_t> generation) override;
    void OnParticipantStatusUpdate(
        const std::string& simulationName, const std::string& participantName,
        const SilKit::Services::Orchestration::ParticipantStatus& participantStatus,
        std::optional<uint64_t> generation) override;
    void OnServiceDiscoveryEvent(
        const std::string& simulationName, const std::string& participantName,
        const SilKit::Core::Discovery::ServiceDiscoveryEvent& serviceDiscoveryEvent) override;
    void OnMetricsUpdate(const std::string& simulationName, const std::string& origin,
                         const VSilKit::MetricsUpdate& metricsUpdate) override;
};
