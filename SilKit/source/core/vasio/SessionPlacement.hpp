// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>

#include "services/logging/ILoggerInternal.hpp"
#include "services/orchestration/SystemStateTracker.hpp"
#include "silkit/services/orchestration/OrchestrationDatatypes.hpp"

#include "core/vasio/IVAsioPeer.hpp"

namespace SilKit {
namespace Core {

struct IRegistryEventListener;

//! \brief Assigns peers to simulation generations and tracks their lifecycle.
class SessionPlacement
{
public:
    //! The names of the registered peers by generation.
    using Snapshot = std::map<uint64_t, std::set<std::string>>;

public:
    SessionPlacement(std::string simulationName, Services::Logging::ILoggerInternal* logger,
                     IRegistryEventListener* registryEventListener = nullptr);

    //! \brief Assigns a generation on the peer's service descriptor without registering the peer.
    //! A known controller's assignment reserves its slot even if registration subsequently fails.
    //! Returns false for a duplicate non-required participant other than the controller.
    [[nodiscard]] bool Admit(IVAsioPeer* peer, const Snapshot& registered);

    void OnStatus(const IServiceEndpoint* from, const Services::Orchestration::ParticipantStatus& msg,
                  const Snapshot& registered);

    //! \brief Accepts only workflows matching the first accepted controller name and required-participant set.
    void OnWorkflow(const IServiceEndpoint* from, const Services::Orchestration::WorkflowConfiguration& msg,
                    const Snapshot& registered);

    //! \brief Removes a departing peer's state; discards the generation if this is its last peer.
    void OnPeerLeaving(const IVAsioPeer* peer, bool isLastInGeneration);

private:
    struct WorkflowContract
    {
        std::string controllerName;
        std::set<std::string> requiredParticipantNames;
    };

    struct Generation
    {
        VSilKit::SystemStateTracker stateTracker;
        bool used{false};
        bool controllerAssigned{false};
    };
    using Generations = std::map<uint64_t, Generation>;

private:
    auto SelectGeneration(const std::string& participantName, const Snapshot& registered) const
        -> std::optional<uint64_t>;

    auto GetOrCreateGeneration(const IServiceEndpoint* from, const Snapshot& registered) -> std::pair<uint64_t, Generation*>;
    auto GetOrCreateGeneration(uint64_t generationNumber) -> Generation&;

    void UpdateGenerationState(uint64_t generationNumber, Generation& generation, bool systemStateChanged);

private:
    std::string _simulationName;
    Services::Logging::ILoggerInternal* _logger;
    IRegistryEventListener* _registryEventListener;
    Generations _generations;
    uint64_t _latestGeneration{0};
    std::optional<WorkflowContract> _workflowContract;

    friend struct SessionPlacementTestAccess;
};

} // namespace Core
} // namespace SilKit
