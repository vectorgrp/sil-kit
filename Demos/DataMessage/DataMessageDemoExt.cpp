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
#include "silkit/util/serdes/sil/Serialization.hpp"
#include "silkit/services/pubsub/DataSpec.hpp"


using namespace SilKit::Services::PubSub;
using namespace std::chrono_literals;


std::ostream& operator<<(std::ostream& os, const std::map<std::string, std::string>& m)
{
    os << "{ ";
    for (auto&& kvp : m)
        os << "{ " << kvp.first << ", " << kvp.second << " } "; 
    os << "}";

    return os;
}

void PublishMessage(IDataPublisher* publisher, std::string msg)
{
    static auto msgIdx = 0;

    std::stringstream messageBuilder;
    messageBuilder << msg << " LocalMsgId=" << msgIdx++;
    auto message = messageBuilder.str();

    std::cout << "<< Send DataMessageEvent with data=" << message << std::endl;

    SilKit::Util::SerDes::Serializer serializer;
    serializer.Serialize(message);
    publisher->Publish(serializer.ReleaseBuffer());
}

void DefaultDataHandler(IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent)
{
    SilKit::Util::SerDes::Deserializer deserializer(SilKit::Util::ToStdVector(dataMessageEvent.data));
    const auto message = deserializer.Deserialize<std::string>();
    std::cout << ">> [DefaultDataHandler] Received new Message: with data=\""
              << message << "\"" << std::endl;
}

void SpecificDataHandlerForPub1(IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent)
{
    SilKit::Util::SerDes::Deserializer deserializer(SilKit::Util::ToStdVector(dataMessageEvent.data));
    const auto message = deserializer.Deserialize<std::string>();
    std::cout << ">> [SpecificDataHandlerForPublisher1] Received new Message: with data=\""
              << message << std::endl;
}

void SpecificDataHandlerForPub2(IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent)
{
    SilKit::Util::SerDes::Deserializer deserializer(SilKit::Util::ToStdVector(dataMessageEvent.data));
    const auto message = deserializer.Deserialize<std::string>();
    std::cout << ">> [SpecificDataHandlerForPublisher2] Received new Message: with data=\""
              << message << "\"" << std::endl;
}

void NewDataSource(IDataSubscriber* /*subscriber*/, const NewDataPublisherEvent& dataSource)
{
    std::cout << ">> New data source: topic=" << dataSource.topic << " mediaType=" << dataSource.mediaType
              << " labels=" << dataSource.labels << "" << std::endl;
}

/**************************************************************************************************
 * Main Function
 **************************************************************************************************/

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0] << " <ParticipantConfiguration.yaml|json> <ParticipantName> [RegistryUri]" << std::endl;
        return -1;
    }

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

        // Set an Init Handler
        auto* lifecycleService = participant->CreateLifecycleService();
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

        const std::string mediaType{ SilKit::Util::SerDes::MediaTypeData() };

        if (participantName == "Publisher1")
        {
            SilKit::Services::PubSub::DataPublisherSpec dataSpec{"Topic1", mediaType};
            dataSpec.AddLabel("KeyA", "ValA");
            dataSpec.AddLabel("KeyB", "ValB");

            auto* publisher = participant->CreateDataPublisher("PubCtrl1", dataSpec, 0);

            timeSyncService->SetSimulationStepHandler(
                [publisher](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                    std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                    PublishMessage(publisher, "Pub1 on Topic1");
                    std::this_thread::sleep_for(1s);

            }, 1s);
        }
        else if (participantName == "Publisher2")
        {
            SilKit::Services::PubSub::DataPublisherSpec dataSpec{"Topic1", mediaType};
            dataSpec.AddLabel("KeyB", "ValB");
            dataSpec.AddLabel("KeyC", "ValC");

            auto* publisher = participant->CreateDataPublisher("PubCtrl1", dataSpec, 0);

            timeSyncService->SetSimulationStepHandler(
                [publisher](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                    std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                    PublishMessage(publisher, "Pub2 on Topic1");
                    std::this_thread::sleep_for(1s);
                }, 1s);
        }
        else //if (participantName == "Subscriber")
        {
            SilKit::Services::PubSub::DataSubscriberSpec dataSpec{"Topic1", mediaType};
            participant->CreateDataSubscriber("SubCtrl1", dataSpec, DefaultDataHandler);

            timeSyncService->SetSimulationStepHandler(
                [](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                    std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                }, 1s);
        }

        auto lifecycleFuture = lifecycleService->StartLifecycle({SilKit::Services::Orchestration::OperationMode::Coordinated});
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
