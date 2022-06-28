// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <algorithm>
#include <cstring>
#include <iostream>
#include <iterator>
#include <string>
#include <sstream>
#include <thread>
#include <vector>

#include "ib/IntegrationBus.hpp"
#include "ib/mw/logging/ILogger.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"
#include "ib/sim/can/all.hpp"
#include "ib/sim/can/string_utils.hpp"

using namespace ib::mw;
using namespace ib::sim;
using namespace ib::sim::can;

using namespace std::chrono_literals;

namespace std {
namespace chrono {
std::ostream& operator<<(std::ostream& out, nanoseconds timestamp)
{
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
    out << seconds.count() << "s";
    return out;
}
}
}

void FrameTransmitHandler(const CanFrameTransmitEvent& ack, logging::ILogger* logger)
{
    std::stringstream buffer;
    buffer << ">> " << ack.status
           << " for CAN Message with transmitId=" << ack.transmitId
           << " timestamp=" << ack.timestamp;
    logger->Info(buffer.str());
}

void FrameHandler(const CanFrameEvent& frameEvent, logging::ILogger* logger)
{
    std::string payload(frameEvent.frame.dataField.begin(), frameEvent.frame.dataField.end());
    std::stringstream buffer;
    buffer << ">> CAN Message: canId=" << frameEvent.frame.canId
           << " timestamp=" << frameEvent.timestamp
           << " \"" << payload << "\"";
    logger->Info(buffer.str());
}

void SendFrame(ICanController* controller, logging::ILogger* logger)
{
    CanFrame canFrame {};
    canFrame.canId = 3;
    canFrame.flags.ide = 0; // Identifier Extension
    canFrame.flags.rtr = 0; // Remote Transmission Request
    canFrame.flags.fdf = 1; // FD Format Indicator
    canFrame.flags.brs = 1; // Bit Rate Switch  (for FD Format only)
    canFrame.flags.esi = 0; // Error State indicator (for FD Format only)

    static int msgId = 0;
    std::stringstream payloadBuilder;
    payloadBuilder << "CAN " << (msgId++)%100;
    auto payloadStr = payloadBuilder.str();

    canFrame.dataField.assign(payloadStr.begin(), payloadStr.end());
    canFrame.dlc = canFrame.dataField.size();

    auto transmitId = controller->SendFrame(std::move(canFrame));
    std::stringstream buffer;
    buffer << "<< CAN Message sent with transmitId=" << transmitId;
    logger->Info(buffer.str());
}

/**************************************************************************************************
 * Main Function
 **************************************************************************************************/

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0]
                  << " <ParticipantConfiguration.yaml|json> <ParticipantName> [domainId] [--async]" << std::endl
                  << "Use \"CanWriter\" or \"CanReader\" as <ParticipantName>." << std::endl;
        return -1;
    }

    if (argc > 5)
    {
        std::cerr << "Too many arguments! Start demo with: " << argv[0]
                  << " <ParticipantConfiguration.yaml|json> <ParticipantName> [domainId] [--async]" << std::endl
                  << "Use \"CanWriter\" or \"CanReader\" as <ParticipantName>." << std::endl;
        return -1;
    }
    
    try
    {
        std::string participantConfigurationFilename(argv[1]);
        std::string participantName(argv[2]);

        uint32_t domainId = 42;

        bool runSync = true;

        std::vector<std::string> args;
        std::copy((argv + 3), (argv + argc), std::back_inserter(args));

        for (auto arg : args)
        {
            if (arg == "--async")
            {
                runSync = false;
            }
            else
            {
                try
                {
                    domainId = static_cast<uint32_t>(std::stoul(arg));
                }
                catch (...)
                {
                    std::cout << "Error: expected a numeric argument for [domainId] but got '" << arg << "'"
                              << std::endl;
                    return -1;
                }
            }
        }

        auto participantConfiguration = ib::cfg::ParticipantConfigurationFromFile(participantConfigurationFilename);
        auto sleepTimePerTick = 1000ms;

        std::cout << "Creating participant '" << participantName << "' in domain " << domainId << std::endl;

        auto participant = ib::CreateParticipant(participantConfiguration, participantName, domainId);

        auto* logger = participant->GetLogger();
        auto* canController = participant->CreateCanController("CAN1");

        canController->AddFrameTransmitHandler(
            [logger](ICanController* /*ctrl*/, const CanFrameTransmitEvent& ack) {
                FrameTransmitHandler(ack, logger);
            });
        canController->AddFrameHandler(
            [logger](ICanController* /*ctrl*/, const CanFrameEvent& frameEvent) {
                FrameHandler(frameEvent, logger);
            });

        if (runSync)
        {
            auto* lifecycleService = participant->GetLifecycleService();
            auto* timeSyncService = lifecycleService->GetTimeSyncService();
            // Set an Init Handler
            lifecycleService->SetCommunicationReadyHandler([canController, &participantName]() {
                std::cout << "Initializing " << participantName << std::endl;
                canController->SetBaudRate(10'000, 1'000'000);
                canController->Start();
            });

            // Set a Stop Handler
            lifecycleService->SetStopHandler([]() {
                std::cout << "Stopping..." << std::endl;
            });

            // Set a Shutdown Handler
            lifecycleService->SetShutdownHandler([]() {
                std::cout << "Shutting down..." << std::endl;
            });

            timeSyncService->SetPeriod(5ms);
            if (participantName == "CanWriter")
            {
                timeSyncService->SetSimulationTask(
                    [canController, logger, sleepTimePerTick](std::chrono::nanoseconds now,
                                                              std::chrono::nanoseconds duration) {
                        std::cout << "now=" << now << ", duration=" << duration << std::endl;
                        SendFrame(canController, logger);
                        std::this_thread::sleep_for(sleepTimePerTick);
                    });
            }
            else
            {
                timeSyncService->SetSimulationTask(
                    [sleepTimePerTick](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {
                        std::cout << "now=" << now << ", duration=" << duration << std::endl;
                        std::this_thread::sleep_for(sleepTimePerTick);
                    });
            }

            auto finalStateFuture = lifecycleService->StartLifecycleWithSyncTime(timeSyncService, {true, true});
            auto finalState = finalStateFuture.get();

            std::cout << "Simulation stopped. Final State: " << finalState << std::endl;
            std::cout << "Press enter to stop the process..." << std::endl;
            std::cin.ignore();
        }
        else
        {
            bool isStopped = false;
            std::thread workerThread;

            if (participantName == "CanWriter")
            {
                workerThread = std::thread{[&]() {
                    while (!isStopped)
                    {
                        SendFrame(canController, logger);
                        std::this_thread::sleep_for(sleepTimePerTick);
                    }
                }};
            }
            else
            {
                workerThread = std::thread{[&]() {
                    while (!isStopped)
                    {
                        std::this_thread::sleep_for(sleepTimePerTick);
                    }
                }};
            }

            std::cout << "Press enter to stop the process..." << std::endl;
            std::cin.ignore();
            isStopped = true;
            workerThread.join();
        }
    }
    catch (const ib::ConfigurationError& error)
    {
        std::cerr << "Invalid configuration: " << error.what() << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
        return -2;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
        return -3;
    }

    return 0;
}
