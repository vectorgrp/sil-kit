// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <iostream>
#include <memory>
#include <string_view>
#include <thread>

#include "silkit/SilKit.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"
#include "silkit/Services/Orchestration/ILifecycleService.hpp"
#include "silkit/Services/Orchestration/ITimeSyncService.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"

#include "../../communication/include/CommandlineParser.hpp"

using namespace SilKit::Services::Ethernet;
using namespace std::chrono_literals;

void OnEthernetFrameReceived(IEthernetController* /*ctrl*/, const EthernetFrameEvent& event)
{
    std::cout << "Ethernet frame received at t=" << event.timestamp.count() << std::endl;
}

void OnEthernetFrameTransmitted(IEthernetController* /*ctrl*/, const EthernetFrameTransmitEvent& event)
{
    std::cout << "Ethernet frame transmitted at t=" << event.timestamp.count() << std::endl;
}

struct Participant
{
    std::unique_ptr<SilKit::IParticipant> participant{};
    SilKit::Services::Ethernet::IEthernetController* ethernetController{};
    SilKit::Services::Orchestration::ILifecycleService* lifecycleService{};
    SilKit::Services::Orchestration::ITimeSyncService* timeSyncService{};
    std::future<SilKit::Services::Orchestration::ParticipantState> finalStateFuture;
};

struct Arguments
{
    int stepSizeNs = 1000000;
    int bandwidthKbits = 1000;
    int durationSec = 1;
    std::string registryUri = "silkit://localhost:0";
};


void OnSimulationStep(Participant& participant, std::chrono::nanoseconds now, std::chrono::nanoseconds duration)
{
    // Placeholder for simulation step logic
    std::cout << "Simulation step for participant at t=" << now.count() << " duration=" << duration.count() << std::endl;
}

auto MakeParticipant(std::string_view name, const Arguments& args)
    -> Participant
{
    // Setup participant configuration and registry URI
    const std::string configString = R"(
Logging:
  Sinks:
    - Type: Stdout
      Level: Info
)";
    auto participantConfiguration = SilKit::Config::ParticipantConfigurationFromString(configString);
    auto participant = SilKit::CreateParticipant(participantConfiguration, std::string{name}, args.registryUri);

    // Create EthernetController
    auto* ethernetController = participant->CreateEthernetController("EthernetController1", "ETH1");

    // Register handlers
    ethernetController->AddFrameTransmitHandler(OnEthernetFrameTransmitted);
    ethernetController->AddFrameHandler(OnEthernetFrameReceived);

    auto* lifecycleService = participant->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated});
    auto* timeSyncService = lifecycleService->CreateTimeSyncService();

    Participant p;
    p.participant = std::move(participant);
    p.ethernetController = ethernetController;
    p.lifecycleService = lifecycleService;
    p.timeSyncService = timeSyncService;

    // Register simulation step handler
    timeSyncService->SetSimulationStepHandler(
        [&p](auto now, auto duration) {
            OnSimulationStep(p, now, duration);
        },
        std::chrono::nanoseconds{args.stepSizeNs});

    lifecycleService->SetCommunicationReadyHandler([ethernetController]() {
        ethernetController->Activate();
    });

    p.finalStateFuture = lifecycleService->StartLifecycle();

    return p;
}

int main(int argc, char** argv)
{
    SilKit::Util::CommandlineParser cmdParser;
    cmdParser.SetDescription("Bandwidth Demo: Simulate multiple participants and measure bandwidth.");
    cmdParser.Add<SilKit::Util::CommandlineParser::Option>(
        "number-of-participants", "n", "1", "[--number-of-participants <N>]",
        std::vector<std::string>{"Number of participants to create (default: 1)"});
    cmdParser.Add<SilKit::Util::CommandlineParser::Option>(
        "bandwidth", "b", "1000", "[--bandwidth <VALUE>]",
        std::vector<std::string>{"Bandwidth value to use KBit/s (default: 1000)"});
    cmdParser.Add<SilKit::Util::CommandlineParser::Option>(
        "step-size", "s", "1000000", "[--step-size <NANOSECONDS>]",
        std::vector<std::string>{"Simulation step size in nanoseconds (default: 1000000, i.e. 1ms)"});
    cmdParser.Add<SilKit::Util::CommandlineParser::Option>(
        "duration", "d", "1", "[--duration <SECONDS>]",
        std::vector<std::string>{"Simulation duration in seconds (default: 1)"});
    try {
        cmdParser.ParseArguments(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        cmdParser.PrintUsageInfo(std::cout);
        return -1;
    }
    int numberOfParticipants = std::stoi(cmdParser.Get<SilKit::Util::CommandlineParser::Option>("number-of-participants").Value());
    Arguments args;
    args.bandwidthKbits = std::stoi(cmdParser.Get<SilKit::Util::CommandlineParser::Option>("bandwidth").Value());
    args.stepSizeNs = std::stoi(cmdParser.Get<SilKit::Util::CommandlineParser::Option>("step-size").Value());
    args.durationSec = std::stoi(cmdParser.Get<SilKit::Util::CommandlineParser::Option>("duration").Value());

    auto&& config = SilKit::Config::ParticipantConfigurationFromString("{}");
    auto&& registry = SilKit::Vendor::Vector::CreateSilKitRegistry(config);
    args.registryUri = registry->StartListening("silkit://localhost:0"); // get actual listening address

    std::vector<std::string> participantNames;
    std::vector<Participant> participants;
    for (auto i = 0; i < numberOfParticipants; i++)
    {
        auto&& name = "Participant_" + std::to_string(i);
        participantNames.push_back(name);
        participants.emplace_back(MakeParticipant(name, args));
    }

    auto&& firstParticipant = participants.at(0);
    auto&& systemController =
        SilKit::Experimental::Participant::CreateSystemController(firstParticipant.participant.get());
    systemController->SetWorkflowConfiguration({participantNames});

    // Sleep for the requested duration before waiting for lifecycle completion
    std::cout << "Main thread sleeping for " << args.durationSec << " seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(args.durationSec));

    firstParticipant.lifecycleService->Stop("Test stopped.");
    for (auto& participant : participants)
    {
        participant.finalStateFuture.get();
    }

    return 0;
}
