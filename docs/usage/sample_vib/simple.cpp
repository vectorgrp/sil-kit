// Copyright (c) Vector Informatik GmbH. All rights reserved.
#include <thread>
#include <string>
#include <chrono>
#include <exception>

#include "ib/IntegrationBus.hpp"
#include "ib/sim/all.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/mw/sync/string_utils.hpp" // string conversions for enums

const auto domainId = 42;

void publisher_main(std::shared_ptr<ib::cfg::IParticipantConfiguration> config)
{
    auto participant = ib::CreateParticipant(config, "PublisherParticipant", domainId, true);
    auto* publisher = participant->CreateDataPublisher("DataService");
    auto* participantCtrl = participant->GetParticipantController();

    participantCtrl->SetSimulationTask([publisher](std::chrono::nanoseconds now) {
        static auto msgIdx = 0;

        //generate some random data
        std::string message = "DataService Msg" + std::to_string(msgIdx++);
        std::vector<uint8_t> data{message.begin(), message.end()};

        //publish the raw bytes of the message to all subscribers
        publisher->Publish(std::move(data));
    });
    //run the simulation main loop forever
    try
    {
        auto result = participantCtrl->Run();
        std::cout << "Publisher: result: " << result << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "ERROR: Publisher exception caught: " << e.what() << std::endl;
    }
}

void subscriber_main(std::shared_ptr<ib::cfg::IParticipantConfiguration> config)
{
    auto participant = ib::CreateParticipant(config, "SubscriberParticipant", domainId, true);
    auto* subscriber = participant->CreateDataSubscriber("DataService");
    auto* participantCtrl = participant->GetParticipantController();

    //Register callback for reception of messages
    subscriber->SetDefaultDataMessageHandler(
        [](ib::sim::data::IDataSubscriber* subscriber, const ib::sim::data::DataMessageEvent& dataMessageEvent) {
            std::string message{dataMessageEvent.data.begin(), dataMessageEvent.data.end()};
            std::cout << " <- Received data=\"" << message << "\"" << std::endl;
        });

    participantCtrl->SetSimulationTask([](std::chrono::nanoseconds) {
        //simulation task must be defined, even an empty one
    });

    try
    {
        auto result = participantCtrl->Run();
        std::cout << "Subscriber: result: " << result << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "ERROR: Subscriber exception caught: " << e.what() << std::endl;
    }
}

int main(int argc, char** argv)
{
    auto config = ib::cfg::ParticipantConfigurationFromFile("simple.yaml");
    std::thread publisher{publisher_main, config};
    std::thread subscriber{subscriber_main, config};

    if (subscriber.joinable())
        subscriber.join();
    if (publisher.joinable())
        publisher.join();

    return 0;
}
