// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <sstream>
#include <thread>

#include "ib/IntegrationBus.hpp"
#include "ib/sim/all.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp"

using namespace ib::mw;
using namespace ib::sim::data;
using namespace std::chrono_literals;

void PublishMessage(IDataPublisher* publisher, std::string topicname)
{
    static auto msgIdx = 0;

    std::stringstream messageBuilder;
    messageBuilder << topicname << " LocalMsgId=" << msgIdx++;
    auto message = messageBuilder.str();

    std::cout << "<< Send DataMessage with data=" << message << std::endl;

    std::vector<uint8_t> data{message.begin(), message.end()};
    publisher->Publish(std::move(data));
}

void ReceiveMessage(IDataSubscriber* subscriber, const std::vector<uint8_t>& data)
{
    std::string message{data.begin(), data.end()};
    std::cout << ">> Received new Message: with data=\"" << message << "\""
              << std::endl;
}

/**************************************************************************************************
 * Main Function
 **************************************************************************************************/

/*
PubSub1
Pub: "Topic1", DataExchangeFormat{"A"} -> Subscriber1
Pub: "Topic2", DataExchangeFormat{"A"} -> Subscriber1, Subscriber2
Sub: "Topic3", DataExchangeFormat{"A"} <- PubSub2

PubSub2
Pub: "Topic1", DataExchangeFormat{"A"} -> Subscriber1
Pub: "Topic3", DataExchangeFormat{"A"} -> PubSub1, PubSub2
Sub: "Topic3", DataExchangeFormat{"A"} <- PubSub2                           (self)

Subscriber1
Sub: "Topic1", DataExchangeFormat{""}  <- PubSub1, PubSub2                  (two pub)
Sub: "Topic2", DataExchangeFormat{"A"} <- PubSub1                           (single pub)

Subscriber2
Sub: "Topic2", DataExchangeFormat{"A"} <- PubSub1                           (single pub)
Sub: "Topic3", DataExchangeFormat{"B"} <- None                              (no match)


*/

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0] << " <ParticipantConfiguration.yaml|json> <ParticipantName> [domainId]" << std::endl;
        return -1;
    }

    try
    {
        std::string participantConfigurationFilename(argv[1]);
        std::string participantName(argv[2]);

        uint32_t domainId = 42;
        if (argc >= 4)
        {
            domainId = static_cast<uint32_t>(std::stoul(argv[3]));
        }

        auto participantConfiguration = ib::cfg::ParticipantConfigurationFromFile(participantConfigurationFilename);

        std::cout << "Creating DataAdapter for participant=" << participantName << " in domain " << domainId << std::endl;
        auto participant = ib::CreateSimulationParticipant(participantConfiguration, participantName, domainId, true);

        auto&& participantController = participant->GetParticipantController();
        participantController->SetInitHandler([&participantName](auto initCmd) {
            std::cout << "Initializing " << participantName << std::endl;
        });
        participantController->SetStopHandler([]() {
            std::cout << "Stopping..." << std::endl;
        });

        participantController->SetShutdownHandler([]() {
            std::cout << "Shutting down..." << std::endl;
        });

        participantController->SetPeriod(1s);
        if (participantName == "PubSub1")
        {
            auto* PubTopic1 = participant->CreateDataPublisher("Topic1", DataExchangeFormat{"A"}, {}, 0);
            auto* PubTopic2 = participant->CreateDataPublisher("Topic2", DataExchangeFormat{"A"}, {}, 0);
            auto* SubTopic3 = participant->CreateDataSubscriber("Topic3", DataExchangeFormat{"A"}, {}, ReceiveMessage);

            participantController->SetSimulationTask(
                [PubTopic1, PubTopic2](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                    std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                    PublishMessage(PubTopic1, "Topic1_from_Pub1");
                    PublishMessage(PubTopic2, "Topic2_from_Pub1");
                    std::this_thread::sleep_for(1s);

            });
        }
        else if (participantName == "PubSub2")
        {
            auto* PubTopic1 = participant->CreateDataPublisher("Topic1", DataExchangeFormat{"A"}, {}, 0);
            auto* PubTopic3 = participant->CreateDataPublisher("Topic3", DataExchangeFormat{"A"}, {}, 0);
            auto* SubTopic3 = participant->CreateDataSubscriber("Topic3", DataExchangeFormat{"A"}, {}, ReceiveMessage);

            participantController->SetSimulationTask(
                [PubTopic1, PubTopic3](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                    std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                    PublishMessage(PubTopic1, "Topic1_from_Pub2");
                    PublishMessage(PubTopic3, "Topic3_from_Pub2");
                    std::this_thread::sleep_for(1s);
                });
        }
        else if (participantName == "Subscriber1")
        {
            auto* SubTopic1 = participant->CreateDataSubscriber("Topic1", DataExchangeFormat{""}, {}, ReceiveMessage);
            auto* SubTopic2 = participant->CreateDataSubscriber("Topic2", DataExchangeFormat{"A"}, {}, ReceiveMessage);

            participantController->SetSimulationTask(
                [SubTopic1, SubTopic2](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                    std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                    std::this_thread::sleep_for(1s);
            });
        }
        else if (participantName == "Subscriber2")
        {
            auto* SubTopic2 = participant->CreateDataSubscriber("Topic2", DataExchangeFormat{"A"}, {}, ReceiveMessage);
            auto* SubTopic3 = participant->CreateDataSubscriber("Topic3", DataExchangeFormat{"B"}, {}, ReceiveMessage);

            participantController->SetSimulationTask(
                [SubTopic2, SubTopic3](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                    std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                    std::this_thread::sleep_for(1s);
                });
        }

        //auto finalStateFuture = participantController->RunAsync();
        //auto finalState = finalStateFuture.get();

        auto finalState = participantController->Run();

        std::cout << "Simulation stopped. Final State: " << finalState << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
    }
    catch (const ib::configuration_error& error)
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