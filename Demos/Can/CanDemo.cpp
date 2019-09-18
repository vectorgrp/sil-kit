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

void AckCallback(const can::CanTransmitAcknowledge& ack, logging::ILogger* logger)
{
    std::stringstream buffer;
    buffer << ">> " << ack.status
           << " for CAN Message with transmitId=" << ack.transmitId
           << " timestamp=" << ack.timestamp;
    logger->Info(buffer.str());
}

void ReceiveMessage(const can::CanMessage& msg, logging::ILogger* logger)
{
    std::string payload(msg.dataField.begin(), msg.dataField.end());
    std::stringstream buffer;
    buffer << ">> CAN Message: canId=" << msg.canId
           << " timestamp=" << msg.timestamp
           << " \"" << payload << "\"";
    logger->Info(buffer.str());
}

void SendMessage(can::ICanController* controller, std::chrono::nanoseconds now, logging::ILogger* logger)
{
    can::CanMessage msg;
    msg.timestamp = now;
    msg.canId = 17;
    msg.flags.ide = 0; // Identifier Extension
    msg.flags.rtr = 0; // Remote Transmission Request
    msg.flags.fdf = 1; // FD Format Indicator
    msg.flags.brs = 1; // Bit Rate Switch  (for FD Format only)
    msg.flags.esi = 0; // Error State indicator (for FD Format only)

    static int msgId = 0;
    std::stringstream payloadBuilder;
    payloadBuilder << "CAN " << msgId++;
    auto payloadStr = payloadBuilder.str();

    msg.dataField.assign(payloadStr.begin(), payloadStr.end());

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
        std::cerr << "Missing arguments! Start demo with: " << argv[0] << " <IbConfig.json> <ParticipantName> [domainId]" << std::endl;
        return -1;
    }
    
    try
    {
        std::string configFilename(argv[1]);
        std::string participantName(argv[2]);

        uint32_t domainId = 42;
        if (argc >= 4)
        {
            domainId = static_cast<uint32_t>(std::stoul(argv[3]));
        }   

        auto ibConfig = ib::cfg::Config::FromJsonFile(configFilename);
        auto sleepTimePerTick = 1000ms;

        std::cout << "Creating ComAdapter for Participant=" << participantName << " in Domain " << domainId << std::endl;
        auto comAdapter = ib::CreateComAdapter(ibConfig, participantName, domainId);
        auto* logger = comAdapter->GetLogger();
        auto* participantController = comAdapter->GetParticipantController();
        auto* canController = comAdapter->CreateCanController("CAN1");

        canController->RegisterTransmitStatusHandler(
            [logger](can::ICanController* /*ctrl*/, const can::CanTransmitAcknowledge& ack) {
                AckCallback(ack, logger);
            });
        canController->RegisterReceiveMessageHandler(
            [logger](can::ICanController* /*ctrl*/, const can::CanMessage& msg) {
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

        participantController->SetPeriod(200us);
        if (participantName == "CanWriter")
        {
            participantController->SetSimulationTask(
                [canController, logger, sleepTimePerTick](std::chrono::nanoseconds now, std::chrono::nanoseconds duration) {

                    std::cout << "now=" << now << ", duration=" << duration << std::endl;
                    SendMessage(canController, now, logger);
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

        //auto finalStateFuture = participantController->RunAsync();
        //auto finalState = finalStateFuture.get();

        auto finalState = participantController->Run();

        std::cout << "Simulation stopped. Final State: " << finalState << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
    }
    catch (const ib::cfg::Misconfiguration& error)
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
