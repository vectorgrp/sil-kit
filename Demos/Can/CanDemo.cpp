// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <algorithm>
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <thread>

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
    CanMessage msg;
    msg.canId = 17;
    msg.flags.ide = 0; // Identifier Extension
    msg.flags.rtr = 0; // Remote Transmission Request
    msg.flags.fdf = 0; // FD Format Indicator
    msg.flags.brs = 1; // Bit Rate Switch  (for FD Format only)
    msg.flags.esi = 0; // Error State indicator (for FD Format only)

    static int msgId = 0;
    std::stringstream payloadBuilder;
    payloadBuilder << "CAN " << msgId++;
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
        std::cerr << "Missing arguments! Start demo with: " << argv[0] << " <ParticipantConfiguration.yaml|json> <ParticipantName> [domainId]" << std::endl;
        return -1;
    }
    
    try
    {
        std::string participantConfigurationFilename(argv[1]);
        std::string participantName(argv[2]);

        uint32_t domainId = 42;
        if (argc >= 4)
        {
            domainId = static_cast<uint32_t>(std::stoul(argv[3]));
        }   

        auto participantConfiguration = ib::cfg::ParticipantConfigurationFromFile(participantConfigurationFilename);
        auto sleepTimePerTick = 1000ms;

        std::cout << "Creating ComAdapter for Participant=" << participantName << " in Domain " << domainId << std::endl;

        auto participant = ib::CreateSimulationParticipant(participantConfiguration, participantName, domainId, true);

        auto* logger = participant->GetLogger();
        auto* participantController = participant->GetParticipantController();
        auto* canController = participant->CreateCanController("CAN1");

        canController->RegisterTransmitStatusHandler(
            [logger](ICanController* /*ctrl*/, const CanTransmitAcknowledge& ack) {
                AckCallback(ack, logger);
            });
        canController->RegisterReceiveMessageHandler(
            [logger](ICanController* /*ctrl*/, const CanMessage& msg) {
                ReceiveMessage(msg, logger);
            });

        // Set an Init Handler
        participantController->SetInitHandler([canController, &participantName](auto initCmd) {

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
                [canController, logger, sleepTimePerTick, &participantController](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {

                    std::cout << "now=" << now << ", duration=" << duration << std::endl;
                    SendMessage(canController, logger);
                    std::this_thread::sleep_for(sleepTimePerTick);

                    if (now == 100ms)
                    {
                        std::cout << "Switching to period length of 1ms..." << std::endl;
                        participantController->SetPeriod(1ms);
                    }

            });

            // This process will disconnect and reconnect during a coldswap
            participantController->EnableColdswap();
        }
        else
        {
            participantController->SetSimulationTask(
                [sleepTimePerTick, &participantController](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {

                    std::cout << "now=" << now << ", duration=" << duration << std::endl;
                    std::this_thread::sleep_for(sleepTimePerTick);

                    if (now == 100ms)
                    {
                        std::cout << "Switching to period length of 1ms..." << std::endl;
                        participantController->SetPeriod(1ms);
                    }
            });
        }

        //auto finalStateFuture = participantController->RunAsync();
        //auto finalState = finalStateFuture.get();

        auto finalState = participantController->Run();

        std::cout << "Simulation stopped. Final State: " << finalState << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
    }
    catch (const ib::configuration_error& error)
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
