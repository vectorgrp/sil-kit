// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <sstream>
#include <thread>
#include <numeric>
#include <algorithm>
#include <iterator>

#include "ib/IntegrationBus.hpp"
#include "ib/sim/all.hpp"
#include "ib/mw/sync/all.hpp"

#include "ib/vendor/CreateIbRegistry.hpp"

using namespace ib::mw;
using namespace ib::mw::sync;
using namespace ib::cfg;
using namespace ib::sim::data;
using namespace std::chrono_literals;

std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    const auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
    out << seconds.count() << "s";
    return out;
}

void PrintUsage(const std::string& executableName)
{
    std::cout << "Usage:" << std::endl
        << executableName
        << " [numberOfSimulations]"
        << " [simulationDuration]"
        << " [numberOfParticipants]"
        << " [messageCount]"
        << " [messageSizeInBytes]"
        << " [domainId]"
        << std::endl 
        << "If no arguments are given default values will be used" << std::endl
        << "\t--help\tshow this message." << std::endl
        << "\t--disable-domainsockets\tDisable local domain sockets." << std::endl
        << "\t--enable-nodelay\tEnable the TCP NO_DELAY flag." << std::endl
        << "\t--domain-id\tSet domain id to NUM" << std::endl
        << "\t--message-size\tSet message size to BYTES" << std::endl
        << "\t--message-count\tSet number of messages to be send in each simulation iteration to NUM" << std::endl
        << "\t--number-participants\tSet number of simulation participants to NUM" << std::endl
        << "\t--number-simulations\tSet number of simulations to perform to NUM" << std::endl
        << "\t--simulation-duration\tSet virtual simulation duration to SECONDS" << std::endl
        ;
}

struct BenchmarkConfig
{
    uint32_t numberOfSimulations = 5;
    std::chrono::seconds simulationDuration = 1s;
    uint32_t numberOfParticipants = 4;
    uint32_t messageCount = 1;
    uint32_t messageSizeInBytes = 100;
    uint32_t domainId = 42;
    bool disableLocaldomainSockets = false;
    bool tcpNoDelay = false;
};

bool Parse(int argc, char** argv, BenchmarkConfig& config)
{
    // skip argv[0] and collect all arguments
    std::vector<std::string> args;
    std::copy((argv + 1), (argv + argc), std::back_inserter(args));

    auto asNum = [](const auto& str) {
        return static_cast<uint32_t>(std::stoul(str));
    };

    // test and remove the flag from args, returns true if flag was present
    auto consumeFlag = [&args](const auto& namedOption) {
        auto it = std::find(args.begin(), args.end(), namedOption);
        if (it != args.end())
        {
            args.erase(it);
            return true;
        }
        return false;
    };

    if (consumeFlag("--disable-domainsockets"))
    {
        config.disableLocaldomainSockets = true;
    }
    if (consumeFlag("--enable-nodelay"))
    {
        config.tcpNoDelay = true;
    }

    if (consumeFlag("--help"))
    {
        PrintUsage(argv[0]);
        return false;
    }

    // Some more human-readable shortcuts for the options.
    // Consume a named option and return its argument,
    // or throw if an invalid argument is given.
    bool haveUserOptions = false;
    auto getArg = [&args, &haveUserOptions](const auto& name) {
        auto argIt = std::find(args.begin(), args.end(), name);
        if (argIt == args.end())
        {
            return std::string{}; //the argument is not even mentioned
        }
        auto valIt = argIt+1;
        if (valIt == args.end())
        {
            throw std::runtime_error{
                std::string{"Option \""}
                + name
                + "\" is missing an argument!" 
            };
        }
        // remove consumed args
        auto result = *valIt;
        args.erase(valIt);
        args.erase(argIt);
        haveUserOptions = true;
        return result;
    };
    auto parseOptional = [ &getArg](const auto& argName, auto& outputValue, auto conversionFunc) {
        auto arg = getArg( argName);
        if (!arg.empty())
        {
            try
            {
                using OutputT = std::remove_reference_t<decltype(outputValue)>;
                outputValue = OutputT{ conversionFunc(arg) };
            }
            catch (const std::exception& ex)
            {
                std::cout << "Error: cannot parse argument of \"" << argName << "\": " << ex.what() <<std::endl;
                std::cout << std::flush;
                throw;
            }
        }
    };
    // Parse and consume the optional named arguments
    parseOptional("--domain-id", config.domainId, asNum);
    parseOptional("--message-size", config.messageSizeInBytes, asNum);
    parseOptional("--message-count", config.messageCount, asNum);
    parseOptional("--number-participants", config.numberOfParticipants, asNum);
    parseOptional("--number-simulations", config.numberOfSimulations, asNum);
    parseOptional("--simulation-duration", config.simulationDuration, asNum);

    //check unknown long options
    for (const auto& arg : args) {
        if (arg.find_first_of("--") == 0) {
            std::cout << "Error: unknown argument \"" << arg << "\"" << std::endl;
            PrintUsage(argv[0]);
            return false;
        }
    }
    // Handle positional arguments, if any:
    if (args.size() > 7)
    {
        std::cout << "Error: Too many arguments!" << std::endl;
        PrintUsage(argv[0]);
        return false;
    }

    try
    {
        switch (args.size())
        {
        case 6: config.domainId = asNum(args.at(6));
            // [[fallthrough]]
        case 5: config.messageSizeInBytes = asNum(args.at(5));
            // [[fallthrough]]
        case 4: config.messageCount = asNum(args.at(4));
            // [[fallthrough]]
        case 3: config.numberOfParticipants = asNum(args.at(3));
            // [[fallthrough]]
        case 2: config.simulationDuration = std::chrono::seconds(asNum(args.at(2)));
            // [[fallthrough]]
        case 1: config.numberOfSimulations = asNum(args.at(1));
            break;
        default:
            if (haveUserOptions)
            {
                std::cout << "Using user specified configuration to override defaults" << std::endl;
            }
            else
            {
                std::cout << "No benchmark arguments given: using default benchmark configuration.";
                std::cout << " Specify '--help' as first argument to display usage." << std::endl << std::endl;
            }
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

void PublishMessages(IDataPublisher* publisher, uint32_t messageCount, uint32_t messageSizeInBytes)
{
    for (uint32_t i = 0; i < messageCount; i++)
    {
        std::vector<uint8_t> data(messageSizeInBytes, '*');
        publisher->Publish(std::move(data));
    }
}

void ReceiveMessage(IDataSubscriber* /*subscriber*/, const std::vector<uint8_t>& /*data*/)
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

void SystemStateHandler(ISystemController* controller, SystemState newState, const std::vector<std::string>& expectedParticipants)
{
    switch (newState)
    {
    case SystemState::ReadyToRun:
        controller->Run();
        break;

    case SystemState::Stopped:
        for (auto&& name : expectedParticipants)
        {
            controller->Shutdown(name);
        }
        break;

    default:
        break;
    }
}

void ParticipantsThread(
    std::shared_ptr<ib::cfg::IParticipantConfiguration> ibConfig,
    const BenchmarkConfig& benchmark,
    const std::string& participantName,
    uint32_t participantIndex,
    size_t& messageCounter)
{
    auto participant = ib::CreateParticipant(ibConfig, participantName, benchmark.domainId);
    auto* lifecycleService = participant->GetLifecycleService();
    auto* timeSyncService = lifecycleService->GetTimeSyncService();
   
   auto publisher = participant->CreateDataPublisher("PubCtrl1", "Topic", {}, {}, 0);
   participant->CreateDataSubscriber("SubCtrl1", "Topic", {}, {}, [&messageCounter](auto*, auto&) {
        // this is handled in I/O thread, so no data races on counter.
        messageCounter++;
    });

    const auto isVerbose = participantIndex == 0;
   timeSyncService->SetSimulationTask(
        [=, &publisher](std::chrono::nanoseconds now) {

        if (now > benchmark.simulationDuration)
        {
            lifecycleService->Stop("Simulation done");
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
        PublishMessages(publisher, benchmark.messageCount, benchmark.messageSizeInBytes);
    });

    lifecycleService->StartLifecycleWithSyncTime(timeSyncService, {true, true});
}

/**************************************************************************************************
* Main Function
**************************************************************************************************/
int main(int argc, char** argv)
{
    auto participantConfiguration = ib::cfg::ParticipantConfigurationFromString("{}");
    BenchmarkConfig benchmark;
    if (!Parse(argc, argv, benchmark) || !Validate(benchmark))
    {
        return -1;
    }

    if (benchmark.disableLocaldomainSockets)
    {
        //ibConfig.middlewareConfig.vasio.enableDomainSockets = false;
    }

    if (benchmark.tcpNoDelay)
    {
        //ibConfig.middlewareConfig.vasio.tcpNoDelay = true;
        //ibConfig.middlewareConfig.vasio.tcpQuickAck = true;
    }

    try
    {
        std::unique_ptr<ib::vendor::IIbRegistry> registry;
        // TODO use new config
        registry = ib::vendor::CreateIbRegistry(ib::cfg::ParticipantConfigurationFromString("{}"));
        registry->ProvideDomain(benchmark.domainId);

        std::vector<size_t> messageCounts;
        std::vector<std::chrono::nanoseconds> measuredRealDurations;

        for (uint32_t simulationRun = 1; simulationRun <= benchmark.numberOfSimulations; simulationRun++)
        {
            std::vector<size_t> counters(benchmark.numberOfParticipants, 0);
            std::cout << "Simulation " << simulationRun << ": ";
            auto startTimestamp = std::chrono::system_clock::now();

            std::vector<std::string> participantNames;
            std::vector<std::thread> threads;
            size_t idx = 0;
            for (uint32_t participantIndex = 0; participantIndex < benchmark.numberOfParticipants; participantIndex++)
            {
                std::string participantName = "Participant" + std::to_string(participantIndex);
                participantNames.push_back(participantName);
                auto& counter = counters.at(idx);
                idx++;
                threads.emplace_back(&ParticipantsThread, participantConfiguration, benchmark,  participantName, participantIndex, std::ref(counter));
            }

            auto participant = ib::CreateParticipant(participantConfiguration, "SystemController", benchmark.domainId);
            auto controller = participant->GetSystemController();
            auto monitor = participant->GetSystemMonitor();

            controller->SetWorkflowConfiguration({participantNames});

            monitor->AddSystemStateHandler([controller, participantNames](SystemState newState) {
                SystemStateHandler(controller, newState, participantNames);
            });

            monitor->AddParticipantStatusHandler([controller](const ParticipantStatus& newStatus) {
                ParticipantStatusHandler(controller, newStatus);
            });

            for (auto&& thread : threads)
            {
                thread.join();
            }

            participant.reset();

            auto endTimestamp = std::chrono::system_clock::now();
            measuredRealDurations.emplace_back(endTimestamp - startTimestamp);
            auto totalCount = std::accumulate(counters.begin(), counters.end(), size_t{0});
            messageCounts.emplace_back(totalCount);
        }

        const auto minMaxDuration = std::minmax_element(measuredRealDurations.begin(), measuredRealDurations.end());
        const auto averageDuration = std::accumulate(measuredRealDurations.begin(), measuredRealDurations.end(), 0ns) / measuredRealDurations.size();
        const auto averageNumberMessages = std::accumulate(messageCounts.begin(), messageCounts.end(), size_t{ 0 }) / messageCounts.size();

        std::cout << std::endl;
        std::cout << "Completed simulations with the following parameters:" << std::endl;
        std::cout << "Number of simulations = " << benchmark.numberOfSimulations << std::endl;
        std::cout << "Simulation duration = " << benchmark.simulationDuration << std::endl;
        std::cout << "Number of participants = " << benchmark.numberOfParticipants << std::endl;
        std::cout << "Message count per simulation task = " << benchmark.messageCount << std::endl;
        std::cout << "Message size in bytes = " << benchmark.messageSizeInBytes << std::endl;
        std::cout << "Domain ID = " << benchmark.domainId << std::endl;
        std::cout << std::endl;

        std::cout << "Average realtime duration: " << averageDuration << std::endl;
        std::cout << "Average number of messages: " << averageNumberMessages << std::endl;

        if (benchmark.numberOfSimulations > 1)
        {
            std::cout << "Minimum duration: " << *minMaxDuration.first << std::endl;
            std::cout << "Maximum duration: " << *minMaxDuration.second << std::endl;

            std::cout << std::endl;
            uint32_t runNumber = 1;
            for (auto&& duration : measuredRealDurations)
            {
                std::cout << "Simulation run " << runNumber << ": " << duration << std::endl;
                std::cout << "Simulation run " << runNumber << ": number of messages: " << messageCounts.at(runNumber-1) << std::endl;
                runNumber++;
            }
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

