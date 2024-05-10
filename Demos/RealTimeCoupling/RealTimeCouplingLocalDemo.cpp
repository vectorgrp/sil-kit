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

#include <future>
#include <iostream>
#include <sstream>
#include <thread>

#include "silkit/SilKit.hpp"
#include "silkit/services/all.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/services/pubsub/all.hpp"
#include "silkit/services/rpc/all.hpp"
#include "silkit/util/serdes/Serialization.hpp"


using namespace SilKit::Services::Orchestration;
using namespace SilKit::Services::PubSub;
using namespace SilKit::Services::Rpc;

using namespace std::chrono_literals;

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

void ReceiveData(IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent)
{
    auto eventData = SilKit::Util::ToStdVector(dataMessageEvent.data);
    std::string str(dataMessageEvent.data.begin(), dataMessageEvent.data.end());
    //std::cout << "<< Received data=" << eventData << ", t=" << dataMessageEvent.timestamp.count() *1e-6
    //          << std::endl;
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0]
                  << " <ParticipantConfiguration.yaml|json> <ParticipantName> [RegistryUri]" << std::endl;
        return -1;
    }

    try
    {
        std::string participantConfigurationFilename(argv[1]);
        std::string registryUri = "silkit://localhost:8500";

        std::string participantName(argv[2]);
        bool isRealtime = false;
        const auto stepDuration = 1000ms;

        std::vector<std::string> args;
        std::copy((argv + 3), (argv + argc), std::back_inserter(args));

        for (auto&& arg : args)
        {
            registryUri = arg;
        }

        auto participantConfiguration =
            SilKit::Config::ParticipantConfigurationFromFile(participantConfigurationFilename);

        std::cout << "Creating participant '" << participantName << "' with registry " << registryUri << std::endl;
        auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryUri);
        auto* lifecycleService = participant->CreateLifecycleService({OperationMode::Coordinated});

        // Observe state changes
        lifecycleService->SetStopHandler([]() {
            std::cout << "Stop handler called" << std::endl;
        });
        lifecycleService->SetShutdownHandler([]() {
            std::cout << "Shutdown handler called" << std::endl;
        });
        lifecycleService->SetAbortHandler([](auto lastState) {
            std::cout << "Abort handler called while in state " << lastState << std::endl;
        });

        std::string mediaType{SilKit::Util::SerDes::MediaTypeData()};
        PubSubSpec dataSpec{"DataPulse", mediaType};
        auto* dataPublisher = participant->CreateDataPublisher("DataPublisher", dataSpec, 0);
        auto* dataSubscriber = participant->CreateDataSubscriber("DataSubscriber", dataSpec, &ReceiveData);

        lifecycleService->SetCommunicationReadyHandler([&participantName]() {
            std::cout << "Communication ready handler called for " << participantName << std::endl;
        });


        SilKit::Services::Orchestration::ITimeSyncService* timeSyncService{nullptr};
        timeSyncService = lifecycleService->CreateTimeSyncService();

        timeSyncService->SetSimulationStepHandler(
            [stepDuration, dataPublisher](
                std::chrono::nanoseconds nowVT, std::chrono::nanoseconds /*duration*/) {

                static auto timeSimStart = std::chrono::system_clock::now();
                auto nowRT = std::chrono::system_clock::now() - timeSimStart;

                std::cout << "nowRT=" << nowRT << std::endl;
                std::cout << "nowVT=" << nowVT << std::endl;

                //std::cout << "Publish" << std::endl;
                std::array<uint8_t, 1> dataBytes{1};
                dataPublisher->Publish(SilKit::Util::MakeSpan(dataBytes));

                auto randDelay = (rand() / double(RAND_MAX)) * 1.5s;
                std::cout << "randDelay=" << randDelay << std::endl;
                std::this_thread::sleep_for(randDelay);
                std::cout << "--------------------" << std::endl;
            },
            stepDuration);

        auto finalStateFuture = lifecycleService->StartLifecycle();
        auto finalState = finalStateFuture.get();

        std::cout << "Simulation stopped. Final State: " << finalState << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
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
