// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include <algorithm>
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <thread>

#include "ib/IntegrationBus.hpp"
#include "ib/sim/all.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"

using namespace ib::mw;
using namespace ib::sim;

using namespace std::chrono_literals;

std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
    out << seconds.count() << "s";
    return out;
}

std::ostream& operator<<(std::ostream& out, can::CanTransmitStatus status)
{
    switch (status)
    {
    case can::CanTransmitStatus::Canceled:
        out << "NACK(canceled)";
        break;
    case can::CanTransmitStatus::DuplicatedTransmitId:
        out << "NACK(duplicatedTransmitId)";
        break;
    case can::CanTransmitStatus::TransmitQueueFull:
        out << "NACK(transmitQueueFull)";
        break;
    case can::CanTransmitStatus::Transmitted:
        out << "ACK";
        break;
    default:
        assert(false);
    }
    return out;
}

void AckCallback(can::ICanController* /*controller*/, const can::CanTransmitAcknowledge& ack)
{
    std::cout << ">> " << ack.status
              << " for CAN Message with transmitId=" << ack.transmitId
              << " timestamp=" << ack.timestamp
              << std::endl;
}

void ReceiveMessage(can::ICanController* /*controller*/, const can::CanMessage& msg)
{
    std::string payload(msg.dataField.begin(), msg.dataField.end());
    std::cout << ">> CAN Message: canId=" << msg.canId
              << " timestamp=" << msg.timestamp
              << " \"" << payload << "\""
              << std::endl;
}

void SendMessage(can::ICanController* controller, std::chrono::nanoseconds now)
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
    std::cout << "<< CAN Message sent with transmitId=" << transmitId << std::endl;
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
    std::string configFilename = argv[1];
    std::string participantName = argv[2];

    uint32_t domainId = 42;
    if (argc >= 4)
    {
        domainId = static_cast<uint32_t>(std::stoul(argv[3]));
    }

    ib::cfg::Config ibConfig;
    try
    {
        ibConfig = ib::cfg::Config::FromJsonFile(configFilename);
    }
    catch (const ib::cfg::Misconfiguration& error)
    {
        std::cerr << "Invalid configuration: " << error.what() << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
        return -2;
    }

    const auto sleepTimePerTick = 1000ms;

    std::cout << "Creating ComAdapter for Participant=" << participantName << " in Domain " << domainId << std::endl;
    auto comAdapter = ib::CreateFastRtpsComAdapter(ibConfig, participantName, domainId);

    auto canController = comAdapter->CreateCanController("CAN1");
    canController->RegisterTransmitStatusHandler(&AckCallback);
    canController->RegisterReceiveMessageHandler(&ReceiveMessage);
    canController->SetBaudRate(10'000, 1'000'000);
    canController->Start();
    

    // Create a SyncMaster if the participant is configured to do so.
    // NB: New SyncMaster Interface is subject to changes (cf. AFTMAGT-124)
    auto& participantConfig = ib::cfg::get_by_name(ibConfig.simulationSetup.participants, participantName);
    if (participantConfig.isSyncMaster)
    {
        comAdapter->CreateSyncMaster();
        std::cout << "Created SyncMaster at Participant: " << participantName << std::endl;
    }

    // Set an Init Handler
    auto&& participantController = comAdapter->GetParticipantController();
    participantController->SetInitHandler([&participantName](auto initCmd) {

        std::cout << "Initializing " << participantName << std::endl;

    });

    // Set a Stop Handler
    participantController->SetStopHandler([]() {

        std::cout << "Stopping..." << std::endl;

    });

    // Set a Shutdown Handler
    participantController->SetShutdownHandler([]() {

        std::cout << "Shutting down..." << std::endl;

    });

    participantController->SetPeriod(1ms);
    if (participantName == "CanWriter")
    {
        participantController->SetSimulationTask([canController, sleepTimePerTick](std::chrono::nanoseconds now) {

            std::cout << "now=" << now << std::endl;
            SendMessage(canController, now);
            std::this_thread::sleep_for(sleepTimePerTick);

        });
    }
    else
    {
        participantController->SetSimulationTask([sleepTimePerTick](std::chrono::nanoseconds now) {

            std::cout << "now=" << now << std::endl;
            std::this_thread::sleep_for(sleepTimePerTick);

        });
    }

    //auto finalStateFuture = participantController->RunAsync();
    //auto finalState = finalStateFuture.get();

    auto finalState = participantController->Run();

    std::cout << "Simulation stopped. Final State: " << finalState << std::endl;
    std::cout << "Press enter to stop the process..." << std::endl;
    std::cin.ignore();

    return 0;
}
