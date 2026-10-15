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
#include <functional>
#include <limits>
#include <random>

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

static std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    const auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
    out << seconds.count() << "s";
    return out;
}

static void PrintUsage(const std::string& executableName)
{
    std::cout
        << "Usage:" << std::endl
        << executableName << " [numberOfSimulationRuns]" << " [simulationDuration]" << " [simulationStepSize]"
        << " [service]" << " [numberOfParticipants]" << " [messageCount]" << " [messageSizeInBytes]" << " [registryURi]"
        << std::endl
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
        << "\t--simulation-step-size-us\tSets the simulation step size (virtual time) to MICROSECONDS instead."
        << std::endl
        << "\t--simulation-duration\tSets the simulation duration (virtual time) to SECONDS. Default: 1s" << std::endl
        << "\t--compute-us\tBusy-waits MICROSECONDS in every simulation step to emulate computation. Default: 0"
        << std::endl
        << "\t--compute-jitter-us\tAdds a random busy-wait of up to MICROSECONDS to every simulation step. Default: 0"
        << std::endl
        << "\t--quiet-participants\tThe last NUM participants send no messages. Default: 0" << std::endl
        << "\t--sync-mode\tTime synchronization of all participants: lockstep (classic), max-run-ahead or exact."
        << " Default: as in the configuration file" << std::endl
        << "\t--latency\tWith --sync-mode exact: how long after sending messages are delivered, in (fractional)"
        << " MILLISECONDS. Default: 5" << std::endl
        << "\t--max-run-ahead\tWith --sync-mode max-run-ahead: how far participants may run ahead, in (fractional)"
        << " MILLISECONDS. Default: 5" << std::endl
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

//! The time synchronization algorithm all participants use
enum class SyncMode
{
    AsConfigured, //!< whatever the configuration file says (lockstep by default)
    Lockstep,     //!< classic: no participant starts a step before all others announced they reached it
    MaxRunAhead,  //!< bounded run-ahead: participants may run up to MaxRunAhead ahead of the others
    Exact         //!< exact mode: messages are delivered Latency after they were sent, participants run ahead by it
};

static std::ostream& operator<<(std::ostream& outStream, const ServiceType& service)
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

struct BenchmarkConfig
{
    uint32_t numberOfSimulationRuns = 4;
    std::chrono::seconds simulationDuration = 1s;
    std::chrono::nanoseconds simulationStepSize = 1ms;
    uint32_t numberOfParticipants = 2;
    uint32_t messageCount = 50;
    uint32_t messageSizeInBytes = 1000;
    std::string registryUri = "silkit://localhost:0";
    std::string silKitConfigPath = "";
    std::string writeCsv = "";
    ServiceType service = ServiceType::PubSub;
    std::chrono::microseconds computeTime = 0us;
    std::chrono::microseconds computeJitter = 0us;
    uint32_t quietParticipants = 0;
    SyncMode syncMode = SyncMode::AsConfigured;
    double maxRunAheadMs = std::numeric_limits<double>::quiet_NaN(); // NaN: not given
    double latencyMs = std::numeric_limits<double>::quiet_NaN();     // NaN: not given
};

//! The time synchronization settings the benchmark imposes on all participants
struct TimeSyncSettings
{
    bool imposed{false}; //!< false: leave the configuration as it is
    double maxRunAheadMs{0.0};
    double latencyMs{0.0};
};

static auto ImposedTimeSync(const BenchmarkConfig& benchmark) -> TimeSyncSettings
{
    switch (benchmark.syncMode)
    {
    case SyncMode::Lockstep:
        return {true, 0.0, 0.0};
    case SyncMode::MaxRunAhead:
        return {true, std::isnan(benchmark.maxRunAheadMs) ? 5.0 : benchmark.maxRunAheadMs, 0.0};
    case SyncMode::Exact:
        return {true, 0.0, std::isnan(benchmark.latencyMs) ? 5.0 : benchmark.latencyMs};
    default:
        return {};
    }
}

static auto TimeSyncLabel(const TimeSyncSettings& settings) -> std::string
{
    std::ostringstream label;
    if (!settings.imposed)
    {
        label << "as configured";
    }
    else if (settings.latencyMs > 0.0)
    {
        label << "exact (" << settings.latencyMs << "ms latency)";
    }
    else if (settings.maxRunAheadMs > 0.0)
    {
        label << "max-run-ahead (" << settings.maxRunAheadMs << "ms)";
    }
    else
    {
        label << "lockstep";
    }
    return label.str();
}

// Where the timestamp of a received message lies relative to the receiver's simulation step [now, now + step size]:
// within it is on time, before it is late (the receiver ran ahead of the sender), beyond it is early (the sender
// ran ahead of the receiver).
struct ReceptionTiming
{
    uint64_t received{0};
    uint64_t late{0};
    uint64_t early{0};
    std::chrono::nanoseconds sumLateness{0};
    std::chrono::nanoseconds maxLateness{0};
    std::chrono::nanoseconds maxEarliness{0};
    // FNV-1a over (receiver time, timestamp, sender) in delivery order, up to a cutoff every participant reaches
    // before the simulation stops: equal digests mean identical deliveries
    uint64_t digest{14695981039346656037ull};
    std::chrono::nanoseconds digestCutoff{std::chrono::nanoseconds::max()};

    void Record(std::chrono::nanoseconds timestamp, std::chrono::nanoseconds now, std::chrono::nanoseconds stepSize,
                uint8_t senderTag)
    {
        if (now < digestCutoff)
        {
            Mix(static_cast<uint64_t>(now.count()));
            Mix(static_cast<uint64_t>(timestamp.count()));
            Mix(senderTag);
        }

        if (now < 0ns)
        {
            // No step executed yet, the upcoming one starts at 0
            now = -stepSize;
        }
        received++;
        if (timestamp < now)
        {
            late++;
            sumLateness += now - timestamp;
            maxLateness = std::max(maxLateness, now - timestamp);
        }
        else if (timestamp > now + stepSize)
        {
            early++;
            maxEarliness = std::max(maxEarliness, timestamp - (now + stepSize));
        }
    }

    void Merge(const ReceptionTiming& other)
    {
        received += other.received;
        late += other.late;
        early += other.early;
        sumLateness += other.sumLateness;
        maxLateness = std::max(maxLateness, other.maxLateness);
        maxEarliness = std::max(maxEarliness, other.maxEarliness);
    }

private:
    void Mix(uint64_t value)
    {
        for (int i = 0; i < 8; ++i)
        {
            digest = (digest ^ ((value >> (8 * i)) & 0xff)) * 1099511628211ull;
        }
    }
};

static bool Parse(int argc, char** argv, BenchmarkConfig& config)
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
    parseOptional("--simulation-step-size", config.simulationStepSize,
                  [&asNum](const auto& a) { return std::chrono::milliseconds{asNum(a)}; });
    parseOptional("--simulation-step-size-us", config.simulationStepSize,
                  [&asNum](const auto& a) { return std::chrono::microseconds{asNum(a)}; });
    parseOptional("--simulation-duration", config.simulationDuration, asNum);
    parseOptional("--service", config.service, asServiceType);
    parseOptional("--configuration", config.silKitConfigPath, asStr);
    parseOptional("--write-csv", config.writeCsv, asStr);
    parseOptional("--compute-us", config.computeTime, asNum);
    parseOptional("--compute-jitter-us", config.computeJitter, asNum);
    parseOptional("--quiet-participants", config.quietParticipants, asNum);
    parseOptional("--sync-mode", config.syncMode, [](const auto& a) {
        const auto mode = std::string{a};
        if (mode == "lockstep")
            return SyncMode::Lockstep;
        if (mode == "max-run-ahead")
            return SyncMode::MaxRunAhead;
        if (mode == "exact")
            return SyncMode::Exact;
        throw std::runtime_error{"unknown sync mode \"" + mode + "\", expected lockstep, max-run-ahead or exact"};
    });
    parseOptional("--max-run-ahead", config.maxRunAheadMs, [](const auto& a) { return std::stod(std::string{a}); });
    parseOptional("--latency", config.latencyMs, [](const auto& a) { return std::stod(std::string{a}); });

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

static bool Validate(const BenchmarkConfig& config)
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

    if (!std::isnan(config.maxRunAheadMs) && config.syncMode != SyncMode::MaxRunAhead)
    {
        std::cout << "Invalid argument: --max-run-ahead requires --sync-mode max-run-ahead." << std::endl;
        return false;
    }

    if (!std::isnan(config.maxRunAheadMs) && config.maxRunAheadMs <= 0.0)
    {
        std::cout << "Invalid argument: --max-run-ahead must be greater than 0; use --sync-mode lockstep instead."
                  << std::endl;
        return false;
    }

    if (!std::isnan(config.latencyMs) && config.syncMode != SyncMode::Exact)
    {
        std::cout << "Invalid argument: --latency requires --sync-mode exact." << std::endl;
        return false;
    }

    if (!std::isnan(config.latencyMs) && config.latencyMs <= 0.0)
    {
        std::cout << "Invalid argument: --latency must be greater than 0; use --sync-mode lockstep instead."
                  << std::endl;
        return false;
    }

    if (config.quietParticipants >= config.numberOfParticipants)
    {
        std::cout << "Invalid argument: At least one participant must not be quiet." << std::endl;
        return false;
    }

    return true;
}

static void CheckAndTruncateMessageSize(BenchmarkConfig& benchmark)
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

static void PublishMessages(IDataPublisher* publisher, uint32_t messageCount, uint32_t messageSizeInBytes,
                            uint8_t publisherTag)
{
    std::vector<uint8_t> data(messageSizeInBytes, '*');
    // Lets subscribers tell their own publications (self delivery) from those of other participants
    data[0] = publisherTag;
    for (uint32_t i = 0; i < messageCount; i++)
    {
        publisher->Publish(SilKit::Util::ToSpan(data));
    }
}

static void SendEthernetFrames(IEthernetController* ethernetController, uint32_t messageCount, uint32_t messageSizeInBytes,
                               uint8_t senderTag)
{
    std::vector<uint8_t> frameData(messageSizeInBytes, '*');
    frameData[0] = senderTag;
    for (uint32_t i = 0; i < messageCount; i++)
    {
        ethernetController->SendFrame(EthernetFrame{SilKit::Util::ToSpan(frameData)});
    }
}

static void SendCanFrames(ICanController* canController, uint32_t messageCount, uint32_t messageSizeInBytes,
                          uint8_t senderTag)
{
    std::vector<uint8_t> frameData(messageSizeInBytes, '*');
    frameData[0] = senderTag;
    for (uint32_t i = 0; i < messageCount; i++)
    {
        CanFrame canFrame{};
        canFrame.dataField = SilKit::Util::ToSpan(frameData);

        canController->SendFrame(std::move(canFrame));
    }
}

static void BusyWait(std::chrono::nanoseconds duration)
{
    const auto until = std::chrono::steady_clock::now() + duration;
    while (std::chrono::steady_clock::now() < until)
    {
    }
}

static void ParticipantsThread(std::shared_ptr<SilKit::Config::IParticipantConfiguration> config,
                        const BenchmarkConfig& benchmark, const std::string& participantName, uint32_t participantIndex,
                        ReceptionTiming* receptionTiming, std::chrono::nanoseconds* lastStep)
{
    auto participant = SilKit::CreateParticipant(config, participantName, benchmark.registryUri);
    auto* lifecycleService = participant->CreateLifecycleService({OperationMode::Coordinated});
    auto* timeSyncService = lifecycleService->CreateTimeSyncService();

    // Reception handlers and the simulation step handler all run on the participant's I/O thread
    const auto recordTiming = [timeSyncService, receptionTiming, stepSize = benchmark.simulationStepSize](
                                  std::chrono::nanoseconds timestamp, uint8_t senderTag) {
        receptionTiming->Record(timestamp, timeSyncService->Now(), stepSize, senderTag);
    };
    const auto tagOf = [](const auto& bytes) { return bytes.size() > 0 ? static_cast<uint8_t>(bytes[0]) : uint8_t{0}; };

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
        const auto ownTag = static_cast<uint8_t>(participantIndex);
        participant->CreateDataSubscriber("SubCtrl1", matchingDataSpec, [recordTiming, ownTag](auto*, const auto& event) {
            if (event.data.size() > 0 && event.data[0] != ownTag)
            {
                recordTiming(event.timestamp, event.data[0]);
            }
        });
        break;
    }
    case ServiceType::Ethernet:
    {
        ethernetController = participant->CreateEthernetController("Eth1", "Eth1");
        ethernetController->AddFrameHandler(
            [recordTiming, tagOf](auto*, const auto& event) { recordTiming(event.timestamp, tagOf(event.frame.raw)); });
        lifecycleService->SetCommunicationReadyHandler([ethernetController]() { ethernetController->Activate(); });
        break;
    }
    case ServiceType::Can:
    {
        canController = participant->CreateCanController("CAN1", "CAN1");
        canController->AddFrameHandler([recordTiming, tagOf](auto*, const auto& event) {
            recordTiming(event.timestamp, tagOf(event.frame.dataField));
        });
        lifecycleService->SetCommunicationReadyHandler([canController]() {
            canController->SetBaudRate(10'000, 1'000'000, 2'000'000);
            canController->Start();
        });
        break;
    }
    }

    const auto isVerbose = participantIndex == 0;
    const auto isQuiet = participantIndex >= benchmark.numberOfParticipants - benchmark.quietParticipants;
    std::mt19937 random{participantIndex};
    std::uniform_int_distribution<std::chrono::nanoseconds::rep> jitter{
        0, std::chrono::nanoseconds{benchmark.computeJitter}.count()};
    timeSyncService->SetSimulationStepHandler([=](std::chrono::nanoseconds now, const auto /*duration*/) mutable {
        *lastStep = now;
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

        BusyWait(benchmark.computeTime + std::chrono::nanoseconds{jitter(random)});
        if (isQuiet)
        {
            return;
        }

        switch (benchmark.service)
        {
        case ServiceType::PubSub:
        {
            PublishMessages(publisher, benchmark.messageCount, benchmark.messageSizeInBytes,
                            static_cast<uint8_t>(participantIndex));
            break;
        }
        case ServiceType::Ethernet:
        {
            SendEthernetFrames(ethernetController, benchmark.messageCount, benchmark.messageSizeInBytes,
                               static_cast<uint8_t>(participantIndex));
            break;
        }
        case ServiceType::Can:
        {
            SendCanFrames(canController, benchmark.messageCount, benchmark.messageSizeInBytes,
                          static_cast<uint8_t>(participantIndex));
            break;
        }
        }
    }, benchmark.simulationStepSize);

    auto lifecycleFuture = lifecycleService->StartLifecycle();
    lifecycleFuture.get();
}

static void PrintParameters(BenchmarkConfig benchmark)
{
#ifdef SILKIT_BUILD_IS_DEBUG
    std::cout << "WARNING: The benchmark demo is executed in a Debug build configuration." << std::endl
              << "For more reliable timings, please use a Release/RelWithDebInfo build configuration" << std::endl
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
              << std::left << std::setw(39) << "- CSV output: " << benchmark.writeCsv << std::endl
              << std::left << std::setw(39) << "- Compute per step (busy wait): " << benchmark.computeTime.count()
              << "us + up to " << benchmark.computeJitter.count() << "us jitter" << std::endl
              << std::left << std::setw(39) << "- Quiet participants: " << benchmark.quietParticipants << std::endl
              << std::left << std::setw(39) << "- Time synchronization: " << TimeSyncLabel(ImposedTimeSync(benchmark))
              << std::endl;
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

struct BenchmarkResult
{
    TimeSyncSettings timeSync;
    std::vector<uint64_t> runDigests;
    std::pair<double, double> duration;
    std::pair<double, double> speedup;
    std::pair<double, double> throughput;
    std::pair<double, double> messageRate;
    size_t numberMessages{0};
    ReceptionTiming receptionTiming;
    uint32_t incompleteRuns{0};

    auto MeanLatenessUs() const -> double
    {
        return receptionTiming.late == 0 ? 0.0 : ToUs(receptionTiming.sumLateness) / receptionTiming.late;
    }

    static auto ToUs(std::chrono::nanoseconds d) -> double
    {
        return std::chrono::duration<double, std::micro>{d}.count();
    }

    bool DigestsIdentical() const
    {
        return std::adjacent_find(runDigests.begin(), runDigests.end(), std::not_equal_to<uint64_t>{})
               == runDigests.end();
    }
};

// --sync-mode is layered on top of --configuration via an include. The configuration file must not set
// MaxRunAhead or Latency itself: the 0 of lockstep is the default value and would lose against the file.
static auto MakeParticipantConfiguration(const BenchmarkConfig& benchmark, const TimeSyncSettings& timeSync)
    -> std::shared_ptr<SilKit::Config::IParticipantConfiguration>
{
    if (!timeSync.imposed)
    {
        if (benchmark.silKitConfigPath.empty())
        {
            return SilKit::Config::ParticipantConfigurationFromString("{}");
        }
        return SilKit::Config::ParticipantConfigurationFromFile(benchmark.silKitConfigPath);
    }

    std::ostringstream yaml;
    if (!benchmark.silKitConfigPath.empty())
    {
        std::ifstream file{benchmark.silKitConfigPath};
        const std::string content{std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
        if (content.find("MaxRunAhead") != std::string::npos || content.find("Latency") != std::string::npos)
        {
            throw SilKit::ConfigurationError{"Set MaxRunAhead and Latency either in the configuration file or with "
                                             "--sync-mode, not both."};
        }

        // Single-quoted YAML keeps Windows backslashes literal; quotes are escaped by doubling them
        auto path = benchmark.silKitConfigPath;
        for (auto pos = path.find('\''); pos != std::string::npos; pos = path.find('\'', pos + 2))
        {
            path.insert(pos, 1, '\'');
        }
        yaml << "Includes:\n  Files:\n    - '" << path << "'\n";
    }
    yaml << "Experimental:\n  TimeSynchronization:\n    MaxRunAhead: " << timeSync.maxRunAheadMs
         << "\n    Latency: " << timeSync.latencyMs << "\n";
    return SilKit::Config::ParticipantConfigurationFromString(yaml.str());
}

static auto RunSimulations(const BenchmarkConfig& benchmark,
                           std::shared_ptr<SilKit::Config::IParticipantConfiguration> config,
                           const TimeSyncSettings& timeSync) -> BenchmarkResult
{
    BenchmarkResult result;
    result.timeSync = timeSync;
    std::vector<std::chrono::nanoseconds> measuredRealDurations;

    for (uint32_t simulationRun = 1; simulationRun <= benchmark.numberOfSimulationRuns; simulationRun++)
    {
        std::cout << "> Simulation " << simulationRun << ": ";
        auto startTimestamp = std::chrono::high_resolution_clock::now();

        std::vector<std::string> participantNames;
        std::vector<std::thread> threads;
        std::vector<ReceptionTiming> receptionTimings(benchmark.numberOfParticipants);
        // The first participant to reach the end stops everyone, the others may still be a window behind
        const auto window = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::duration<double, std::milli>{std::max(timeSync.maxRunAheadMs, timeSync.latencyMs)});
        for (auto& receptionTiming : receptionTimings)
        {
            receptionTiming.digestCutoff =
                std::chrono::nanoseconds{benchmark.simulationDuration} - 2 * (window + benchmark.simulationStepSize);
        }
        std::vector<std::chrono::nanoseconds> lastSteps(benchmark.numberOfParticipants, -1ns);
        for (uint32_t participantIndex = 0; participantIndex < benchmark.numberOfParticipants; participantIndex++)
        {
            std::string participantName = "Participant" + std::to_string(participantIndex);
            participantNames.push_back(participantName);
            threads.emplace_back(&ParticipantsThread, config, benchmark, participantName, participantIndex,
                                 &receptionTimings[participantIndex], &lastSteps[participantIndex]);
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
        uint64_t runDigest = 0;
        for (const auto& receptionTiming : receptionTimings)
        {
            result.receptionTiming.Merge(receptionTiming);
            runDigest = (runDigest ^ receptionTiming.digest) * 1099511628211ull;
        }
        result.runDigests.push_back(runDigest);
        // With run-ahead, the others may be behind when the first participant reaches the end and stops
        // the simulation, but by much less than this
        const auto slowestLastStep = *std::min_element(lastSteps.begin(), lastSteps.end());
        const bool incomplete = slowestLastStep < std::chrono::nanoseconds{benchmark.simulationDuration} * 99 / 100;
        if (incomplete)
        {
            result.incompleteRuns++;
        }

        systemControllerParticipant.reset();

        auto endTimestamp = std::chrono::high_resolution_clock::now();
        measuredRealDurations.emplace_back(endTimestamp - startTimestamp);
        std::cout << "  " << measuredRealDurations.back() << "  digest " << std::hex << runDigest << std::dec;
        if (incomplete)
        {
            std::cout << "  INCOMPLETE: the slowest participant only reached " << slowestLastStep;
        }
        std::cout << std::endl;
    }

    std::vector<double> measuredRealDurationsSeconds(measuredRealDurations.size());
    std::transform(measuredRealDurations.begin(), measuredRealDurations.end(), measuredRealDurationsSeconds.begin(),
                   [](auto d) { return static_cast<double>(d.count() / 1e9); });

    result.duration = mean_and_error(measuredRealDurationsSeconds);

    const auto numberSimulationSteps = static_cast<size_t>(benchmark.simulationDuration / benchmark.simulationStepSize);
    // fully connected network topology
    result.numberMessages = numberSimulationSteps * benchmark.messageCount
                            * (benchmark.numberOfParticipants - benchmark.quietParticipants)
                            * (benchmark.numberOfParticipants - 1);

    // Reoccuring factor from error propagation
    const auto sigmaDurOverDurSqr = result.duration.second / result.duration.first / result.duration.first;

    // Byte throughput mean and error (Byte to Mebi and ns to s)
    const auto throughputPrefac = result.numberMessages * benchmark.messageSizeInBytes / 1024.0 / 1024.0;
    result.throughput =
        std::make_pair(throughputPrefac / result.duration.first, throughputPrefac * sigmaDurOverDurSqr);

    // Message rate mean and error
    result.messageRate =
        std::make_pair(result.numberMessages / result.duration.first, result.numberMessages * sigmaDurOverDurSqr);

    // Speedup mean and error
    result.speedup = std::make_pair(benchmark.simulationDuration.count() / result.duration.first,
                                    benchmark.simulationDuration.count() * sigmaDurOverDurSqr);
    return result;
}

static void PrintResult(const BenchmarkConfig& benchmark, const BenchmarkResult& result)
{
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
    averageDurationWithUnit << result.duration.first << "s";

    std::ostringstream averageThroughputWithUnit;
    averageThroughputWithUnit.precision(3);
    averageThroughputWithUnit << result.throughput.first << " MiB/s";

    std::ostringstream averageMsgRateWithUnit;
    averageMsgRateWithUnit << static_cast<int>(result.messageRate.first) << " 1/s";

    const auto& timing = result.receptionTiming;
    std::cout << std::setw(39) << "- Time synchronization: " << TimeSyncLabel(result.timeSync) << std::endl

              << std::setw(39) << "- Realtime duration (runtime): " << std::setw(13) << averageDurationWithUnit.str()
              << " +/- " << result.duration.second << "s" << std::endl

              << std::setw(39) << "- Speedup (virtual time/runtime): " << std::setw(13) << result.speedup.first
              << " +/- " << result.speedup.second << std::endl

              << std::setw(39) << "- Throughput (data size/runtime): " << std::setw(13)
              << averageThroughputWithUnit.str() << " +/- " << result.throughput.second << " MiB/s" << std::endl

              << std::setw(39) << "- Message rate (count/runtime): " << std::setw(13) << averageMsgRateWithUnit.str()
              << " +/- " << static_cast<int>(result.messageRate.second) << " 1/s" << std::endl

              << std::left << std::setw(39) << "- Total number of messages: " << result.numberMessages << std::endl

              << std::left << std::setw(39) << "- Received messages (all runs): " << timing.received << std::endl
              << std::left << std::setw(39) << "- Late (before receiver's step): " << timing.late << ", mean "
              << result.MeanLatenessUs() << "us, max " << BenchmarkResult::ToUs(timing.maxLateness) << "us"
              << std::endl
              << std::left << std::setw(39) << "- Early (beyond receiver's step): " << timing.early << ", max "
              << BenchmarkResult::ToUs(timing.maxEarliness) << "us" << std::endl
              << std::left << std::setw(39) << "- Incomplete runs: " << result.incompleteRuns << std::endl
              << std::left << std::setw(39) << "- Delivery digests: "
              << (result.DigestsIdentical() ? "identical in all runs" : "differ between runs") << std::endl
              << std::endl
              << std::endl;
}

static void WriteCsv(const BenchmarkConfig& benchmark, const BenchmarkResult& result)
{
    std::stringstream csvHeader;
    csvHeader << "# SilKitBenchmarkDemo, SIL Kit Version " << SilKit::Version::String();
    const auto csvColumns = "numRuns; participants; messageSize; messageCount; duration(virtual time, s); "
                            "stepSize(virtual time, ms); "
                            "numberMessageSent; runtime(s); runtime_err; throughput(MiB/s); "
                            "throughput_err; speedup; speedup_err; messageRate(1/s); messageRate_err; "
                            "compute(us); computeJitter(us); quietParticipants; received; late; "
                            "meanLateness(us); maxLateness(us); early; maxEarliness(us); incompleteRuns; "
                            "maxRunAhead(ms); service; latency(ms); digestsIdentical";
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
        const auto& timing = result.receptionTiming;
        // Append data
        csvFile.seekp(0, std::ios_base::end);
        csvFile << benchmark.numberOfSimulationRuns << ";" << benchmark.numberOfParticipants << ";"
                << benchmark.messageSizeInBytes << ";" << benchmark.messageCount << ";"
                << benchmark.simulationDuration.count() << ";"
                << std::chrono::duration<double, std::milli>{benchmark.simulationStepSize}.count() << ";"
                << result.numberMessages << ";" << result.duration.first << ";" << result.duration.second << ";"
                << result.throughput.first << ";" << result.throughput.second << ";" << result.speedup.first << ";"
                << result.speedup.second << ";" << result.messageRate.first << ";" << result.messageRate.second
                << ";" << benchmark.computeTime.count() << ";" << benchmark.computeJitter.count() << ";"
                << benchmark.quietParticipants << ";" << timing.received << ";" << timing.late << ";"
                << result.MeanLatenessUs() << ";" << BenchmarkResult::ToUs(timing.maxLateness) << ";"
                << timing.early << ";" << BenchmarkResult::ToUs(timing.maxEarliness) << ";"
                << result.incompleteRuns << ";";
        if (result.timeSync.imposed)
        {
            csvFile << result.timeSync.maxRunAheadMs;
        }
        csvFile << ";" << benchmark.service << ";";
        if (result.timeSync.imposed)
        {
            csvFile << result.timeSync.latencyMs;
        }
        csvFile << ";" << (result.DigestsIdentical() ? 1 : 0) << std::endl;
    }
    csvFile.close();
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
        std::unique_ptr<SilKit::Vendor::Vector::ISilKitRegistry> registry =
            SilKit::Vendor::Vector::CreateSilKitRegistry(MakeParticipantConfiguration(benchmark, TimeSyncSettings{}));
        benchmark.registryUri = registry->StartListening(benchmark.registryUri);

        const auto timeSync = ImposedTimeSync(benchmark);
        const auto result = RunSimulations(benchmark, MakeParticipantConfiguration(benchmark, timeSync), timeSync);
        PrintResult(benchmark, result);
        if (benchmark.writeCsv != "")
        {
            WriteCsv(benchmark, result);
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
