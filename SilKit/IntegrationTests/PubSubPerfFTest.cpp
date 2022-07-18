// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>

#include "silkit/services/all.hpp"
#include "silkit/util/functional.hpp"
#include "silkit/services/logging/ILogger.hpp"

#include "SimTestHarness.hpp"
#include "GetTestPid.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using namespace std::chrono_literals;

auto Now()
{
    auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now);
}

class PubSubPerfFTest : public testing::Test
{
protected:

    PubSubPerfFTest()
    {
    }

    void ExecuteTest(int numberOfTopics, std::chrono::seconds timeout)
    {
        auto start = Now();

        std::vector<std::string> syncParticipantNames = {"Publisher", "Subscriber"};
        auto registryUri = MakeTestRegistryUri();
        SilKit::Tests::SimTestHarness testHarness(syncParticipantNames, registryUri, true);

        // Subscriber
        auto&& subscriber = testHarness.GetParticipant("Subscriber");
        auto subLogger = subscriber->GetOrCreateLogger();
        subLogger->Info(">>> Created Subscriber participant");

        int receptionCount = 0;
        auto* subLifecycleService = subscriber->GetOrCreateLifecycleServiceWithTimeSync();
        for (auto i = 0; i < numberOfTopics; i++)
        {
            const auto controllerName = "Sub-" + std::to_string(i);
            const auto topic = "TopicName-" + std::to_string(i);
            (void)subscriber->Participant()->CreateDataSubscriber(
                controllerName, topic, "", {},
                [&receptionCount, subLogger, numberOfTopics, &subLifecycleService](
                    SilKit::Services::PubSub::IDataSubscriber* /*subscriber*/, const SilKit::Services::PubSub::DataMessageEvent& /*data*/) {
                    receptionCount++;
                    if (receptionCount == numberOfTopics)
                    {
                        subLogger->Info(">>> Reception complete");
                        subLifecycleService->Stop("Reception complete");
                    }
                });
        }
        subLogger->Info(">>> Created DataSubscriber controllers");

        // Publisher
        auto&& publisher = testHarness.GetParticipant("Publisher");
        auto pubLogger = publisher->GetOrCreateLogger();
        pubLogger->Info(">>> Created Publisher participant");
        std::vector<SilKit::Services::PubSub::IDataPublisher*> pubController;
        std::vector<uint8_t> testData = {1, 1, 1};
        pubController.reserve(numberOfTopics);
        bool allPublished = false;
        for (auto i = 0; i < numberOfTopics; i++)
        {
            const auto controllerName = "Pub-" + std::to_string(i);
            const auto topic = "TopicName-" + std::to_string(i);
            pubController.push_back(publisher->Participant()->CreateDataPublisher(controllerName, topic, "", {}, 0));
        }
        auto* lifecycleService = publisher->GetOrCreateLifecycleServiceWithTimeSync();
        auto* timeSyncService = lifecycleService->GetTimeSyncService();
        timeSyncService->SetSimulationStepHandler(
            [&allPublished, testData, pubController, pubLogger](auto, auto) {
                if (!allPublished)
                {
                    pubLogger->Info(">>> Start to publish");
                    for (auto p : pubController)
                    {
                        p->Publish(testData);
                    }
                    pubLogger->Info(">>> Publish complete");
                    allPublished = true;
                }
            }, 1ms);
        pubLogger->Info(">>> Created DataPublisher controllers");

        pubLogger->Info(">>> Run");
        testHarness.Run(timeout);

        std::chrono::duration<double> duration = Now() - start;
        std::cout << numberOfTopics << " " << duration.count() << std::endl;
    }
};


TEST_F(PubSubPerfFTest, test_pubsub_performance)
{
    std::cout << "#NumberOfTopics Runtime(s)" << std::endl;
    ExecuteTest(1, 10s);
    ExecuteTest(10, 10s);
    ExecuteTest(100, 10s);
    //ExecuteTest(1000, 10s);
    //ExecuteTest(10000, 100s);
    //ExecuteTest(100000, 100s);
    //ExecuteTest(200000, 100s);
    //ExecuteTest(500000, 100s);
    //ExecuteTest(1000000, 100s);

}

} // anonymous namespace
