// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <sstream>
#include <thread>
#include <numeric>
#include <algorithm>

#include "ib/IntegrationBus.hpp"
#include "ib/sim/all.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/cfg/string_utils.hpp"

#include "ib/cfg/Config.hpp"
#include "ib/cfg/ConfigBuilder.hpp"
#include "ib/extensions/CreateExtension.hpp"

using namespace ib::mw;
using namespace ib::mw::sync;
using namespace ib::cfg;
using namespace ib::sim::generic;
using namespace std::chrono_literals;

std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    const auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
    out << seconds.count() << "s";
    return out;
}

void PrintUsage(const std::string& executableName)
{
    std::cout << "Usage:" << std::endl;
    std::cout << executableName;
    std::cout << " [middleware]";
    std::cout << " [numberOfSimulations]";
    std::cout << " [simulationDuration]";
    std::cout << " [numberOfParticipants]";
    std::cout << " [messageCount]";
    std::cout << " [messageSizeInBytes]";
    std::cout << " [domainId]";
    std::cout << std::endl;
}

struct BenchmarkConfig
{
    Middleware usedMiddleware = Middleware::VAsio;
    uint32_t numberOfSimulations = 5;
    std::chrono::seconds simulationDuration = 1s;
    uint32_t numberOfParticipants = 4;
    uint32_t messageCount = 1;
    uint32_t messageSizeInBytes = 100;
    uint32_t domainId = 42;
};

bool Parse(int argc, char** argv, BenchmarkConfig& config)
{
    if (argc > 8)
    {
        std::cout << "Too many arguments!" << std::endl;
        PrintUsage(argv[0]);
        return false;
    }

    if (argc == 2)
    {
        if (std::string(argv[1]) == "--help")
        {
            PrintUsage(argv[0]);
            return false;
        }
    }

    try
    {
        switch (argc)
        {
        case 8: config.domainId = static_cast<uint32_t>(std::stoul(argv[7]));
            // [[fallthrough]]
        case 7: config.messageSizeInBytes = static_cast<uint32_t>(std::stoul(argv[6]));
            // [[fallthrough]]
        case 6: config.messageCount = static_cast<uint32_t>(std::stoul(argv[5]));
            // [[fallthrough]]
        case 5: config.numberOfParticipants = static_cast<uint32_t>(std::stoul(argv[4]));
            // [[fallthrough]]
        case 4: config.simulationDuration = std::chrono::seconds(static_cast<uint32_t>(std::stoul(argv[3])));
            // [[fallthrough]]
        case 3: config.numberOfSimulations = static_cast<uint32_t>(std::stoul(argv[2]));
            // [[fallthrough]]
        case 2: config.usedMiddleware = ib::from_string<Middleware>(argv[1]);
            break;
        default:
            std::cout << "No benchmark arguments given: using default benchmark configuration.";
            std::cout << " Specify '--help' as first argument to display usage." << std::endl << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "Error parsing arguments: " << e.what() << std::endl;
        return false;
    }

    return true;
}

bool Validate(const BenchmarkConfig& config)
{
    if (config.numberOfParticipants < 2)
    {
        std::cout << "Invalid argument: The Number of participants must be at least 2." << std::endl;
        return false;
    }

    if (config.simulationDuration < 1s)
    {
        std::cout << "Invalid argument: The simulation duration must be at least 1 second." << std::endl;
        return false;
    }

    if (config.numberOfSimulations < 1)
    {
        std::cout << "Invalid argument: The number of simulations must be at least 1." << std::endl;
        return false;
    }

    if (config.messageSizeInBytes < 1)
    {
        std::cout << "Invalid argument: The message payload size must be at least 1 byte." << std::endl;
        return false;
    }

    return true;
}

void PublishMessages(IGenericPublisher* publisher, uint32_t messageCount, uint32_t messageSizeInBytes)
{
    for (uint32_t i = 0; i < messageCount; i++)
    {
        std::vector<uint8_t> data(messageSizeInBytes, '*');
        publisher->Publish(std::move(data));
    }
}

void ReceiveMessage(IGenericSubscriber* subscriber, const std::vector<uint8_t>& data)
{
    // do nothing
}

void ParticipantStatusHandler(ISystemController* controller, const ParticipantStatus& newStatus)
{
    switch (newStatus.state)
    {
    case ParticipantState::Stopped:
        controller->Stop();
        break;

    default:
        break;
    }
}

void SystemStateHandler(ISystemController* controller, SystemState newState, const Config& ibConfig)
{
    switch (newState)
    {
    case SystemState::Idle:
        for (auto&& participant : ibConfig.simulationSetup.participants)
        {
            if (participant.name == "SyncMaster")
                continue;
            controller->Initialize(participant.id);
        }
        break;

    case SystemState::Initialized:
        controller->Run();
        break;

    case SystemState::Stopped:
        controller->Shutdown();
        break;

    default:
        break;
    }
}

auto BuildConfig(uint32_t participantCount, Middleware middleware) -> Config
{
    ConfigBuilder config("BenchmarkConfigGenerated");
    auto&& simulationSetup = config.SimulationSetup();

    auto syncType = middleware == Middleware::FastRTPS ? SyncType::DiscreteTime : SyncType::DistributedTimeQuantum;

    std::vector<ParticipantBuilder*> participants;
    for (uint32_t participantCounter = 0; participantCounter < participantCount; participantCounter++)
    {
        std::stringstream linkName;
        linkName << "LinkOfPubOfPart" << participantCounter;
        simulationSetup.AddOrGetLink(Link::Type::GenericMessage, linkName.str());

        std::stringstream participantName;
        participantName << "Participant" << participantCounter;
        auto&& participant = simulationSetup.AddParticipant(participantName.str());

        participant.AddParticipantController().WithSyncType(syncType);

        participant.ConfigureLogger()
            .WithFlushLevel(logging::Level::Critical)
            .AddSink(Sink::Type::Stdout)
            .WithLogLevel(logging::Level::Critical);

        std::stringstream publisherName;
        publisherName << "PubOfPart" << participantCounter;
        participant.AddGenericPublisher(publisherName.str()).WithLink(linkName.str());
        participants.emplace_back(&participant);
    }

    for (uint32_t participantCounter = 0; participantCounter < participantCount; participantCounter++)
    {
        for (uint32_t peerParticipantCounter = 0; peerParticipantCounter < participantCount; peerParticipantCounter++)
        {
            if (participantCounter == peerParticipantCounter)
                continue;

            std::stringstream linkName;
            linkName << "LinkOfPubOfPart" << participantCounter;

            std::stringstream subscriberName;
            subscriberName << "SubOfPart" << peerParticipantCounter << "fromPart" << participantCounter;

            participants.at(peerParticipantCounter)->AddGenericSubscriber(subscriberName.str()).WithLink(linkName.str());
        }
    }

    simulationSetup.AddParticipant("SyncMaster")
        .AsSyncMaster()
        .ConfigureLogger()
        .WithFlushLevel(logging::Level::Critical)
        .AddSink(Sink::Type::Stdout)
        .WithLogLevel(logging::Level::Critical);

    simulationSetup.ConfigureTimeSync()
        .WithLooseSyncPolicy()
        .WithTickPeriod(1ms);

    config.WithActiveMiddleware(middleware);

    return config.Build();
}

void ParticipantsThread(
    const BenchmarkConfig& benchmark,
    const Config& ibConfig,
    const Participant& participant)
{
    auto comAdapter = ib::CreateComAdapter(ibConfig, participant.name, benchmark.domainId);
    auto&& participantController = comAdapter->GetParticipantController();
   
    std::vector<IGenericPublisher*> publishers;
    for (auto& genericPublisher : participant.genericPublishers)
    {
        publishers.emplace_back(comAdapter->CreateGenericPublisher(genericPublisher.name));
    }

    std::vector<IGenericSubscriber*> subscribers;
    for (auto& genericSubscriber : participant.genericSubscribers)
    {
        auto thisSubscriber = comAdapter->CreateGenericSubscriber(genericSubscriber.name);
        thisSubscriber->SetReceiveMessageHandler(ReceiveMessage);
        subscribers.emplace_back(thisSubscriber);
    }

    const auto isVerbose = participant.id == 1;
    participantController->SetSimulationTask(
        [=, &publishers](std::chrono::nanoseconds now) {

        if (now > benchmark.simulationDuration)
        {
            participantController->Stop("Simulation done");
        }

        if (isVerbose)
        {
            const auto simulationDurationInNs = std::chrono::duration_cast<std::chrono::nanoseconds>(benchmark.simulationDuration);
            const auto durationOfOneSimulationPercentile = simulationDurationInNs / 100;

            if (now % durationOfOneSimulationPercentile < 1ms)
            {
                std::cout << ".";
                if (now > simulationDurationInNs - durationOfOneSimulationPercentile)
                    std::cout << std::endl;
            }
        }

        for (auto&& publisher : publishers)
        {
            PublishMessages(publisher, benchmark.messageCount, benchmark.messageSizeInBytes);
        }
    });

    participantController->Run();
}

/**************************************************************************************************
* Main Function
**************************************************************************************************/
int main(int argc, char** argv)
{
    BenchmarkConfig benchmark;
    if (!Parse(argc, argv, benchmark) || !Validate(benchmark))
    {
        return -1;
    }

    auto ibConfig = BuildConfig(benchmark.numberOfParticipants, benchmark.usedMiddleware);

    try
    {
        std::unique_ptr<ib::extensions::IIbRegistry> registry;
        if (benchmark.usedMiddleware == Middleware::VAsio)
        {
            registry = ib::extensions::CreateIbRegistry(ibConfig);
            registry->ProvideDomain(benchmark.domainId);
        }

        std::vector<std::chrono::nanoseconds> measuredRealDurations;
        for (uint32_t simulationRun = 1; simulationRun <= benchmark.numberOfSimulations; simulationRun++)
        {
            std::cout << "Simulation " << simulationRun << ": ";
            auto startTimestamp = std::chrono::system_clock::now();

            std::vector<std::thread> threads;
            for (auto &&participant : ibConfig.simulationSetup.participants)
            {
                if (participant.name == "SyncMaster")
                    continue;

                threads.emplace_back([=] {
                    ParticipantsThread(benchmark, ibConfig, participant);
                });
            }

            auto comAdapter = ib::CreateComAdapter(ibConfig, "SyncMaster", benchmark.domainId);
            auto controller = comAdapter->GetSystemController();
            auto monitor = comAdapter->GetSystemMonitor();

            monitor->RegisterSystemStateHandler([controller, &ibConfig](SystemState newState) {
                SystemStateHandler(controller, newState, ibConfig);
            });

            monitor->RegisterParticipantStatusHandler([controller](const ParticipantStatus& newStatus) {
                ParticipantStatusHandler(controller, newStatus);
            });

            for (auto&& thread : threads)
            {
                thread.join();
            }

            comAdapter.reset();

            auto endTimestamp = std::chrono::system_clock::now();
            measuredRealDurations.emplace_back(endTimestamp - startTimestamp);
        }

        const auto minMaxDuration = std::minmax_element(measuredRealDurations.begin(), measuredRealDurations.end());
        const auto averageDuration = std::accumulate(measuredRealDurations.begin(), measuredRealDurations.end(), 0ns) / measuredRealDurations.size();

        std::cout << std::endl;
        std::cout << "Completed simulations with the following parameters:" << std::endl;
        std::cout << "Middleware = " << benchmark.usedMiddleware << std::endl;
        std::cout << "Number of simulations = " << benchmark.numberOfSimulations << std::endl;
        std::cout << "Simulation duration = " << benchmark.simulationDuration << std::endl;
        std::cout << "Number of participants = " << benchmark.numberOfParticipants << std::endl;
        std::cout << "Message count per simulation task = " << benchmark.messageCount << std::endl;
        std::cout << "Message size in bytes = " << benchmark.messageSizeInBytes << std::endl;
        std::cout << "Domain ID = " << benchmark.domainId << std::endl;
        std::cout << std::endl;

        std::cout << "Average realtime duration: " << averageDuration << std::endl;

        if (benchmark.numberOfSimulations > 1)
        {
            std::cout << "Minimum duration: " << *minMaxDuration.first << std::endl;
            std::cout << "Maximum duration: " << *minMaxDuration.second << std::endl;

            std::cout << std::endl;
            uint32_t runNumber = 1;
            for (auto&& duration : measuredRealDurations)
            {
                std::cout << "Simulation run " << runNumber << ": " << duration << std::endl;
                runNumber++;
            }
        }
    }
    catch (const Misconfiguration& error)
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

