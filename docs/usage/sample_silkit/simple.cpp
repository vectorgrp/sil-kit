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

#include <thread>
#include <string>
#include <chrono>
#include <exception>

#include "silkit/SilKit.hpp"
#include "silkit/services/all.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/orchestration/string_utils.hpp" // string conversions for enums

using namespace std::chrono_literals;

const auto registryUri = "silkit://localhost:8500";

void publisher_main(std::shared_ptr<SilKit::Config::IParticipantConfiguration> config)
{
    auto participant = SilKit::CreateParticipant(config, "PublisherParticipant", registryUri);
    SilKit::Services::PubSub::PubSubSpec pubSubSpec{"DataService", "text/plain"};
    auto* publisher = participant->CreateDataPublisher("PublisherController", pubSubSpec);
    auto* lifecycleService =
        participant->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated});
    auto* timeSyncService = lifecycleService->CreateTimeSyncService();

    timeSyncService->SetSimulationStepHandler([publisher](std::chrono::nanoseconds now) {
        static auto msgIdx = 0;

        //generate some random data
        std::string message = "DataService Msg" + std::to_string(msgIdx++);
        std::vector<uint8_t> data{message.begin(), message.end()};

        //publish the raw bytes of the message to all subscribers
        publisher->Publish(std::move(data));
    }, 1ms);
    //run the simulation main loop forever
    try
    {
        auto result = lifecycleService->StartLifecycle();
        std::cout << "Publisher: result: " << result.get() << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "ERROR: Publisher exception caught: " << e.what() << std::endl;
    }
}

void subscriber_main(std::shared_ptr<SilKit::Config::IParticipantConfiguration> config)
{
    auto participant = SilKit::CreateParticipant(config, "SubscriberParticipant", registryUri);
    
    SilKit::Services::PubSub::PubSubSpec pubSubSpec{"DataService", "text/plain"};

    auto receptionHandler = [](auto* subscriber, const auto& dataMessageEvent) {
        std::string message{dataMessageEvent.data.begin(), dataMessageEvent.data.end()};
        std::cout << " <- Received data=\"" << message << "\"" << std::endl;
    };
    auto* subscriber = participant->CreateDataSubscriber("SubscriberController", pubSubSpec, receptionHandler);

    auto* lifecycleService =
        participant->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated});
    auto* timeSyncService = lifecycleService->CreateTimeSyncService();

    timeSyncService->SetSimulationStepHandler([](std::chrono::nanoseconds) {
        //simulation task must be defined, even an empty one
    }, 1ms);

    try
    {
        auto result = lifecycleService->StartLifecycle();
        std::cout << "Subscriber: result: " << result.get() << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "ERROR: Subscriber exception caught: " << e.what() << std::endl;
    }
}

int main(int argc, char** argv)
{
    auto config = SilKit::Config::ParticipantConfigurationFromFile("simple.yaml");
    std::thread publisher{publisher_main, config};
    std::thread subscriber{subscriber_main, config};

    if (subscriber.joinable())
        subscriber.join();
    if (publisher.joinable())
        publisher.join();

    return 0;
}
