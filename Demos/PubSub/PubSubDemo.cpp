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
#include <sstream>
#include <thread>

#include "silkit/SilKit.hpp"
#include "silkit/services/all.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/services/pubsub/PubSubSpec.hpp"
#include "silkit/util/serdes/Serialization.hpp"


using namespace SilKit::Services::PubSub;
using namespace std::chrono_literals;

/**************************************************************************************************
 * Main Function
 **************************************************************************************************/

/*
Publisher
Pub: "Gps"
Pub: "Temperature"

Subscriber
Sub: "Gps"
Sub: "Temperature"
*/

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0]
                  << " <ParticipantConfiguration.yaml|json> <ParticipantName> [RegistryUri]" << std::endl
                  << "Use \"Publisher1\", \"Publisher2\", \"Subscriber1\" or \"Subscriber2\" as <ParticipantName>." << std::endl;
        return -1;
    }

    std::string mediaType{SilKit::Util::SerDes::MediaTypeData()};

    try
    {
        std::string participantConfigurationFilename(argv[1]);
        std::string participantName(argv[2]);

        auto registryUri = "silkit://localhost:8500";
        if (argc >= 4)
        {
            registryUri = argv[3];
        }

        auto participantConfiguration = SilKit::Config::ParticipantConfigurationFromFile(participantConfigurationFilename);

        std::cout << "Creating participant '" << participantName << "' with registry " << registryUri << std::endl;
        auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryUri);

        auto* lifecycleService =
            participant->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated});
        auto* timeSyncService = lifecycleService->CreateTimeSyncService();

        lifecycleService->SetCommunicationReadyHandler([&participantName]() {
            std::cout << "Communication ready for " << participantName << std::endl;
        });
        lifecycleService->SetStopHandler([]() {
            std::cout << "Stopping..." << std::endl;
        });

        lifecycleService->SetShutdownHandler([]() {
            std::cout << "Shutting down..." << std::endl;
        });

        if (participantName == "Publisher")
        {
            // Create a data publisher for GPS data
            SilKit::Services::PubSub::PubSubSpec dataSpecPubGps{"Gps", mediaType};
            auto* gpsPublisher = participant->CreateDataPublisher("Gps", dataSpecPubGps, 0);
            
            // Create a data publisher for temperature data
            SilKit::Services::PubSub::PubSubSpec dataSpecPubTemperature{"Temperature", mediaType};
            auto* temperaturePublisher = participant->CreateDataPublisher("Temperature", dataSpecPubTemperature, 0);

            timeSyncService->SetSimulationStepHandler(
                [gpsPublisher, temperaturePublisher](std::chrono::nanoseconds now,
                                                     std::chrono::nanoseconds /*duration*/) {
                    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                    std::cout << "now=" << nowMs.count() << "ms" << std::endl;

                    // GPS
                    double lat = 48.8235 + static_cast<double>((rand() % 150)) / 100000;
                    double lon = 9.0965 + static_cast<double>((rand() % 150)) / 100000;
                    std::string signalQuality = "Strong";

                    // Serialize data
                    SilKit::Util::SerDes::Serializer serializer;
                    serializer.BeginStruct();
                    serializer.Serialize(lat);
                    serializer.Serialize(lon);
                    serializer.Serialize(signalQuality);
                    serializer.EndStruct();

                    // Publish serialized data
                    gpsPublisher->Publish(serializer.ReleaseBuffer());
                    std::cout << ">> Published Gps data lat=" << lat << ", lon=" << lon << ", signalQuality=" << signalQuality << std::endl;

                    // Temperature
                    double temperature = 25.0 + static_cast<double>(rand() % 10) / 10.0;

                    // Serialize data
                    serializer.Serialize(temperature);

                    // Publish serialized data
                    temperaturePublisher->Publish(serializer.ReleaseBuffer());
                    std::cout << ">> Published temperature data temperature=" << temperature << std::endl;

                    std::this_thread::sleep_for(1s);

            }, 1s);
        }
        else if (participantName == "Subscriber")
        {
            SilKit::Services::PubSub::PubSubSpec dataSpecPubGps{"Gps", mediaType};
            participant->CreateDataSubscriber(
                "Gps", dataSpecPubGps,
                [](IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent) {
                    auto eventData = SilKit::Util::ToStdVector(dataMessageEvent.data);

                    // Deserialize event data
                    SilKit::Util::SerDes::Deserializer deserializer(eventData);
                    deserializer.BeginStruct();
                    double lat = deserializer.Deserialize<double>();
                    double lon = deserializer.Deserialize<double>();
                    std::string signalQuality = deserializer.Deserialize<std::string>();
                    deserializer.EndStruct();

                    // Print results
                    std::cout << "<< Received Gps data lat=" << lat << ", lon=" << lon << ", signalQuality=" << signalQuality << std::endl;
                });

            SilKit::Services::PubSub::PubSubSpec dataSpecPubTemperature{"Temperature", mediaType};
            participant->CreateDataSubscriber(
                "Temperature", dataSpecPubTemperature,
                [](IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent) {
                    auto eventData = SilKit::Util::ToStdVector(dataMessageEvent.data);

                    // Deserialize event data
                    SilKit::Util::SerDes::Deserializer deserializer(eventData);
                    double temperature = deserializer.Deserialize<double>();

                    // Print results
                    std::cout << "<< Received temperature data temperature=" << temperature << std::endl;
                });

            timeSyncService->SetSimulationStepHandler(
                [](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                    std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                    std::this_thread::sleep_for(1s);
                }, 1s);
        }
        else
        {
            std::cout << "Wrong participant name provided. Use either \"Publisher\" or \"Subscriber\"." << std::endl;
            return 1;
        }

        auto lifecycleFuture =
            lifecycleService->StartLifecycle();
        auto finalState = lifecycleFuture.get();

        std::cout << "Simulation stopped. Final State: " << finalState << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
    }
    catch (const SilKit::ConfigurationError& error)
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
