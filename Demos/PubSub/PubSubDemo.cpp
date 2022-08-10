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
#include "silkit/services/pubsub/PubSubSpec.hpp"


using namespace SilKit::Services::PubSub;
using namespace std::chrono_literals;

void PublishMessage(IDataPublisher* publisher, std::string topicname)
{
    static auto msgIdx = 0;

    std::stringstream messageBuilder;
    messageBuilder << topicname << " LocalMsgId=" << msgIdx++;
    auto message = messageBuilder.str();

    std::cout << "<< Send DataMessageEvent with data=" << message << std::endl;

    SilKit::Util::SerDes::Serializer serializer;
    serializer.Serialize(message);
    publisher->Publish(serializer.ReleaseBuffer());
}

void ReceiveMessage(IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent)
{
    SilKit::Util::SerDes::Deserializer deserializer(SilKit::Util::ToStdVector(dataMessageEvent.data));
    const auto message = deserializer.Deserialize<std::string>();
    std::cout << ">> Received new Message: with data=\"" << message << "\"" << std::endl;
}

/**************************************************************************************************
 * Main Function
 **************************************************************************************************/

/*
PubSub1
Pub: "Topic1", "A"                                  -> Subscriber1
Pub: "Topic2", "A"                                  -> Subscriber1, Subscriber2
Sub: "Topic3", "A"                                  <- PubSub2
Sub: "Topic4", "A", {"KeyA", "ValA", Mandatory}     <- PubSub2

PubSub2
Pub: "Topic1", "A"                                  -> Subscriber1
Pub: "Topic3", "A"                                  -> PubSub1, PubSub2
Sub: "Topic3", "A"                                  <- PubSub2                        
Pub: "Topic4", "A", {"KeyA", "ValA", Optional}      -> PubSub1

Subscriber1
Sub: "Topic1", ""                                   <- PubSub1, PubSub2               
Sub: "Topic2", "A"                                  <- PubSub1                        
Sub: "Topic4", "A"                                  <- PubSub2

Subscriber2
Sub: "Topic2", "A"                                  <- PubSub1                        
Sub: "Topic3", "B"                                  <- None                           

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

    std::string mediaTypeWildcard{""};
    std::string mediaTypeJson{"application/json"};
    std::string mediaTypeText{"text/plain"};

    std::map<std::string, std::string> labelsEmpty;

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

        if (participantName == "PubSub1")
        {
            SilKit::Services::PubSub::PubSubSpec dataSpecPub1{"Topic1", mediaTypeJson};
            auto* PubTopic1 = participant->CreateDataPublisher("PubCtrl1", dataSpecPub1, 0);

            SilKit::Services::PubSub::PubSubSpec dataSpecPub2{"Topic2", mediaTypeJson};
            auto* PubTopic2 = participant->CreateDataPublisher("PubCtrl2", dataSpecPub2, 0);

            SilKit::Services::PubSub::PubSubSpec dataSpecSub1{"Topic3", mediaTypeJson};
            participant->CreateDataSubscriber("SubCtrl1", dataSpecSub1, ReceiveMessage);

            SilKit::Services::PubSub::PubSubSpec dataSpecSub2{"Topic4", mediaTypeJson};
            dataSpecSub2.AddLabel("KeyA", "ValA", SilKit::Services::MatchingLabel::Kind::Mandatory);
            participant->CreateDataSubscriber("SubCtrl2", dataSpecSub2, ReceiveMessage);

            timeSyncService->SetSimulationStepHandler(
                [PubTopic1, PubTopic2](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                    std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                    PublishMessage(PubTopic1, "Topic1_from_Pub1");
                    PublishMessage(PubTopic2, "Topic2_from_Pub1");
                    std::this_thread::sleep_for(1s);

            }, 1s);
        }
        else if (participantName == "PubSub2")
        {

            SilKit::Services::PubSub::PubSubSpec dataSpecPub1{"Topic1", mediaTypeJson};
            auto* PubTopic1 = participant->CreateDataPublisher("PubCtrl1", dataSpecPub1, 0);

            SilKit::Services::PubSub::PubSubSpec dataSpecPub2{"Topic3", mediaTypeJson};
            auto* PubTopic3 = participant->CreateDataPublisher("PubCtrl2", dataSpecPub2, 0);

            SilKit::Services::PubSub::PubSubSpec dataSpecSub1{"Topic3", mediaTypeJson};
            participant->CreateDataSubscriber("SubCtrl1", dataSpecSub1, ReceiveMessage);

            SilKit::Services::PubSub::PubSubSpec dataSpecPub3{"Topic4", mediaTypeJson};
            dataSpecPub3.AddLabel("KeyA", "ValA", SilKit::Services::MatchingLabel::Kind::Optional);
            auto* PubTopic4 = participant->CreateDataPublisher("PubCtrl3", dataSpecPub3, 0);

            timeSyncService->SetSimulationStepHandler(
                [PubTopic1, PubTopic3, PubTopic4](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                    std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                    PublishMessage(PubTopic1, "Topic1_from_Pub2");
                    PublishMessage(PubTopic3, "Topic3_from_Pub2");
                    PublishMessage(PubTopic4, "Topic4_from_Pub2");
                    std::this_thread::sleep_for(1s);
                }, 1s);
        }
        else if (participantName == "Subscriber1")
        {
            SilKit::Services::PubSub::PubSubSpec dataSpecSub1{"Topic1", mediaTypeWildcard};
            participant->CreateDataSubscriber("SubCtrl1", dataSpecSub1, ReceiveMessage);

            SilKit::Services::PubSub::PubSubSpec dataSpecSub2{"Topic2", mediaTypeJson};
            participant->CreateDataSubscriber("SubCtrl2", dataSpecSub2, ReceiveMessage);

            SilKit::Services::PubSub::PubSubSpec dataSpecSub3{"Topic4", mediaTypeJson};
            participant->CreateDataSubscriber("SubCtrl3", dataSpecSub3, ReceiveMessage);

            timeSyncService->SetSimulationStepHandler(
                [](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                    std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                    std::this_thread::sleep_for(1s);
            }, 1s);
        }
        else if (participantName == "Subscriber2")
        {
            SilKit::Services::PubSub::PubSubSpec dataSpecSub1{"Topic2", mediaTypeJson};
            participant->CreateDataSubscriber("SubCtrl1", dataSpecSub1, ReceiveMessage);

            SilKit::Services::PubSub::PubSubSpec dataSpecSub2{"Topic3", mediaTypeText};
            participant->CreateDataSubscriber("SubCtrl2", dataSpecSub2, ReceiveMessage);

            timeSyncService->SetSimulationStepHandler(
                [](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                    std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                    std::this_thread::sleep_for(1s);
                }, 1s);
        }
        else
        {
            std::cout << "Wrong participant name provided. Use either \"PubSub1\", \"PubSub2\", \"Subscriber1\" or \"Subscriber2\"." << std::endl;
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
