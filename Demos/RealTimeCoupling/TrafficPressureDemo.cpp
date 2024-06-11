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
#include "silkit/services/all.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/services/pubsub/all.hpp"
#include "silkit/services/rpc/all.hpp"
#include "silkit/util/serdes/Serialization.hpp"
#include "silkit/SilKitVersion.hpp"

using namespace SilKit::Services::Orchestration;
using namespace SilKit::Services::PubSub;
using namespace SilKit::Services::Rpc;

using namespace std::chrono_literals;

double bandwidthSend = 0;
double bandwidthReceive = 0;

/**************************************************************************************************
 * Main Function
 **************************************************************************************************/

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec)
{
    os << "[";
    for (size_t i = 0; i < vec.size(); ++i)
    {
        os << static_cast<int>(vec[i]);
        if (i != vec.size() - 1)
        {
            os << ", ";
        }
    }
    os << "]";
    return os;
}

std::ostream& operator<<(std::ostream& out, std::chrono::duration<double> timestamp)
{
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp);
    out << milliseconds.count() << "ms";
    return out;
}

void Receive(IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent)
{
    static size_t bytesReceived = 0;
    const static auto firstReceive = std::chrono::steady_clock::now();
    const std::chrono::nanoseconds runtime = std::chrono::steady_clock::now() - firstReceive;
    bytesReceived += dataMessageEvent.data.size();
    bandwidthReceive = bytesReceived / 1e6 / runtime.count() * 1e9; // MB/s

    std::cout << "\rbytes received (MB) = " << std::left << std::setw(10) << bytesReceived / 1e6
              << " bandwidth (MB/s) = " << std::left << std::setw(10) << bandwidthReceive
              << " bandwidth (GBit/s) = " << std::left << std::setw(10) << bandwidthReceive * 0.008;
}

void Publish(IDataPublisher* dataPublisher, size_t msgSizeBytes)
{
    static size_t bytesSent = 0;
    static const auto firstSend = std::chrono::steady_clock::now();

    auto dataBytes = std::vector<uint8_t>(msgSizeBytes, 1);
    dataPublisher->Publish(dataBytes);
    bytesSent += msgSizeBytes;

    const std::chrono::nanoseconds runtime = std::chrono::steady_clock::now() - firstSend;
    bandwidthSend = bytesSent / 1e6 / runtime.count() * 1e9; // MB/s

    std::cout << "\rbytes sent (MB) = " << std::left << std::setw(10) << bytesSent / 1e6
              << " bandwidth (MB/s) = " << std::left << std::setw(10) << bandwidthSend
              << " bandwidth (GBit/s) = " << std::left << std::setw(10) << bandwidthSend * 0.008;
}

int main(int argc, char** argv)
{
    if (argc < 5)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0]
                  << " <ParticipantConfiguration.yaml|json> <ParticipantName> <sleep ms> <msg size bytes> [--async] [--sender] [--receiver]" << std::endl;
        return -1;
    }

    try
    {
        std::string participantConfigurationFilename(argv[1]);
        std::string registryUri = "silkit://localhost:8500";

        std::string participantName(argv[2]);
        std::chrono::milliseconds sleepTimeMs(atoi(argv[3]));
        size_t msgSizeBytes(atoi(argv[4]));

        bool isAsync = false;
        bool isSender = false;
        bool isReceiver = false;
        const auto stepDuration = 1000ms;

        std::vector<std::string> args;
        std::copy((argv + 5), (argv + argc), std::back_inserter(args));

        for (auto&& arg : args)
        {
            if (arg == "--async")
            {
                isAsync = true;
            }
            else if (arg == "--sender")
            {
                isSender = true;
            }
            else if (arg == "--receiver")
            {
                isReceiver = true;
            }
        }

        //std::cout << "Run args:" << std::endl;
        //std::cout << "name:             " << participantName << std::endl;
        //std::cout << "sleep ms :        " << sleepTimeMs << std::endl;
        //std::cout << "msg size bytes:   " << msgSizeBytes << std::endl;
        //std::cout << "async:            " << isAsync << std::endl;
        //std::cout << "sender:           " << isSender << std::endl;
        //std::cout << "receiver:         " << isReceiver << std::endl;

        auto participantConfiguration =
            SilKit::Config::ParticipantConfigurationFromFile(participantConfigurationFilename);

        auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryUri);
        auto* lifecycleService = participant->CreateLifecycleService({OperationMode::Coordinated});


        std::string mediaType{SilKit::Util::SerDes::MediaTypeData()};
        PubSubSpec dataSpec{"DataPulse", mediaType};

        std::promise<void> promiseObj{};
        std::future<void> futureObj = promiseObj.get_future();
        std::thread workerThread;
        
        IDataPublisher* dataPublisher{nullptr};
        IDataSubscriber* dataSubscriber{nullptr};

        if (isSender)
        {
            dataPublisher = participant->CreateDataPublisher("DataPublisher", dataSpec, 0);
        }
        if (isReceiver)
        {
            dataSubscriber = participant->CreateDataSubscriber("DataSubscriber", dataSpec, &Receive);
        }

        if (isAsync)
        {

            lifecycleService->SetCommunicationReadyHandler([&]() {
                workerThread = std::thread{[&]() {
                    futureObj.get();

                    while (lifecycleService->State() == ParticipantState::ReadyToRun
                           || lifecycleService->State() == ParticipantState::Running)
                    {
                        if (isSender)
                        {
                            Publish(dataPublisher, msgSizeBytes);
                        }
                        if (sleepTimeMs > 0ms)
                        {
                            std::this_thread::sleep_for(sleepTimeMs);
                        }
                    }
                }};
            });

            lifecycleService->SetStartingHandler([&]() {
                promiseObj.set_value();
            });
        }
        else
        {
            SilKit::Services::Orchestration::ITimeSyncService* timeSyncService{nullptr};
            timeSyncService = lifecycleService->CreateTimeSyncService();

            timeSyncService->SetSimulationStepHandler(
                [&](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                    {
                        //if (now == 20000s)
                        //{
                        //    lifecycleService->Stop("Done");
                        //}
                        //else
                        {
                            if (isSender)
                            {
                                Publish(dataPublisher, msgSizeBytes);
                            }
                            if (sleepTimeMs > 0ms)
                            {
                                std::this_thread::sleep_for(sleepTimeMs);
                            }
                        }
                    }
                },
                stepDuration);
        }

        auto finalStateFuture = lifecycleService->StartLifecycle();
        auto finalState = finalStateFuture.get();

        if (isAsync)
        {
            workerThread.join();
        }

        std::string csvOutputFile{}; 
        if (isSender)
        {
            csvOutputFile = "traffic-pressure-data-sender.csv";
        }
        else
        {
            csvOutputFile = "traffic-pressure-data-receiver.csv";
        }

        std::stringstream csvHeader;
        csvHeader << "# SilKitTrafficPressureDemo, SIL Kit Version " << SilKit::Version::String();
        const auto csvColumns =
            "messageSize (bytes); sleepTime (ms); bandwidth receive (MB/s); bandwidth send (MB/s)";
        std::fstream csvFile;
        csvFile.open(csvOutputFile, std::ios_base::in | std::ios_base::out); // Try to open
        bool csvValid{true};
        if (!csvFile.is_open())
        {
            // File doesn't exist, create new file and write header
            csvFile.open(csvOutputFile, std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
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
                std::cerr << "Invalid header in file \"" << csvOutputFile << "\"." << std::endl;
                csvValid = false;
            }
        }
        if (csvValid)
        {
            // Append data
            csvFile.seekp(0, std::ios_base::end);
            csvFile << msgSizeBytes << ";" << sleepTimeMs << ";" << bandwidthReceive << ";" << bandwidthSend
                    << std::endl;
        }
        csvFile.close();

        //std::cout << "Simulation stopped. Final State: " << finalState << std::endl;
        //std::cout << "Press enter to stop the process..." << std::endl;
        //std::cin.ignore();
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
