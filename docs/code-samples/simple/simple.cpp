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
#include <thread>

#include "silkit/SilKit.hpp"
#include "silkit/services/all.hpp"
#include "silkit/services/orchestration/string_utils.hpp"

using namespace std::chrono_literals;

const auto registryUri = "silkit://localhost:8500";

void publisher_main(std::shared_ptr<SilKit::Config::IParticipantConfiguration> config)
{
    auto participant = SilKit::CreateParticipant(config, "PublisherParticipant", registryUri);

    auto* lifecycleService =
        participant->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated});

    SilKit::Services::PubSub::PubSubSpec pubSubSpec{"DataService", "text/plain"};
    auto* publisher = participant->CreateDataPublisher("PublisherController", pubSubSpec);

    auto* timeSyncService = lifecycleService->CreateTimeSyncService();
    timeSyncService->SetSimulationStepHandler(
        [publisher](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
            // Generate some data
            static auto msgIdx = 0;
            std::string message = "DataService Msg" + std::to_string(msgIdx++);
            std::vector<uint8_t> data{message.begin(), message.end()};

            // Publish the raw bytes of the message to all subscribers
            publisher->Publish(std::move(data));

            // Delay the simulation
            std::this_thread::sleep_for(1s);
        },
        1ms);

    try
    {
        // Run the simulation main loop until stopped by the sil-kit-system-controller
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

    auto* lifecycleService =
        participant->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated});

    SilKit::Services::PubSub::PubSubSpec pubSubSpec{"DataService", "text/plain"};
    auto receptionHandler = [](auto* subscriber, const auto& dataMessageEvent) {
        std::string message{dataMessageEvent.data.begin(), dataMessageEvent.data.end()};
        std::cout << " <- Received data=\"" << message << "\"" << std::endl;
    };
    auto* subscriber = participant->CreateDataSubscriber("SubscriberController", pubSubSpec, receptionHandler);

    auto* timeSyncService = lifecycleService->CreateTimeSyncService();
    timeSyncService->SetSimulationStepHandler(
        [](std::chrono::nanoseconds /*now*/, std::chrono::nanoseconds /*duration*/) {
            // Simulation task must be defined, even an empty one
        },
        1ms);

    try
    {
        // Run the simulation main loop until stopped by the sil-kit-system-controller
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
    try
    {
        // Load the YAML configuration
        auto config = SilKit::Config::ParticipantConfigurationFromFile("simple.yaml");

        // Launch the participant threads
        std::thread publisher{publisher_main, config};
        std::thread subscriber{subscriber_main, config};

        // Once finished, close the threads
        if (subscriber.joinable())
            subscriber.join();
        if (publisher.joinable())
            publisher.join();
    }
    catch (const std::exception& e)
    {
        std::cout << "ERROR: Exception caught: " << e.what() << std::endl;
    }

    return 0;
}
