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
#include "ib/cfg/ConfigBuilder.hpp"

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

void SendMessage(can::ICanController* controller, logging::ILogger* logger)
{
    can::CanMessage msg;
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
    if (argc < 2)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0] << " <ParticipantName> [domainId]" << std::endl;
        return -1;
    }
    
    try
    {
        std::string participantName(argv[1]);

        uint32_t domainId = 42;
        if (argc >= 4)
        {
            domainId = static_cast<uint32_t>(std::stoul(argv[2]));
        }

        ib::cfg::ConfigBuilder cfg("CAN_Writer_Cfg");
        cfg.WithActiveMiddleware(ib::cfg::Middleware::VAsio);
        auto& participant = cfg.SimulationSetup().AddParticipant(participantName);
        participant.AddCan("CAN1");
        participant.ConfigureLogger()
            .AddSink(ib::cfg::Sink::Type::Remote).WithLogLevel(ib::mw::logging::Level::Trace)
            ->AddSink(ib::cfg::Sink::Type::Stdout).WithLogLevel(ib::mw::logging::Level::Trace);
        cfg.WithActiveMiddleware(ib::cfg::Middleware::VAsio);

        auto ibConfig = cfg.Build();
        auto sleepTimePerTick = 1000ms;

        std::cout << "Creating ComAdapter for Participant=" << participantName << " in Domain " << domainId << std::endl;
        auto comAdapter = ib::CreateComAdapter(ibConfig, participantName, domainId);
        auto* logger = comAdapter->GetLogger();
        auto* canController = comAdapter->CreateCanController("CAN1");

        canController->RegisterTransmitStatusHandler(
            [logger](can::ICanController* /*ctrl*/, const can::CanTransmitAcknowledge& ack) {
                AckCallback(ack, logger);
            });
        canController->RegisterReceiveMessageHandler(
            [logger](can::ICanController* /*ctrl*/, const can::CanMessage& msg) {
                ReceiveMessage(msg, logger);
            });


        if (participantName.substr(0, std::string{"CanWriter"}.size()) == "CanWriter")
        {
            while (true)
            {
                SendMessage(canController, logger);
                std::this_thread::sleep_for(sleepTimePerTick);

            }
        }

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
