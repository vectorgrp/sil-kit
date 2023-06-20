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

struct GpsData
{
    double latitude;
    double longitude;
    std::string signalQuality;
};

std::vector<uint8_t> Serialize(const GpsData& gpsData)
{
    // Serialize data
    SilKit::Util::SerDes::Serializer serializer;
    serializer.BeginStruct();
    serializer.Serialize(gpsData.latitude);
    serializer.Serialize(gpsData.longitude);
    serializer.Serialize(gpsData.signalQuality);
    serializer.EndStruct();

    return serializer.ReleaseBuffer();
}

GpsData Deserialize(const std::vector<uint8_t>& data)
{
    GpsData gpsData;

    // Deserialize event data
    SilKit::Util::SerDes::Deserializer deserializer(data);
    deserializer.BeginStruct();
    gpsData.latitude = deserializer.Deserialize<double>();
    gpsData.longitude = deserializer.Deserialize<double>();
    gpsData.signalQuality = deserializer.Deserialize<std::string>();
    deserializer.EndStruct();

    return gpsData;
}

void PublishData(SilKit::Services::PubSub::IDataPublisher* gpsPublisher,
                 SilKit::Services::PubSub::IDataPublisher* temperaturePublisher)
{
    // GPS
    GpsData gpsData;
    gpsData.latitude = 48.8235 + static_cast<double>((rand() % 150)) / 100000;
    gpsData.longitude = 9.0965 + static_cast<double>((rand() % 150)) / 100000;
    gpsData.signalQuality = "Strong";

    // Serialize data
    auto gpsSerialized = Serialize(gpsData);

    // Publish serialized data
    gpsPublisher->Publish(gpsSerialized);
    std::cout << ">> Published GPS data: lat=" << gpsData.latitude << ", lon=" << gpsData.longitude
              << ", signalQuality=" << gpsData.signalQuality << std::endl;

    // Temperature
    double temperature = 25.0 + static_cast<double>(rand() % 10) / 10.0;

    // Serialize data
    SilKit::Util::SerDes::Serializer temperatureSerializer;
    temperatureSerializer.Serialize(temperature);

    // Publish serialized data
    temperaturePublisher->Publish(temperatureSerializer.ReleaseBuffer());
    std::cout << ">> Published temperature data: temperature=" << temperature << std::endl;
}

void ReceiveGpsData(IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent)
{
    auto eventData = SilKit::Util::ToStdVector(dataMessageEvent.data);
    auto gpsData = Deserialize(eventData);

    // Print results
    std::cout << "<< Received GPS data: lat=" << gpsData.latitude << ", lon=" << gpsData.longitude
              << ", signalQuality=" << gpsData.signalQuality << std::endl;
}

void ReceiveTemperatureData(IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent)
{
    auto eventData = SilKit::Util::ToStdVector(dataMessageEvent.data);

    // Deserialize event data
    SilKit::Util::SerDes::Deserializer deserializer(eventData);
    double temperature = deserializer.Deserialize<double>();

    // Print results
    std::cout << "<< Received temperature data: temperature=" << temperature << std::endl;
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0]
                  << " <ParticipantConfiguration.yaml|json> <ParticipantName> [RegistryUri] [--async]" << std::endl
                  << "Use \"Publisher\" or \"Subscriber\" as <ParticipantName>." << std::endl;
        return -1;
    }

    std::string participantName(argv[2]);

    if (participantName != "Publisher" && participantName != "Subscriber")
    {
        std::cout << "Wrong participant name provided. Use either \"Publisher\" or \"Subscriber\"."
                  << std::endl;
        return 1;
    }

    std::string mediaType{SilKit::Util::SerDes::MediaTypeData()};
    SilKit::Services::PubSub::PubSubSpec dataSpecPubGps{"Gps", mediaType};
    SilKit::Services::PubSub::PubSubSpec dataSpecPubTemperature{"Temperature", mediaType};

    try
    {
        std::string participantConfigurationFilename(argv[1]);

        std::string registryUri = "silkit://localhost:8500";

        bool runSync = true;

        std::vector<std::string> args;
        std::copy((argv + 3), (argv + argc), std::back_inserter(args));

        for (auto&& arg : args)
        {
            if (arg == "--async")
            {
                runSync = false;
            }
            else
            {
                registryUri = arg;
            }
        }

        auto participantConfiguration = SilKit::Config::ParticipantConfigurationFromFile(participantConfigurationFilename);

        std::cout << "Creating participant '" << participantName << "' with registry " << registryUri << std::endl;
        auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryUri);
        auto operationMode = (runSync ? SilKit::Services::Orchestration::OperationMode::Coordinated
                              : SilKit::Services::Orchestration::OperationMode::Autonomous);
        auto* lifecycleService = participant->CreateLifecycleService({operationMode});
        lifecycleService->SetStopHandler([]() {
            std::cout << "Stop handler called" << std::endl;
        });

        lifecycleService->SetShutdownHandler([]() {
            std::cout << "Shutdown handler called" << std::endl;
        });

        auto isPublisher = (participantName == "Publisher");

        // Create a data publisher for GPS data
        auto* gpsPublisher = (isPublisher
                              ? participant->CreateDataPublisher("GpsPublisher", dataSpecPubGps, 0)
                              : nullptr);
        // Create a data publisher for temperature data
        auto* temperaturePublisher = (
            isPublisher
            ? participant->CreateDataPublisher("TemperaturePublisher", dataSpecPubTemperature, 0)
            : nullptr);

        if (!isPublisher)
        {
            participant->CreateDataSubscriber(
                "GpsSubscriber", dataSpecPubGps,
                &ReceiveGpsData);
            participant->CreateDataSubscriber(
                "TemperatureSubscriber", dataSpecPubTemperature,
                &ReceiveTemperatureData);
        }

        if (runSync)
        {
            auto* timeSyncService = lifecycleService->CreateTimeSyncService();

            lifecycleService->SetCommunicationReadyHandler([&participantName]() {
                std::cout << "Communication ready handler called for " << participantName << std::endl;
            });

            if (isPublisher)
            {
                timeSyncService->SetSimulationStepHandler(
                    [gpsPublisher, temperaturePublisher](std::chrono::nanoseconds now,
                                                         std::chrono::nanoseconds /*duration*/) {
                        auto nowMs = 
                            std::chrono::duration_cast<std::chrono::milliseconds>(now);
                        std::cout << "now=" << nowMs.count() << "ms" << std::endl;

                        if (gpsPublisher && temperaturePublisher)
                        {
                            PublishData(gpsPublisher, temperaturePublisher);
                        }
                        std::this_thread::sleep_for(1s);
                    },
                    1s);
            }
            else
            {
                timeSyncService->SetSimulationStepHandler(
                    [](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                        auto nowMs = 
                            std::chrono::duration_cast<std::chrono::milliseconds>(now);
                        std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                        std::this_thread::sleep_for(1s);
                    },
                    1s);
            }

            auto lifecycleFuture = lifecycleService->StartLifecycle();
            auto finalState = lifecycleFuture.get();

            std::cout << "Simulation stopped. Final State: " << finalState << std::endl;
            std::cout << "Press enter to stop the process..." << std::endl;
            std::cin.ignore();
        }
        else
        {
            bool isStopped = false;
            std::thread workerThread;
            std::promise<void> startHandlerPromise;
            auto startHandlerFuture = startHandlerPromise.get_future();
            lifecycleService->SetCommunicationReadyHandler([&]() {
                std::cout << "Communication ready handler called for " << participantName << std::endl;
                workerThread = std::thread{[&]() {
                    startHandlerFuture.get();
                    while (!isStopped)
                    {
                        if (gpsPublisher && temperaturePublisher)
                        {
                            PublishData(gpsPublisher, temperaturePublisher);
                        }
                        std::this_thread::sleep_for(1s);
                    }
                    lifecycleService->Stop("User Requested to Stop");
                }};
            });

            lifecycleService->SetStartingHandler([&]() {
                startHandlerPromise.set_value();
            });

            auto finalStateFuture = lifecycleService->StartLifecycle();
            std::cout << "Press enter to stop the process..." << std::endl;
            std::cin.ignore();

            isStopped = true;
            if (workerThread.joinable())
            {
                workerThread.join();
            }
            auto finalState = finalStateFuture.get();
            std::cout << "Simulation stopped. Final State: " << finalState << std::endl;
        }
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
