// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <sstream>
#include <thread>

#include "ib/IntegrationBus.hpp"
#include "ib/sim/all.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"

using namespace ib::mw;
using namespace ib::sim::generic;
using namespace std::chrono_literals;

void PublishMessage(IGenericPublisher* publisher, std::string topicname)
{
    static auto msgIdx = 0;

    std::stringstream messageBuilder;
    messageBuilder << topicname << " UniqueMsgId=" << msgIdx++;
    auto message = messageBuilder.str();

    std::cout << "<< Send GenericMessage with data=" << message << std::endl;

    std::vector<uint8_t> data{message.begin(), message.end()};
    publisher->Publish(std::move(data));
}

void ReceiveMessage(IGenericSubscriber* subscriber, const std::vector<uint8_t>& data)
{
    std::string message{data.begin(), data.end()};
    std::cout
        << ">> Received new " << subscriber->Config().name
        << " Message: with data=\"" << message << "\"" << std::endl;
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

    if (argc < 2)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0] << " <participantId> [domainId]" << std::endl;
        return -1;
    }

    std::cout << "Creating GenericAdapter for participant=" << participantName << " in domain " << domainId << std::endl;
    auto comAdapter = ib::CreateFastRtpsComAdapter(ibConfig, participantName, domainId);

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
    if (participantName == "Publisher")
    {
        auto* groundTruth = comAdapter->CreateGenericPublisher("GroundTruth");
        auto* vehicleModelOut = comAdapter->CreateGenericPublisher("VehicleModelOut");

        participantController->SetSimulationTask(
            [groundTruth, vehicleModelOut](std::chrono::nanoseconds now)
            {
                auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                PublishMessage(groundTruth, "GroundTruth");
                PublishMessage(vehicleModelOut, "VehicleModelOut");
                std::this_thread::sleep_for(1s);
            }
        );
    }
    else
    {
        auto* groundTruth = comAdapter->CreateGenericSubscriber("GroundTruth");
        auto* vehicleModelOut = comAdapter->CreateGenericSubscriber("VehicleModelOut");
        groundTruth->SetReceiveMessageHandler(ReceiveMessage);
        vehicleModelOut->SetReceiveMessageHandler(ReceiveMessage);

        participantController->SetSimulationTask(
            [groundTruth, vehicleModelOut](std::chrono::nanoseconds now)
            {
                auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                std::this_thread::sleep_for(1s);
            }
        );
    }

    //auto finalStateFuture = participantController->RunAsync();
    //auto finalState = finalStateFuture.get();

    auto finalState = participantController->Run();

    std::cout << "Simulation stopped. Final State: " << finalState << std::endl;
    std::cout << "Press enter to stop the process..." << std::endl;
    std::cin.ignore();

    return 0;
}