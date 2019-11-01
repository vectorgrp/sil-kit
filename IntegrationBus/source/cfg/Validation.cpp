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
}

void Validate(const Participant& participant, const Config& /*ibConfig*/)
{
    auto& participantController = participant.participantController;
    if (!participantController._is_configured)
        return;

    if (participantController.syncType == SyncType::Unsynchronized)
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
            if (!participantController._is_configured)
                continue;

            if (participantController.syncType == SyncType::DiscreteTime || participantController.syncType == SyncType::DiscreteTimePassive)
            {
                std::cerr << "ERROR: Invalid TimeSync configuration! TickPeriod must not be 0 when using SyncType::DiscreteTime!" << std::endl;
                throw ib::cfg::Misconfiguration{"TickPeriod must not be 0"};
            }
        }
    }
}

} // namespace cfg
} // namespace ib
