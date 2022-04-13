// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include <thread>
#include <string>
#include <chrono>
#include <exception>

#include "ib/IntegrationBus.hpp"
#include "ib/sim/all.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp" // string conversions for enums

using namespace ib::cfg;
using namespace ib::sim::generic;

const auto domainId = 123;

void publisher_main(Config config)
{
    auto participant = ib::CreateParticipant(std::move(config), "PublisherParticipant", domainId);
    auto* publisher = participant->CreateGenericPublisher("DataService");
    auto* participantCtrl = participant->GetParticipantController();

    participantCtrl->SetSimulationTask(
        [publisher, participantCtrl](std::chrono::nanoseconds now) {
            static auto msgIdx = 0;

            //generate some random data
            std::string message = "DataService Msg" + std::to_string(msgIdx++);
            std::vector<uint8_t> data{ message.begin(), message.end() };

            //publish the raw bytes of the message to all subscribers
            publisher->Publish(std::move(data)); 
    });
    //run the simulation main loop forever
    try {
        auto result = participantCtrl->Run();
        std::cout << "Publisher: result: " << result << std::endl;
    }
    catch (const std::exception &e) {
        std::cout << "ERROR: Publisher exception caught: " << e.what() << std::endl;
    }
}

void subscriber_main(Config config)
{
    auto participant = ib::CreateParticipant(std::move(config), "SubscriberParticipant", domainId);
    auto* subscriber = participant->CreateGenericSubscriber("DataService");
    auto* participantCtrl = participant->GetParticipantController();

    //Register callback for reception of messages
    subscriber->SetReceiveMessageHandler(
        [participantCtrl](IGenericSubscriber* subscriber, const std::vector<uint8_t>& data) {
            using namespace std::chrono;
            std::string message{ data.begin(), data.end() };
            std::cout << " <- Received data=\"" << message << "\"" << std::endl;
    });

    participantCtrl->SetSimulationTask(
        [](std::chrono::nanoseconds) {
            //simulation task must be defined, even an empty one
    });

    try {
        auto result = participantCtrl->Run();
        std::cout << "Subscriber: result: " << result << std::endl;
    }
    catch (const std::exception& e) {
        std::cout << "ERROR: Subscriber exception caught: " << e.what() << std::endl;
    }
}

int main(int argc, char** argv)
{
    auto config = Config::FromJsonFile("simple.json");
    std::thread publisher { publisher_main, config };
    std::thread subscriber{ subscriber_main, config };

    if (subscriber.joinable()) subscriber.join();
    if (publisher.joinable()) publisher.join();

    return 0;
}
