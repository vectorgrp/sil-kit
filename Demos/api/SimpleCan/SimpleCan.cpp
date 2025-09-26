// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <iostream>

#include "silkit/SilKit.hpp"

using namespace SilKit::Services::Can;
using namespace std::chrono_literals;

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
        auto* lifecycleService =
            participant->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated});
        auto* timeSyncService = lifecycleService->CreateTimeSyncService();
        auto* logger = participant->GetLogger();

        // CAN controller
        auto* canController = participant->CreateCanController("CanController1", "CAN1");

        canController->AddFrameTransmitHandler(
            [logger](ICanController* /*ctrl*/, const CanFrameTransmitEvent& /*ack*/) {
            logger->Info("Receive CAN frame transmit acknowledge");
        });
        canController->AddFrameHandler([logger](ICanController* /*ctrl*/, const CanFrameEvent& canFrameEvent) {
            std::string payload(canFrameEvent.frame.dataField.begin(), canFrameEvent.frame.dataField.end());
            logger->Info("Receive CAN frame: data='" + payload + "'");
        });

        // Initialize CAN controller
        lifecycleService->SetCommunicationReadyHandler([canController]() {
            canController->SetBaudRate(10'000, 1'000'000, 2'000'000);
            canController->Start();
        });

        // Simulation steps
        const auto stepSize = 1ms;
        timeSyncService->SetSimulationStepHandler(
            [participantName, canController, logger](std::chrono::nanoseconds now,
                                                     std::chrono::nanoseconds /*duration*/) {
            // Send CAN Frame every 10 seconds
            if (now.count() % std::chrono::nanoseconds(10s).count() == 0)
            {
                logger->Info("--------- T = " + std::to_string(now.count() / 1000000000) + "s ---------");

                // Create CAN frame with dynamic content
                const std::string payloadStr = "Data from " + participantName + ": " + std::to_string(std::rand());
                std::vector<uint8_t> payloadBytes(payloadStr.begin(), payloadStr.end());
                CanFrame canFrame{};
                canFrame.canId = 1;
                canFrame.dataField = SilKit::Util::ToSpan(payloadBytes);
                canFrame.dlc = static_cast<uint16_t>(canFrame.dataField.size());

                // Send frame (move ownership of canFrame to avoid additional copy)
                logger->Info("Sending CAN frame: data='" + payloadStr + "'");
                canController->SendFrame(canFrame);
            }
        },
            stepSize);

        // Start and wait
        auto finalStateFuture = lifecycleService->StartLifecycle();
        finalStateFuture.get();
    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        return -2;
    }

    return 0;
}
