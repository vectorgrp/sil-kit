// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "silkit/participant/IParticipant.hpp"
#include "silkit/experimental/management/ManagementDatatypes.hpp"

#include "management/IRegistryManagementParticipant.hpp"

namespace VSilKit {

class RegistryManagementParticipant final : public IRegistryManagementParticipant
{
public:
    RegistryManagementParticipant(SilKit::Services::Logging::ILoggerInternal* logger, bool enableDomainSockets);
    ~RegistryManagementParticipant() override;

    // IRegistryManagementParticipant
    void OnParticipantConfiguration(const std::string& simulationName, const std::string& participantName,
                                    const std::string& configurationJson) override;
    void Start(const std::string& registryUri) override;
    void Stop() override;

    // IRegistryEventListener
    void OnLoggerCreated(SilKit::Services::Logging::ILoggerInternal*) override {}
    void OnRegistryUri(const std::string&) override {}
    void OnParticipantConnected(const std::string& simulationName, const std::string& participantName) override;
    void OnParticipantDisconnected(const std::string& simulationName, const std::string& participantName) override;
    void OnRequiredParticipantsUpdate(const std::string& simulationName, const std::string& participantName,
                                      SilKit::Util::Span<const std::string> requiredParticipantNames) override;
    void OnParticipantStatusUpdate(const std::string&, const std::string&,
                                   const SilKit::Services::Orchestration::ParticipantStatus&) override
    {
    }
    void OnServiceDiscoveryEvent(const std::string&, const std::string&,
                                 const SilKit::Core::Discovery::ServiceDiscoveryEvent&) override
    {
    }
    void OnMetricsUpdate(const std::string&, const std::string&, const VSilKit::MetricsUpdate&) override {}

public:
    auto GetRegistryInfo() const -> SilKit::Experimental::Management::RegistryInfo;
    auto GetSimulations() const -> SilKit::Experimental::Management::Simulations;
    auto GetParticipantConfiguration(const SilKit::Experimental::Management::ParticipantConfigurationRequest& request)
        const -> SilKit::Experimental::Management::ParticipantConfigurationReply;

private:
    struct ParticipantState
    {
        std::optional<std::string> configurationJson;
    };

    struct SimulationState
    {
        std::map<std::string, ParticipantState> participants;
        std::vector<std::string> requiredParticipantNames;
    };

private:
    auto MakeSimulations() const -> SilKit::Experimental::Management::Simulations;
    void PublishSimulations();

private:
    SilKit::Services::Logging::ILoggerInternal* _logger;
    bool _enableDomainSockets;

    mutable std::mutex _mutex;
    std::map<std::string, SimulationState> _simulations;
    uint64_t _sequence{0};
    std::unique_ptr<SilKit::IParticipant> _participant;
    SilKit::Services::PubSub::IDataPublisher* _simulationsPublisher{nullptr};
};

} // namespace VSilKit
