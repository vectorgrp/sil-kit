// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "core/vasio/SessionPlacement.hpp"

#include <utility>

#include "fmt/ranges.h"

#include "services/logging/LoggerMessage.hpp"
#include "silkit/services/orchestration/string_utils.hpp"

#include "core/internal/traits/SilKitLoggingTraits.hpp"

namespace Log = SilKit::Services::Logging;

namespace SilKit {
namespace Core {

namespace {

constexpr uint64_t FIRST_GENERATION{1};

} // namespace

SessionPlacement::SessionPlacement(std::string simulationName, Services::Logging::ILoggerInternal* logger)
    : _simulationName{std::move(simulationName)}
    , _logger{logger}
{
}

bool SessionPlacement::ShouldEnterGeneration(const std::string& participantName, uint64_t generationNumber,
                                             const std::set<std::string>& participants) const
{
    if (participants.count(participantName) != 0)
    {
        return false;
    }

    const auto generationIt{_generations.find(generationNumber)};
    if (generationIt == _generations.end())
    {
        return true;
    }

    // Required participants and the controller cannot join a run that has already started.
    return !generationIt->second.used;
}

auto SessionPlacement::SelectGeneration(const std::string& participantName, const Snapshot& registered) const
    -> uint64_t
{
    if (registered.empty())
    {
        return FIRST_GENERATION;
    }

    if (_workflowContract.has_value() && _workflowContract->controllerName != participantName
        && _workflowContract->requiredParticipantNames.count(participantName) == 0)
    {
        const auto& newest{*registered.rbegin()};
        return newest.second.count(participantName) == 0 ? newest.first : newest.first + 1;
    }

    // std::map iterates in ascending generation order
    for (const auto& generationPair : registered)
    {
        if (ShouldEnterGeneration(participantName, generationPair.first, generationPair.second))
        {
            return generationPair.first;
        }
    }

    return registered.rbegin()->first + 1;
}

void SessionPlacement::Admit(IVAsioPeer* peer, const Snapshot& registered)
{
    const auto& participantName{peer->GetInfo().participantName};

    const auto generation{SelectGeneration(participantName, registered)};

    _logger->MakeMessage(Log::Level::Debug, TopicOf(*this))
        .SetMessage("Participant '{}' in simulation '{}' is placed into generation {}", participantName,
                    _simulationName, generation)
        .Dispatch();

    // The generation is stamped on the service descriptor of the peer, like the simulation name, so that receivers of
    // messages and the connection can read it from there. Names are not unique across generations, so peers are
    // identified by pointer.
    auto peerDescriptor{peer->GetServiceDescriptor()};
    peerDescriptor.SetGeneration(generation);
    peer->SetServiceDescriptor(peerDescriptor);
}

auto SessionPlacement::GetOrCreateGeneration(const IServiceEndpoint* from, const Snapshot& registered)
    -> std::pair<uint64_t, Generation*>
{
    if (from == nullptr)
    {
        return {0, nullptr};
    }

    const auto generationNumber{from->GetServiceDescriptor().GetGeneration()};
    if (!generationNumber.has_value() || registered.count(*generationNumber) == 0)
    {
        return {0, nullptr};
    }

    const auto [generationIt, created] = _generations.try_emplace(*generationNumber);
    if (created)
    {
        generationIt->second.stateTracker.SetLogger(_logger);
    }
    return {*generationNumber, &generationIt->second};
}

void SessionPlacement::OnStatus(const IServiceEndpoint* from, const Services::Orchestration::ParticipantStatus& msg,
                                const Snapshot& registered)
{
    const auto [generationNumber, generation] = GetOrCreateGeneration(from, registered);
    if (generation == nullptr)
    {
        return;
    }

    // Late messages of a participant that has already left must not bring it back into the tracker.
    if (registered.at(generationNumber).count(msg.participantName) != 0)
    {
        generation->stateTracker.UpdateParticipantStatus(msg);
        UpdateGenerationUsage(generationNumber, *generation);
    }
}

void SessionPlacement::OnWorkflow(const IServiceEndpoint* from,
                                  const Services::Orchestration::WorkflowConfiguration& msg,
                                  const Snapshot& registered)
{
    if (from == nullptr)
    {
        return;
    }

    const auto& descriptor{from->GetServiceDescriptor()};
    const auto generationNumber{descriptor.GetGeneration()};
    if (!generationNumber.has_value() || registered.count(*generationNumber) == 0)
    {
        return;
    }

    const auto& controllerName{descriptor.GetParticipantName()};
    std::set<std::string> requiredParticipantNames{msg.requiredParticipantNames.begin(),
                                                 msg.requiredParticipantNames.end()};
    if (_workflowContract.has_value()
        && (_workflowContract->controllerName != controllerName
            || _workflowContract->requiredParticipantNames != requiredParticipantNames))
    {
        _logger->MakeMessage(Log::Level::Error, TopicOf(*this))
            .SetMessage("Rejected workflow for simulation '{}' in generation {}: expected controller '{}' with "
                        "required participants [{}], received controller '{}' with required participants [{}]",
                        _simulationName, *generationNumber, _workflowContract->controllerName,
                        fmt::join(_workflowContract->requiredParticipantNames, ", "), controllerName,
                        fmt::join(requiredParticipantNames, ", "))
            .Dispatch();
        return;
    }

    if (!_workflowContract.has_value())
    {
        _workflowContract = WorkflowContract{controllerName, std::move(requiredParticipantNames)};
    }

    const auto [number, generation] = GetOrCreateGeneration(from, registered);
    generation->stateTracker.UpdateRequiredParticipants(msg.requiredParticipantNames);
    UpdateGenerationUsage(number, *generation);
}

void SessionPlacement::OnPeerLeaving(const IVAsioPeer* peer, bool isLastInGeneration)
{
    const auto generationNumber{peer->GetServiceDescriptor().GetGeneration()};
    if (!generationNumber.has_value())
    {
        return;
    }

    const auto generationIt{_generations.find(*generationNumber)};
    if (generationIt == _generations.end())
    {
        return;
    }

    if (isLastInGeneration)
    {
        _generations.erase(generationIt);
    }
    else
    {
        generationIt->second.stateTracker.RemoveParticipant(peer->GetInfo().participantName);
    }
}

void SessionPlacement::UpdateGenerationUsage(uint64_t generationNumber, Generation& generation)
{
    if (generation.used)
    {
        return;
    }

    using SystemState = SilKit::Services::Orchestration::SystemState;

    switch (generation.stateTracker.GetSystemState())
    {
    case SystemState::ReadyToRun:
    case SystemState::Running:
    case SystemState::Paused:
    case SystemState::Stopping:
    case SystemState::Stopped:
    case SystemState::ShuttingDown:
    case SystemState::Shutdown:
        break;
    default:
        return;
    }

    generation.used = true;

    _logger->MakeMessage(Log::Level::Info, TopicOf(*this))
        .SetMessage("The simulation '{}' in generation {} has started (system state {}), required participants and "
                    "the system controller can no longer join this generation",
                    _simulationName, generationNumber, to_string(generation.stateTracker.GetSystemState()))
        .Dispatch();
}

} // namespace Core
} // namespace SilKit
