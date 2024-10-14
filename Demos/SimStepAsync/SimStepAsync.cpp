// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <future>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <condition_variable>
#include <deque>

#include "silkit/SilKit.hpp"
#include "silkit/services/all.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/services/pubsub/PubSubSpec.hpp"
#include "silkit/util/serdes/Serialization.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"

#include "silkit/experimental/participant/ParticipantExtensions.hpp"


using namespace SilKit::Services::PubSub;
using namespace SilKit::Services::Orchestration;
using namespace std::chrono_literals;

/**************************************************************************************************
 * Main Function
 **************************************************************************************************/
struct CondVar
{
    std::condition_variable cv;
    std::mutex mx;
    std::deque<std::string> workItems;

    auto Lock() -> std::unique_lock<std::mutex>
    {
        return std::unique_lock<std::mutex>{mx};
    }

    void SignalWork(std::string newValue)
    {
        auto lock = Lock();
        workItems.emplace_back(std::move(newValue));
        cv.notify_one();
    }

    auto WaitForWork(std::unique_lock<std::mutex>& lock) -> std::string
    {
        cv.wait(lock, [this]() { return workItems.size() > 0; });
        auto workItem = std::move(workItems.front());
        workItems.pop_front();
        return workItem;
    }
};

void SimulatePublish(IDataPublisher* publisher, std::string workItem)
{
    // Serialize data
    const auto dataToSend = "Hello, world! time=" + workItem;
    SilKit::Util::SerDes::Serializer stringSerializer;
    stringSerializer.Serialize(dataToSend);

    // Publish serialized data
    publisher->Publish(stringSerializer.ReleaseBuffer());
}

// background worker thread and support
SilKit::Experimental::Services::Orchestration::ISystemController* systemController{nullptr};
CondVar condvar;
std::thread workerThread;
bool workerThreadDone{false};
void WorkerThreadMain(IDataPublisher* publisher, ITimeSyncService* timesyncService)
{
    while (!workerThreadDone)
    {
        auto lock = condvar.Lock();
        auto&& workItem = condvar.WaitForWork(lock);
        if (workItem == "stop")
            return;
        SimulatePublish(publisher, std::move(workItem)); //do some heavy lifting and parallel sending here
        timesyncService->CompleteSimulationStep();
    }
}

int main(int argc, char** argv)
try
{
    auto participantConfiguration = SilKit::Config::ParticipantConfigurationFromString("{}");

    const auto pubSubSpec = PubSubSpec{"Lines", SilKit::Util::SerDes::MediaTypeData()};

    auto registry = SilKit::Vendor::Vector::CreateSilKitRegistry(participantConfiguration);
    auto registryUri = registry->StartListening("silkit://localhost:0");


    // We are going to set up two participants:
    auto publisher = SilKit::CreateParticipant(participantConfiguration, "PublisherParticipant", registryUri);
    auto subscriber = SilKit::CreateParticipant(participantConfiguration, "SubscriberParticipant", registryUri);

    // Set up  Publisher
    auto* publisherLifecycle = publisher->CreateLifecycleService({OperationMode::Coordinated});
    publisherLifecycle->SetCommunicationReadyHandler([]() { std::cout << "Publisher ready." << std::endl; });
    publisherLifecycle->SetStopHandler([]() { std::cout << "Publisher Stop handler called." << std::endl; });
    auto* publisherService = publisher->CreateDataPublisher("SomeData", pubSubSpec, 0);
    auto* publisherTimesyncService = publisherLifecycle->CreateTimeSyncService();
    publisherTimesyncService->SetSimulationStepHandlerAsync(
        [publisherService](auto timestamp, auto) {
        condvar.SignalWork(std::to_string(timestamp.count()));
        }, 1ms);

    // Add a system controller, which is used to start the coordinated simulation
    systemController = SilKit::Experimental::Participant::CreateSystemController(publisher.get());
    systemController->SetWorkflowConfiguration({{"PublisherParticipant", "SubscriberParticipant"}});
 
    //Set up Subscriber
    auto* subscriberLifecycle = subscriber->CreateLifecycleService({OperationMode::Coordinated});
    subscriberLifecycle->SetCommunicationReadyHandler([]() { std::cout << "Subscriber ready." << std::endl; });
    subscriberLifecycle->SetStopHandler([]() { std::cout << "Subscriber Stop handler called." << std::endl; });
    auto* subscriberService = subscriber->CreateDataSubscriber("SomeData", pubSubSpec, 0);
    subscriberService->SetDataMessageHandler([](auto, auto& data) {
        // Deserialize event data
        auto eventData = SilKit::Util::ToStdVector(data.data);
        SilKit::Util::SerDes::Deserializer deserializer(eventData);
        auto message = deserializer.Deserialize<std::string>();
        std::cout << "Subscriber received data @"
                  << std::chrono::duration_cast<std::chrono::milliseconds>(data.timestamp).count() << "ms: \""
                  << message << "\"" << std::endl;
    });

    // Start the coordinated simulation using the SystemController API
    subscriberLifecycle->StartLifecycle();
    publisherLifecycle->StartLifecycle();

    workerThread = std::thread{WorkerThreadMain, publisherService, publisherTimesyncService};

    std::cin.ignore(); // wait for user input to stop the demo

    //clean up
    workerThreadDone = true;
    publisherLifecycle->Stop("Demo finished.");
    subscriberLifecycle->Stop("Demo finished.");
    systemController->AbortSimulation();
    condvar.SignalWork("stop");

    if (workerThread.joinable())
    {
        workerThread.join();
    }

    return 0;
}
catch (const std::exception& ex)
{
    if (systemController)
    {
        systemController->AbortSimulation();
    }
    std::cout << "Error: caught exception \"" << ex.what() << "\"" << std::endl;
    return 1;
}
