/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <iostream>
#include <iomanip>
#include <sstream>
#include <thread>
#include <numeric>
#include <algorithm>
#include <iterator>
#include <cmath>
#include <fstream>

#include "silkit/SilKit.hpp"
#include "silkit/SilKitVersion.hpp"
#include "silkit/services/all.hpp"
#include "silkit/services/orchestration/all.hpp"

#include "silkit/vendor/CreateSilKitRegistry.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"

using namespace SilKit::Services::Orchestration;
using namespace SilKit::Config;
using namespace SilKit::Services::PubSub;
using namespace SilKit::Services::Ethernet;
using namespace SilKit::Services::Can;
using namespace std::chrono_literals;

std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    const auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
    out << seconds.count() << "s";
    return out;
}

void PrintUsage(const std::string& executableName)
{
    std::cout
        << "Usage:" << std::endl
        << executableName << " [numberOfSimulationRuns]"
        << " [simulationDuration]"
        << " [simulationStepSize]"
        << " [service]"
        << " [numberOfParticipants]"
        << " [messageCount]"
        << " [messageSizeInBytes]"
        << " [registryURi]" << std::endl
        << "If no arguments are given, default values will be used." << std::endl
        << "\t--help\tshow this message." << std::endl
        << "\t--registry-uri\tThe URI of the registry to start. Default: silkit://localhost:0" << std::endl
        << "\t--message-size\tSets the message size to BYTES. Default: 1000" << std::endl
        << "\t--message-count\tSets the number of messages to be send per participant in each simulation step to "
           "NUM. Default: 50"
        << std::endl
        << "\t--number-participants\tSets the number of simulation participants to NUM. Default: 2" << std::endl
        << "\t--number-simulation-runs\tSets the number of simulation runs to perform to NUM. Default: 4" << std::endl
        << "\t--simulation-step-size\tSets the simulation step size (virtual time) to MILLISECONDS. Default: 1ms"
        << std::endl
        << "\t--simulation-duration\tSets the simulation duration (virtual time) to SECONDS. Default: 1s" << std::endl
        << "\t--service\tSets the service type (pubsub, can or ethernet). Default: pubsub" << std::endl
        << "\t--configuration\tPath and filename of the participant configuration YAML or JSON file. Default: empty"
        << std::endl
        << "\t--write-csv\tPath and filename of csv file with benchmark results. Default: empty" << std::endl;
}

enum class ServiceType
{
    PubSub,
    Can,
    Ethernet
};

std::ostream& operator<<(std::ostream& outStream, const ServiceType& service)
{
    switch (service)
    {
    case ServiceType::PubSub:
        outStream << "PubSub";
        break;
    case ServiceType::Can:
        outStream << "CAN";
        break;
    case ServiceType::Ethernet:
        outStream << "Ethernet";
        break;
    default:
        outStream << "Invalid Service Type";
    }
    return outStream;
}

std::string to_string(const ServiceType& service)
{
    std::stringstream outStream;
    outStream << service;
    return outStream.str();
}

struct BenchmarkConfig
{
    uint32_t numberOfSimulationRuns = 4;
    std::chrono::seconds simulationDuration = 1s;
    std::chrono::milliseconds simulationStepSize = 1ms;
    uint32_t numberOfParticipants = 2;
    uint32_t messageCount = 50;
    uint32_t messageSizeInBytes = 1000;
    std::string registryUri = "silkit://localhost:0";
    std::string silKitConfigPath = "";
    std::string writeCsv = "";
    ServiceType service = ServiceType::PubSub;
};

bool Parse(int argc, char** argv, BenchmarkConfig& config)
{
    // skip argv[0] and collect all arguments
    std::vector<std::string> args;
    std::copy((argv + 1), (argv + argc), std::back_inserter(args));

    auto asNum = [](const auto& str) { return static_cast<uint32_t>(std::stoul(str)); };
    auto asStr = [](auto& a) { return std::string{a}; };
    auto asServiceType = [](const auto& a) {
        auto arg = std::string{a};

        if (arg == "pubsub")
        {
            return ServiceType::PubSub;
        }
        else if (arg == "ethernet")
        {
            return ServiceType::Ethernet;
        }
        else if (arg == "can")
        {
            return ServiceType::Can;
        }
        else
        {
            throw std::runtime_error{"Invalid argument \"" + arg + "\"!"};
        }
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
        auto valIt = argIt + 1;
        if (valIt == args.end())
        {
            throw std::runtime_error{std::string{"Option \""} + name + "\" is missing an argument!"};
        }
        // remove consumed args
        auto result = *valIt;
        args.erase(valIt);
        args.erase(argIt);
        haveUserOptions = true;
        return result;
    };
    auto parseOptional = [&getArg](const auto& argName, auto& outputValue, auto conversionFunc) {
        auto arg = getArg(argName);
        if (!arg.empty())
        {
            try
            {
                using OutputT = std::remove_reference_t<decltype(outputValue)>;
                outputValue = OutputT{conversionFunc(arg)};
            }
            catch (const std::exception& ex)
            {
                std::cout << "Error: cannot parse argument of \"" << argName << "\": " << ex.what() << std::endl;
                std::cout << std::flush;
                throw;
            }
        }
    };
    // Parse and consume the optional named arguments
    parseOptional("--registry-uri", config.registryUri, asStr);
    parseOptional("--message-size", config.messageSizeInBytes, asNum);
    parseOptional("--message-count", config.messageCount, asNum);
    parseOptional("--number-participants", config.numberOfParticipants, asNum);
    parseOptional("--number-simulation-runs", config.numberOfSimulationRuns, asNum);
    parseOptional("--simulation-step-size", config.simulationStepSize, asNum);
    parseOptional("--simulation-duration", config.simulationDuration, asNum);
    parseOptional("--service", config.service, asServiceType);
    parseOptional("--configuration", config.silKitConfigPath, asStr);
    parseOptional("--write-csv", config.writeCsv, asStr);

    //check unknown long options
    for (const auto& arg : args)
    {
        if (arg.find_first_of("--") == 0)
        {
            std::cout << "Error: unknown argument \"" << arg << "\"" << std::endl;
            PrintUsage(argv[0]);
            return false;
        }
    }
    // Handle positional arguments, if any:
    if (args.size() > 8)
    {
        std::cout << "Error: Too many arguments!" << std::endl;
        PrintUsage(argv[0]);
        return false;
    }

    try
    {
        switch (args.size())
        {
        case 8:
            config.registryUri = args.at(7);
            // [[fallthrough]]
        case 7:
            config.messageSizeInBytes = asNum(args.at(6));
            // [[fallthrough]]
        case 6:
            config.messageCount = asNum(args.at(5));
            // [[fallthrough]]
        case 5:
            config.numberOfParticipants = asNum(args.at(4));
            // [[fallthrough]]
        case 4:
            config.service = asServiceType(args.at(3));
            // [[fallthrough]]
        case 3:
            config.simulationStepSize = std::chrono::milliseconds(asNum(args.at(2)));
            // [[fallthrough]]
        case 2:
            config.simulationDuration = std::chrono::seconds(asNum(args.at(1)));
            // [[fallthrough]]
        case 1:
            config.numberOfSimulationRuns = asNum(args.at(0));
            break;
        default:
            if (haveUserOptions)
            {
                std::cout << std::endl << "Using user specified configuration to override defaults." << std::endl;
            }
            else
            {
                std::cout << std::endl << "No benchmark arguments given: using default benchmark configuration.";
                std::cout << std::endl << " Specify '--help' as first argument to display usage." << std::endl;
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
        std::cout << "Invalid argument: The simulation duration (virtual time) must be at least 1 second." << std::endl;
        return false;
    }

    if (config.simulationDuration < config.simulationStepSize)
    {
        std::cout << "Invalid argument: The simulation duration (virtual time) must be greater or equal the simulation "
                     "step size."
                  << std::endl;
        return false;
    }

    if (config.numberOfSimulationRuns < 1)
    {
        std::cout << "Invalid argument: The number of simulations runs must be at least 1." << std::endl;
        return false;
    }

    if (config.messageSizeInBytes < 1)
    {
        std::cout << "Invalid argument: The message payload size must be at least 1 byte." << std::endl;
        return false;
    }

    return true;
}

void CheckAndTruncateMessageSize(BenchmarkConfig& benchmark)
{
    const uint32_t maxSizeEth = 1500;
    const uint32_t maxSizeCan = 8;

    if (benchmark.service == ServiceType::Ethernet && benchmark.messageSizeInBytes > maxSizeEth)
    {
        benchmark.messageSizeInBytes = maxSizeEth;
        std::cout << std::endl
                  << "CAUTION: Ethernet frame size is truncated to " << benchmark.messageSizeInBytes << " Byte."
                  << std::endl;
    }
    else if (benchmark.service == ServiceType::Can && benchmark.messageSizeInBytes > maxSizeCan)
    {
        benchmark.messageSizeInBytes = maxSizeCan;
        std::cout << std::endl
                  << "CAUTION: CAN frame payload size is truncated to " << benchmark.messageSizeInBytes << " Byte."
                  << std::endl;
    }
}

void PublishMessages(IDataPublisher* publisher, uint32_t messageCount, uint32_t messageSizeInBytes)
{
    std::vector<uint8_t> data(messageSizeInBytes, '*');
    for (uint32_t i = 0; i < messageCount; i++)
    {
        publisher->Publish(SilKit::Util::ToSpan(data));
    }
}

void SendEthernetFrames(IEthernetController* ethernetController, uint32_t messageCount, uint32_t messageSizeInBytes)
{
    std::vector<uint8_t> frameData(messageSizeInBytes, '*');
    for (uint32_t i = 0; i < messageCount; i++)
    {
        ethernetController->SendFrame(EthernetFrame{SilKit::Util::ToSpan(frameData)});
    }
}

void SendCanFrames(ICanController* canController, uint32_t messageCount, uint32_t messageSizeInBytes)
{
    std::vector<uint8_t> frameData(messageSizeInBytes, '*');
    for (uint32_t i = 0; i < messageCount; i++)
    {
        CanFrame canFrame{};
        canFrame.dataField = SilKit::Util::ToSpan(frameData);

        canController->SendFrame(std::move(canFrame));
    }
}

void ParticipantsThread(std::shared_ptr<SilKit::Config::IParticipantConfiguration> config,
                        const BenchmarkConfig& benchmark, const std::string& participantName, uint32_t participantIndex)
{
    auto participant = SilKit::CreateParticipant(config, participantName, benchmark.registryUri);
    auto* lifecycleService = participant->CreateLifecycleService({OperationMode::Coordinated});
    auto* timeSyncService = lifecycleService->CreateTimeSyncService();

    IDataPublisher* publisher = nullptr;
    IEthernetController* ethernetController = nullptr;
    ICanController* canController = nullptr;

    switch (benchmark.service)
    {
    case ServiceType::PubSub:
    {
        SilKit::Services::PubSub::PubSubSpec dataSpec{"Topic", {}};
        SilKit::Services::PubSub::PubSubSpec matchingDataSpec{"Topic", {}};
        publisher = participant->CreateDataPublisher("PubCtrl1", dataSpec, 0);
        participant->CreateDataSubscriber("SubCtrl1", matchingDataSpec, [](auto*, auto&) {});
        break;
    }
    case ServiceType::Ethernet:
    {
        ethernetController = participant->CreateEthernetController("Eth1", "Eth1");
        ethernetController->AddFrameHandler([](auto*, auto&) {});
        lifecycleService->SetCommunicationReadyHandler([ethernetController]() { ethernetController->Activate(); });
        break;
    }
    case ServiceType::Can:
    {
        canController = participant->CreateCanController("CAN1", "CAN1");
        canController->AddFrameHandler([](auto*, auto&) {});
        lifecycleService->SetCommunicationReadyHandler([canController]() {
            canController->SetBaudRate(10'000, 1'000'000, 2'000'000);
            canController->Start();
        });
        break;
    }
    }

    const auto isVerbose = participantIndex == 0;
    timeSyncService->SetSimulationStepHandler([=](std::chrono::nanoseconds now, const auto /*duration*/) {
        if (now == benchmark.simulationDuration)
        {
            lifecycleService->Stop("Simulation done");
            return;
        }

        if (isVerbose)
        {
            const auto simulationDurationInNs =
                std::chrono::duration_cast<std::chrono::nanoseconds>(benchmark.simulationDuration);
            const auto durationOfOneSimulationPercentile = simulationDurationInNs / 20;

            if (now % durationOfOneSimulationPercentile < benchmark.simulationStepSize)
            {
                std::cout << ".";
            }
        }

        switch (benchmark.service)
        {
        case ServiceType::PubSub:
        {
            PublishMessages(publisher, benchmark.messageCount, benchmark.messageSizeInBytes);
            break;
        }
        case ServiceType::Ethernet:
        {
            SendEthernetFrames(ethernetController, benchmark.messageCount, benchmark.messageSizeInBytes);
            break;
        }
        case ServiceType::Can:
        {
            SendCanFrames(canController, benchmark.messageCount, benchmark.messageSizeInBytes);
            break;
        }
        }
    }, benchmark.simulationStepSize);

    auto lifecycleFuture = lifecycleService->StartLifecycle();
    lifecycleFuture.get();
}

void PrintParameters(BenchmarkConfig benchmark)
{
#ifndef NDEBUG
    std::cout << "WARNING: The benchmark demo is executed in a DEBUG build configuration." << std::endl
              << "For more reliable timings, please use a RELEASE build configuration" << std::endl
              << "of the SIL Kit library and the benchmark demo." << std::endl;
    std::this_thread::sleep_for(2s);
#endif

    std::cout << std::endl
              << "This benchmark demo produces timings of a configurable simulation setup." << std::endl
              << "<N> participants exchange <M> messages of <B> bytes per simulation step" << std::endl
              << "with a period of <T> ms and run for <S> seconds (virtual time)." << std::endl
              << "This simulation run is repeated <K> times and averages over all runs are calculated." << std::endl
              << "The demo can be run using PubSub, CAN or Ethernet controllers for the message exchange," << std::endl
              << "where each participant broadcasts the messages to all other participants." << std::endl
              << std::endl
              << "Running simulations with the following parameters:" << std::endl
              << std::endl
              << std::left << std::setw(39) << "- Communication service type: " << benchmark.service << std::endl
              << std::left << std::setw(39) << "- Number of simulation runs: " << benchmark.numberOfSimulationRuns
              << std::endl
              << std::left << std::setw(39) << "- Simulation step size (virtual time): " << benchmark.simulationStepSize
              << std::endl
              << std::left << std::setw(39) << "- Simulation duration (virtual time): " << benchmark.simulationDuration
              << std::endl
              << std::left << std::setw(39) << "- Number of participants: " << benchmark.numberOfParticipants
              << std::endl
              << std::left << std::setw(39) << "- Messages per simulation step: " << benchmark.messageCount << std::endl
              << std::left << std::setw(39) << "- Message size (bytes): " << benchmark.messageSizeInBytes << std::endl
              << std::left << std::setw(39) << "- Registry URI: " << benchmark.registryUri << std::endl
              << std::left << std::setw(39) << "- Configuration: " << benchmark.silKitConfigPath << std::endl
              << std::left << std::setw(39) << "- CSV output: " << benchmark.writeCsv << std::endl;
}

template <typename T>
std::pair<T, T> mean_and_error(const std::vector<T>& vec)
{
    const size_t sz = vec.size();
    if (sz == 1)
    {
        return std::make_pair(vec[0], 0.0);
    }

    const T mean = std::accumulate(vec.begin(), vec.end(), 0.0) / sz;
    auto variance_func = [&mean, &sz](T accumulator, const T& val) {
        return accumulator + ((val - mean) * (val - mean) / (sz - 1));
    };

    return std::make_pair(mean, std::sqrt(std::accumulate(vec.begin(), vec.end(), 0.0, variance_func)));
}

/**************************************************************************************************
* Main Function
**************************************************************************************************/
int main(int argc, char** argv)
{
    std::cout.precision(3);
    BenchmarkConfig benchmark;
    if (!Parse(argc, argv, benchmark) || !Validate(benchmark))
    {
        return -1;
    }

    CheckAndTruncateMessageSize(benchmark);

    PrintParameters(benchmark);

    try
    {
        std::shared_ptr<SilKit::Config::IParticipantConfiguration> config;
        if (benchmark.silKitConfigPath == "")
        {
            config = SilKit::Config::ParticipantConfigurationFromString("{}");
        }
        else
        {
            config = SilKit::Config::ParticipantConfigurationFromFile(benchmark.silKitConfigPath);
        }

        std::unique_ptr<SilKit::Vendor::Vector::ISilKitRegistry> registry =
            SilKit::Vendor::Vector::CreateSilKitRegistry(config);
        benchmark.registryUri = registry->StartListening(benchmark.registryUri);

        std::vector<std::chrono::nanoseconds> measuredRealDurations;

        for (uint32_t simulationRun = 1; simulationRun <= benchmark.numberOfSimulationRuns; simulationRun++)
        {
            std::cout << "> Simulation " << simulationRun << ": ";
            auto startTimestamp = std::chrono::high_resolution_clock::now();

            std::vector<std::string> participantNames;
            std::vector<std::thread> threads;
            for (uint32_t participantIndex = 0; participantIndex < benchmark.numberOfParticipants; participantIndex++)
            {
                std::string participantName = "Participant" + std::to_string(participantIndex);
                participantNames.push_back(participantName);
                threads.emplace_back(&ParticipantsThread, config, benchmark, participantName, participantIndex);
            }

            const auto systemControllerName = "SystemController";
            participantNames.push_back(systemControllerName);
            auto systemControllerParticipant =
                SilKit::CreateParticipant(config, systemControllerName, benchmark.registryUri);
            auto systemController =
                SilKit::Experimental::Participant::CreateSystemController(systemControllerParticipant.get());
            systemController->SetWorkflowConfiguration({participantNames});
            auto lifecycleService = systemControllerParticipant->CreateLifecycleService({OperationMode::Coordinated});
            auto lifecycleFuture = lifecycleService->StartLifecycle();
            lifecycleFuture.get();

            for (auto&& thread : threads)
            {
                thread.join();
            }

            systemControllerParticipant.reset();

            auto endTimestamp = std::chrono::high_resolution_clock::now();
            measuredRealDurations.emplace_back(endTimestamp - startTimestamp);
            std::cout << "  " << measuredRealDurations.back() << std::endl;
        }

        std::vector<double> measuredRealDurationsSeconds(measuredRealDurations.size());
        std::transform(measuredRealDurations.begin(), measuredRealDurations.end(), measuredRealDurationsSeconds.begin(),
                       [](auto d) { return static_cast<double>(d.count() / 1e9); });

        const auto averageDuration = mean_and_error(measuredRealDurationsSeconds);

        const auto virtualDurationMilliseconds =
            std::chrono::duration_cast<std::chrono::milliseconds>(benchmark.simulationDuration);
        const auto numberSimulationSteps =
            static_cast<size_t>(virtualDurationMilliseconds.count() / benchmark.simulationStepSize.count());
        // fully connected network topology
        const size_t numberMessages = numberSimulationSteps * benchmark.messageCount * benchmark.numberOfParticipants
                                      * (benchmark.numberOfParticipants - 1);

        // Reoccuring factor from error propagation
        const auto sigmaDurOverDurSqr = averageDuration.second / averageDuration.first / averageDuration.first;

        // Byte throughput mean and error (Byte to Mebi and ns to s)
        const auto throughputPrefac = numberMessages * benchmark.messageSizeInBytes / 1024.0 / 1024.0;
        const auto averageThroughput =
            std::make_pair(throughputPrefac / averageDuration.first, throughputPrefac * sigmaDurOverDurSqr);

        // Message rate mean and error
        const auto averageMsgRate =
            std::make_pair(numberMessages / averageDuration.first, numberMessages * sigmaDurOverDurSqr);

        // Speedup mean and error
        const auto averageSpeedup = std::make_pair(benchmark.simulationDuration.count() / averageDuration.first,
                                                   benchmark.simulationDuration.count() * sigmaDurOverDurSqr);

        if (benchmark.numberOfSimulationRuns > 1)
        {
            std::cout << std::endl << "Averages over all simulation runs:" << std::endl << std::endl;
        }
        else
        {
            std::cout << std::endl << "Result of the simulation run:" << std::endl << std::endl;
        }

        // Stream helper to combine value and unit to use it with std::setw as a whole
        std::ostringstream averageDurationWithUnit;
        averageDurationWithUnit.precision(3);
        averageDurationWithUnit << averageDuration.first << "s";

        std::ostringstream averageThroughputWithUnit;
        averageThroughputWithUnit.precision(3);
        averageThroughputWithUnit << averageThroughput.first << " MiB/s";

        std::ostringstream averageMsgRateWithUnit;
        averageMsgRateWithUnit << static_cast<int>(averageMsgRate.first) << " 1/s";

        std::cout << std::setw(39) << "- Realtime duration (runtime): " << std::setw(13)
                  << averageDurationWithUnit.str() << " +/- " << averageDuration.second << "s" << std::endl

                  << std::setw(39) << "- Speedup (virtual time/runtime): " << std::setw(13) << averageSpeedup.first
                  << " +/- " << averageSpeedup.second << std::endl

                  << std::setw(39) << "- Throughput (data size/runtime): " << std::setw(13)
                  << averageThroughputWithUnit.str() << " +/- " << averageThroughput.second << " MiB/s" << std::endl

                  << std::setw(39) << "- Message rate (count/runtime): " << std::setw(13)
                  << averageMsgRateWithUnit.str() << " +/- " << static_cast<int>(averageMsgRate.second) << " 1/s"
                  << std::endl

                  << std::left << std::setw(39) << "- Total number of messages: " << numberMessages << std::endl

                  << std::endl
                  << std::endl;

        if (benchmark.writeCsv != "")
        {
            std::stringstream csvHeader;
            csvHeader << "# SilKitBenchmarkDemo, SIL Kit Version " << SilKit::Version::String();
            const auto csvColumns = "numRuns; participants; messageSize; messageCount; duration(virtual time, s); "
                                    "stepSize(virtual time, ms); "
                                    "numberMessageSent; runtime(s); runtime_err; throughput(MiB/s); "
                                    "throughput_err; speedup; speedup_err; messageRate(1/s); messageRate_err";
            std::fstream csvFile;
            csvFile.open(benchmark.writeCsv, std::ios_base::in | std::ios_base::out); // Try to open
            bool csvValid{true};
            if (!csvFile.is_open())
            {
                // File doesn't exist, create new file and write header
                csvFile.open(benchmark.writeCsv, std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
                csvFile << csvHeader.str() << std::endl;
                csvFile << csvColumns << std::endl;
            }
            else
            {
                // File is there, check if header is valid
                std::string header;
                std::getline(csvFile, header);
                csvFile.clear();
                if (header != csvHeader.str())
                {
                    std::cerr << "Invalid header in file \"" << benchmark.writeCsv << "\"." << std::endl;
                    csvValid = false;
                }
            }
            if (csvValid)
            {
                // Append data
                csvFile.seekp(0, std::ios_base::end);
                csvFile << benchmark.numberOfSimulationRuns << ";" << benchmark.numberOfParticipants << ";"
                        << benchmark.messageSizeInBytes << ";" << benchmark.messageCount << ";"
                        << benchmark.simulationDuration.count() << ";" << benchmark.simulationStepSize.count() << ";"
                        << numberMessages << ";" << averageDuration.first << ";" << averageDuration.second << ";"
                        << averageThroughput.first << ";" << averageThroughput.second << ";" << averageSpeedup.first
                        << ";" << averageSpeedup.second << ";" << averageMsgRate.first << ";" << averageMsgRate.second
                        << std::endl;
            }
            csvFile.close();
        }
    }
    catch (const SilKit::ConfigurationError& error)
    {
        std::cerr << "Invalid configuration: " << error.what() << std::endl;
        std::cout << "Press enter to end the process..." << std::endl;
        std::cin.ignore();
        return -2;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        std::cout << "Press enter to end the process..." << std::endl;
        std::cin.ignore();
        return -3;
    }

    return 0;
}
