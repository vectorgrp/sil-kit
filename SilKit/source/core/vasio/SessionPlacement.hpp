// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <map>
#include <set>
#include <string>

#include "services/logging/ILoggerInternal.hpp"
#include "services/orchestration/SystemStateTracker.hpp"
#include "silkit/services/orchestration/OrchestrationDatatypes.hpp"

#include "core/vasio/IVAsioPeer.hpp"

namespace SilKit {
namespace Core {

//! \brief Decides into which generation a joining peer of a simulation is placed, and tracks the system state of
//! generations.
class SessionPlacement
{
public:
    //! The names of the registered peers by generation.
    using Snapshot = std::map<uint64_t, std::set<std::string>>;

public:
    SessionPlacement(std::string simulationName, Services::Logging::ILoggerInternal* logger);

    //! \brief Decides into which generation the joining peer is placed and stamps it on the service descriptor of the
    //! peer.
    void Admit(IVAsioPeer* peer, const Snapshot& registered);

    void OnStatus(const IServiceEndpoint* from, const Services::Orchestration::ParticipantStatus& msg,
                  const Snapshot& registered);

    void OnWorkflow(const IServiceEndpoint* from, const Services::Orchestration::WorkflowConfiguration& msg,
                    const Snapshot& registered);

    //! \brief Drops the state of the registered peer that is about to be removed, and the state of its generation if
    //! it is the last peer of the generation.
    void OnPeerLeaving(const IVAsioPeer* peer, bool isLastInGeneration);

private:
    struct Generation
    {
        VSilKit::SystemStateTracker stateTracker;
        bool used{false};
    };
    using Generations = std::map<uint64_t, Generation>;

private:
    bool ShouldEnterGeneration(const std::string& participantName, uint64_t generationNumber,
                               const std::set<std::string>& participants) const;
    auto SelectGeneration(const std::string& participantName, const Snapshot& registered) const -> uint64_t;

    auto GetOrCreateGeneration(const IServiceEndpoint* from, const Snapshot& registered) -> std::pair<uint64_t, Generation*>;

    void UpdateGenerationUsage(uint64_t generationNumber, Generation& generation);

private:
    std::string _simulationName;
    Services::Logging::ILoggerInternal* _logger;
    Generations _generations;

    friend struct SessionPlacementTestAccess;
};

} // namespace Core
} // namespace SilKit
