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

void AckCallback(const CanTransmitAcknowledge& ack, logging::ILogger* logger)
{
    std::stringstream buffer;
    buffer << ">> " << ack.status
           << " for CAN Message with transmitId=" << ack.transmitId
           << " timestamp=" << ack.timestamp;
    logger->Info(buffer.str());
}

void ReceiveMessage(const CanMessage& msg, logging::ILogger* logger)
{
    std::string payload(msg.dataField.begin(), msg.dataField.end());
    std::stringstream buffer;
    buffer << ">> CAN Message: canId=" << msg.canId
           << " timestamp=" << msg.timestamp
           << " \"" << payload << "\"";
    logger->Info(buffer.str());
}

void SendMessage(ICanController* controller, logging::ILogger* logger)
{
    CanMessage msg {};
    msg.canId = 3;
    msg.flags.ide = 0; // Identifier Extension
    msg.flags.rtr = 0; // Remote Transmission Request
    msg.flags.fdf = 1; // FD Format Indicator
    msg.flags.brs = 1; // Bit Rate Switch  (for FD Format only)
    msg.flags.esi = 0; // Error State indicator (for FD Format only)

    static int msgId = 0;
    std::stringstream payloadBuilder;
    payloadBuilder << "CAN " << (msgId++)%100;
    auto payloadStr = payloadBuilder.str();

    msg.dataField.assign(payloadStr.begin(), payloadStr.end());
    msg.dlc = msg.dataField.size();

    auto transmitId = controller->SendMessage(std::move(msg));
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

        auto participant = ib::CreateParticipant(participantConfiguration, participantName, domainId, runSync);

        auto* logger = participant->GetLogger();
        auto* canController = participant->CreateCanController("CAN1");

        canController->RegisterTransmitStatusHandler(
            [logger](ICanController* /*ctrl*/, const CanTransmitAcknowledge& ack) {
                AckCallback(ack, logger);
            });
        canController->RegisterReceiveMessageHandler(
            [logger](ICanController* /*ctrl*/, const CanMessage& msg) {
                ReceiveMessage(msg, logger);
            });

        if (runSync)
        {
            auto* participantController = participant->GetParticipantController();
            // Set an Init Handler
            participantController->SetInitHandler([canController, &participantName](auto /*initCmd*/) {
                std::cout << "Initializing " << participantName << std::endl;
                canController->SetBaudRate(10'000, 1'000'000);
                canController->Start();
            });

            // Set a Stop Handler
            participantController->SetStopHandler([]() {
                std::cout << "Stopping..." << std::endl;
            });

            // Set a Shutdown Handler
            participantController->SetShutdownHandler([]() {
                std::cout << "Shutting down..." << std::endl;
            });

            participantController->SetPeriod(5ms);
            if (participantName == "CanWriter")
            {
                participantController->SetSimulationTask(
                    [canController, logger, sleepTimePerTick](std::chrono::nanoseconds now,
                                                              std::chrono::nanoseconds duration) {
                        std::cout << "now=" << now << ", duration=" << duration << std::endl;
                        SendMessage(canController, logger);
                        std::this_thread::sleep_for(sleepTimePerTick);
                    });

                // This process will disconnect and reconnect during a coldswap
                participantController->EnableColdswap();
            }
            else
            {
                participantController->SetSimulationTask(
                    [sleepTimePerTick](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {
                        std::cout << "now=" << now << ", duration=" << duration << std::endl;
                        std::this_thread::sleep_for(sleepTimePerTick);
                    });
            }

            auto finalState = participantController->Run();

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
                        SendMessage(canController, logger);
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
