// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "Validation.hpp"

#include <iostream>

using namespace std::chrono_literals;

namespace ib {
namespace cfg {

void Validate(const Config& config)
{
    Validate(config.simulationSetup, config);
}

void Validate(const SimulationSetup& testConfig, const Config& ibConfig)
{
    for (auto& participant : testConfig.participants)
        Validate(participant, ibConfig);
    Validate(testConfig.timeSync, ibConfig);

    std::vector<mw::EndpointId> endpointIds;

    auto add_to_endpoints =
        [&endpointIds](const auto& services)
        {
            for (auto& service : services)
                endpointIds.push_back(service.endpointId);
        };

    for (auto& participant : testConfig.participants)
    {
        add_to_endpoints(participant.analogIoPorts);
        add_to_endpoints(participant.digitalIoPorts);
        add_to_endpoints(participant.patternPorts);
        add_to_endpoints(participant.pwmPorts);
        add_to_endpoints(participant.genericPublishers);
        add_to_endpoints(participant.genericSubscribers);
        add_to_endpoints(participant.canControllers);
        add_to_endpoints(participant.ethernetControllers);
        add_to_endpoints(participant.flexrayControllers);
        add_to_endpoints(participant.linControllers);
    }

    for (auto& sw : testConfig.switches)
        add_to_endpoints(sw.ports);

    std::sort(endpointIds.begin(), endpointIds.end());
    auto result = std::unique(endpointIds.begin(), endpointIds.end());
    if (result != endpointIds.end())
    {
        std::cerr << "ERROR: EndpointId " << *result << " is not unique!" << std::endl;
        throw ib::cfg::Misconfiguration{ "EndpointIds must be unique" };
    }
}

void Validate(const Participant& participant, const Config& ibConfig)
{
    //tracing related validation:
    std::vector<std::string> sinkNames;
    for(const auto& sink: participant.traceSinks)
    {
        sinkNames.push_back(sink.name);
    }
    auto sinkExists = [&sinkNames](const auto& name) -> bool {
        return std::end(sinkNames) != std::find(sinkNames.cbegin(), sinkNames.cend(), name);
    };

    auto validateController = [&sinkExists, &participant](const auto& controllersConfig)
    {
        for (const auto& ctrl : controllersConfig)
        {
            for (const auto& traceSink : ctrl.useTraceSinks)
            {
                if (traceSink.empty())
                {
                    std::cerr << "ERROR:  Participant \"" << participant.name
                        << "/" << ctrl.name
                        << "\" has a Tracer which refers to an empty TraceSink!"
                        << std::endl;
                    throw ib::cfg::Misconfiguration{"Tracer refers to an empty TraceSink name"};
                }
                if (!sinkExists(traceSink))
                {
                    std::cerr << "ERROR:  Participant \"" << participant.name 
                        << "/" << ctrl.name
                        << "\" has a Tracer which refers to a non-existing TraceSink: "
                        << traceSink
                        << std::endl;
                    throw ib::cfg::Misconfiguration{"Tracer refers to non-existing TraceSink name"};
                }
            }
        }

    };

    validateController(participant.ethernetControllers);
    validateController(participant.canControllers);
    validateController(participant.linControllers);
    validateController(participant.flexrayControllers);
    validateController(participant.genericPublishers);
    validateController(participant.genericSubscribers);
    validateController(participant.analogIoPorts);
    validateController(participant.digitalIoPorts);
    validateController(participant.patternPorts);
    validateController(participant.pwmPorts);

    //participant controller related validation:
    auto& participantController = participant.participantController;
    if (!participantController)
        return;

    if (participant.id == 0)
    {
        std::cerr << "ERROR: Participant " << participant.name << " uses ParticipantId 0!" << std::endl;
        throw ib::cfg::Misconfiguration{ "ParticipantId must not be 0" };
    }

    if (participantController->syncType == SyncType::Unsynchronized)
    {
        std::cerr << "ERROR: Participant " << participant.name << " uses ParticipantController but did not specify a SyncType!" << std::endl;
        throw ib::cfg::Misconfiguration{"SyncType of ParticipantControllers must not be SyncType::Unsynchronized"};
    }
}

void Validate(const TimeSync& testConfig, const Config& ibConfig)
{
    // The TickPeriod must not be zero in certain cases:
    // 1.) When using StrictSync (because it is used to derive the FastRTPS heart beat period)
    // 2.) When DiscreteTime synchronization is used
    if (testConfig.tickPeriod == 0ns)
    {
        if (testConfig.syncPolicy == TimeSync::SyncPolicy::Strict)
        {
            std::cerr << "ERROR: Invalid TimeSync configuration! TickPeriod must not be 0 when using SyncPolicy::Strict!" << std::endl;
            throw ib::cfg::Misconfiguration{"TickPeriod must not be 0"};
        }

        for (auto&& participant : ibConfig.simulationSetup.participants)
        {
            auto& participantController = participant.participantController;
            if (!participantController)
                continue;

            if (participantController->syncType == SyncType::DiscreteTime || participantController->syncType == SyncType::DiscreteTimePassive)
            {
                std::cerr << "ERROR: Invalid TimeSync configuration! TickPeriod must not be 0 when using SyncType::DiscreteTime!" << std::endl;
                throw ib::cfg::Misconfiguration{"TickPeriod must not be 0"};
            }
        }
    }
}

} // namespace cfg
} // namespace ib
