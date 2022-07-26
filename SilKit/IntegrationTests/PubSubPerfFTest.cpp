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

#include "silkit/services/all.hpp"
#include "functional.hpp"
#include "silkit/services/logging/ILogger.hpp"
#include "silkit/services/pubsub/DataSpec.hpp"

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
            SilKit::Services::PubSub::DataSubscriberSpec dataSpec{topic, ""};

            (void)subscriber->Participant()->CreateDataSubscriber(controllerName, dataSpec,
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
            SilKit::Services::PubSub::DataPublisherSpec dataSpec{topic, ""};

            pubController.push_back(publisher->Participant()->CreateDataPublisher(controllerName, dataSpec, 0));
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
    ExecuteTest(1000, 10s);
    //ExecuteTest(10000, 100s);
    //ExecuteTest(100000, 100s);
    //ExecuteTest(200000, 100s);
    //ExecuteTest(500000, 100s);
    //ExecuteTest(1000000, 100s);

}

} // anonymous namespace
