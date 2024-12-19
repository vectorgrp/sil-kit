// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <iostream>

#include "silkit/SilKit.hpp"

using namespace std::chrono_literals;

std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    out << std::chrono::duration_cast<std::chrono::milliseconds>(timestamp).count() << "ms";
    return out;
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Wrong number of arguments! Start demo with: " << argv[0] << " <ParticipantName>" << std::endl;
        return -1;
    }
    std::string participantName(argv[1]);

    try
    {
        // Setup participant, lifecycle, time synchronization and logging
        const std::string registryUri = "silkit://localhost:8500";
        const std::string configString = R"({"Logging":{"Sinks":[{"Type":"Stdout","Level":"Info"}]}})";
        auto participantConfiguration = SilKit::Config::ParticipantConfigurationFromString(configString);

        auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryUri);
        auto logger = participant->GetLogger();

        auto* lifecycleService =
            participant->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated});

        auto* timeSyncService = lifecycleService->CreateTimeSyncService();

        // Simulation steps
        const auto stepSize = 1ms;
        timeSyncService->SetSimulationStepHandler(
            [logger](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
            std::stringstream ss;
            ss << "--------- Simulation step T=" << now << " ---------";
            logger->Info(ss.str());
        }, stepSize);

        auto finalStateFuture = lifecycleService->StartLifecycle();
        finalStateFuture.get();
    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        return -3;
    }

    return 0;
}
