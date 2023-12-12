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
        << executableName << " [messageCount]"
        << " [messageSizeInBytes]"
        << " [registryURi]" << std::endl
        << "If no arguments are given, default values will be used." << std::endl
        << "\t--help\tshow this message." << std::endl
        << "\t--isReceiver\tThis process is the receiving counterpart of the latency measurement" << std::endl
        << "\t--registry-uri\tThe URI of the registry to start. Default: silkit://localhost:8500" << std::endl
        << "\t--message-size\tSets the message size to BYTES. Default: 1000" << std::endl
        << "\t--message-count\tSets the number of messages to be send per participant in each simulation step to "
           "NUM. Default: 1000"
        << std::endl
        << "\t--configuration\tPath and filename of the participant configuration YAML or JSON file. Default: empty"
        << std::endl
        << "\t--write-csv\tPath and filename of csv file with benchmark results. Default: empty" << std::endl;
}

struct BenchmarkConfig
{
    uint32_t messageCount = 1000;
    uint32_t messageSizeInBytes = 1000;
    bool isReceiver = false;
    std::string registryUri = "silkit://localhost:8500";
    std::string silKitConfigPath = "";
    std::string writeCsv = "";
};

bool Parse(int argc, char** argv, BenchmarkConfig& config)
{
    // skip argv[0] and collect all arguments
    std::vector<std::string> args;
    std::copy((argv + 1), (argv + argc), std::back_inserter(args));

    auto asNum = [](const auto& str) {
        return static_cast<uint32_t>(std::stoul(str));
    };
    auto asStr = [](auto& a) {
        return std::string{a};
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

    if (consumeFlag("--isReceiver"))
    {
        config.isReceiver = true;
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
    if (args.size() > 3)
    {
        std::cout << "Error: Too many arguments!" << std::endl;
        PrintUsage(argv[0]);
        return false;
    }

    try
    {
        switch (args.size())
        {
        case 3:
            config.registryUri = args.at(2);
            // [[fallthrough]]
        case 2:
            config.messageSizeInBytes = asNum(args.at(1));
            // [[fallthrough]]
        case 1: config.messageCount = asNum(args.at(0)); break;
        default:
            if (haveUserOptions)
            {
                std::cout << std::endl << "Using user specified configuration to override defaults." << std::endl;
            }
            else
            {
                std::cout << std::endl << "No benchmark arguments given: using default configuration.";
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
    if (config.messageCount < 1)
    {
        std::cout << "Invalid argument: The message count must be at least 1." << std::endl;
        return false;
    }
    if (config.messageSizeInBytes < 1)
    {
        std::cout << "Invalid argument: The message payload size must be at least 1 byte." << std::endl;
        return false;
    }

    return true;
}

void PrintParameters(BenchmarkConfig benchmark)
{
#ifndef NDEBUG
    std::cout << "WARNING: The latency demo is executed in a DEBUG build configuration." << std::endl
              << "For more reliable timings, please use a RELEASE build configuration" << std::endl
              << "of the SIL Kit library and the latency demo." << std::endl;
    std::this_thread::sleep_for(2s);
#endif

    std::cout << std::endl
              << "This latency demo produces timings of a configurable simulation setup." << std::endl
              << "Two participants exchange <M> messages of <B> bytes without time synchronization." << std::endl
              << "The demo uses PubSub controllers performing a message roundtrip (ping-pong) " << std::endl
              << "to calculate latency and throughput timings." << std::endl
              << "Note that the two participants must use the same parameters for a valid measurement " << std::endl
              << "and one participant must use the --isReceiver flag." << std::endl
              << std::endl
              << "Running simulations with the following parameters:" << std::endl
              << std::endl
              << std::left << std::setw(38) << "- Is Receiver: " << (benchmark.isReceiver ? "True" : "False") << std::endl
              << std::left << std::setw(38) << "- Total message count: " << benchmark.messageCount << std::endl
              << std::left << std::setw(38) << "- Message size (bytes): " << benchmark.messageSizeInBytes << std::endl
              << std::left << std::setw(38) << "- Registry URI: " << benchmark.registryUri << std::endl
              << std::left << std::setw(38) << "- Configuration: " << benchmark.silKitConfigPath << std::endl
              << std::left << std::setw(38) << "- CSV output: " << benchmark.writeCsv << std::endl
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

        std::vector<std::chrono::nanoseconds> measuredRoundtrips;
        std::chrono::high_resolution_clock::time_point startTimestamp{};

        // -----------------------------------
        // Runtime measurement

        std::string participantName = benchmark.isReceiver ? " Receiver" : "Sender";
        std::vector<uint8_t> data(benchmark.messageSizeInBytes, '*');
        auto participant = SilKit::CreateParticipant(config, participantName, benchmark.registryUri);

        uint32_t sendCount{0};
        std::atomic_bool allSent{false};
        std::promise<void> allSentPromise;
        std::chrono::high_resolution_clock::time_point sendTime;

        std::string topicPub = benchmark.isReceiver ? "Pong" : "Ping";
        std::string topicSub = benchmark.isReceiver ? "Ping" : "Pong";
        auto publisher = participant->CreateDataPublisher("PubCtrl1", {topicPub, {}}, 1);
        participant->CreateDataSubscriber(
            "SubCtrl1", {topicSub, {}},
            [data, publisher, benchmark, &sendCount, &allSent, &allSentPromise,

             &measuredRoundtrips, &sendTime, &startTimestamp](auto*, auto&) {
                if (!allSent)
                {
                    if (!benchmark.isReceiver)
                    {
                        if (sendCount == 0)
                        {
                            startTimestamp = std::chrono::high_resolution_clock::now(); // Initial receive: Start runtime measurement
                        }
                        else
                        {
                            measuredRoundtrips.push_back(std::chrono::high_resolution_clock::now() - sendTime);
                        }
                        sendTime = std::chrono::high_resolution_clock::now();
                    }
                    publisher->Publish(data);
                    sendCount++;
                    if (benchmark.isReceiver && (benchmark.messageCount <= 20 || sendCount % (benchmark.messageCount / 20) == 0))
                    {
                        std::cout << ".";
                    }
                    if (sendCount >= benchmark.messageCount+1) // Initial publish has no timing, use +1
                    {
                        allSentPromise.set_value();
                        allSent = true;
                    }
                }
            });

        if (!benchmark.isReceiver) // Initial publish without timing
        {
            publisher->Publish(data);
        }
        auto allSendFuture = allSentPromise.get_future();
        allSendFuture.wait();

        // -----------------------------------
        // End Runtime measurement

        auto duration = std::chrono::high_resolution_clock::now() - startTimestamp;
        std::cout << std::endl << std::endl << "Runtime measurement done. Synchronizing ..." << std::endl;

        // Sync Ping-Pong to make sure last publish (probably large) has arrived
        std::promise<void> syncParticipants;
        std::string topicPubAllDone = benchmark.isReceiver ? "AllDoneReceiver" : "AllDoneSender";
        std::string topicSubAllDone = benchmark.isReceiver ? "AllDoneSender" : "AllDoneReceiver";
        auto allDonePublisher = participant->CreateDataPublisher("PubCtrl2", {topicPubAllDone, {}}, 1);
        if (benchmark.isReceiver)
        {
            allDonePublisher->Publish(std::vector<uint8_t>{0});
        }
        participant->CreateDataSubscriber("SubCtrl2", {topicSubAllDone, {}},
            [&syncParticipants, benchmark, allDonePublisher](auto*, auto&) {
                if (!benchmark.isReceiver)
                {
                    allDonePublisher->Publish(std::vector<uint8_t>{0});
                }
                syncParticipants.set_value();
            });
        auto syncParticipantsFuture = syncParticipants.get_future();
        syncParticipantsFuture.wait();
        std::cout << "... done." << std::endl;

        if (benchmark.isReceiver) // Receiver is done here
        {
            std::cout << "Receiver done." << std::endl;
            return 0;
        }

        // -----------------------------------
        // Calculation of KPIs

        std::vector<double> measuredLatencySeconds(measuredRoundtrips.size());
        std::transform(measuredRoundtrips.begin(), measuredRoundtrips.end(), measuredLatencySeconds.begin(),
                       [](auto d) {
                           return static_cast<double>(d.count() / 1.e3 * 0.5); // Convert to microseconds, factor 0.5 for latency from roundtrip
                       });

        const auto averageLatency = mean_and_error(measuredLatencySeconds);
        std::ostringstream averageLatencyWithUnit;
        averageLatencyWithUnit.precision(3);
        averageLatencyWithUnit << averageLatency.first << " us";

        // Total throughput includes both direction, hence the factor 2.0
        const auto durationSeconds = duration.count() / 1e9;
        const auto throughput =
            2.0 * benchmark.messageCount * benchmark.messageSizeInBytes / 1024.0 / 1024.0 / durationSeconds;
        std::ostringstream throughputWithUnit;
        throughputWithUnit.precision(3);
        throughputWithUnit << throughput << " MiB/s";

        std::cout << std::endl << std::endl << "Result of the simulation run:" << std::endl << std::endl;

        // Stream helper to combine value and unit to use it with std::setw as a whole
        std::ostringstream durationWithUnit;
        durationWithUnit.precision(3);
        durationWithUnit << durationSeconds << " s";

        std::cout << std::setw(38) << "- Realtime duration (runtime): "     << std::setw(6) << durationWithUnit.str() << std::endl
                  << std::setw(38) << "- Throughput (data size/runtime): " << std::setw(6) << throughputWithUnit.str() << std::endl
                  << std::setw(38) << "- Latency: "                         << std::setw(6) << averageLatencyWithUnit.str() << " +/- " << averageLatency.second << std::endl
                  << std::endl;

        if (benchmark.writeCsv != "")
        {
            std::stringstream csvHeader;
            csvHeader << "# SilKitBenchmarkDemo, SIL Kit Version " << SilKit::Version::String();
            const auto csvColumns = "messageSize; messageCount; runtime(s); throughput(MiB/s); latency(us); latency_err";
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
                csvFile << benchmark.messageSizeInBytes << ";" << benchmark.messageCount << ";" 
                        << durationSeconds << ";"
                        << throughput << ";"
                        << averageLatency.first << ";" << averageLatency.second << std::endl;
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
