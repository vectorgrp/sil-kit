// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>

#include "ib/sim/all.hpp"
#include "ib/util/functional.hpp"
#include "ib/mw/logging/ILogger.hpp"

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
        auto domainId = static_cast<uint32_t>(GetTestPid());
        ib::test::SimTestHarness testHarness(syncParticipantNames, domainId, true);

        // Subscriber
        auto&& subscriber = testHarness.GetParticipant("Subscriber")->Participant();
        auto subLogger = subscriber->GetLogger();
        subLogger->Info(">>> Created Subscriber participant");

        int receptionCount = 0;
        auto&& subParticipantController = subscriber->GetParticipantController();
        for (auto i = 0; i < numberOfTopics; i++)
        {
            const auto controllerName = "Sub-" + std::to_string(i);
            const auto topic = "TopicName-" + std::to_string(i);
            (void)subscriber->CreateDataSubscriber(
                controllerName, topic, "", {},
                [&receptionCount, subLogger, numberOfTopics, subParticipantController](
                    ib::sim::data::IDataSubscriber* /*subscriber*/, const ib::sim::data::DataMessageEvent& /*data*/) {
                    receptionCount++;
                    if (receptionCount == numberOfTopics)
                    {
                        subLogger->Info(">>> Reception complete");
                        subParticipantController->Stop("Reception complete");
                    }
                });
        }
        subLogger->Info(">>> Created DataSubscriber controllers");

        // Publisher
        auto&& publisher = testHarness.GetParticipant("Publisher")->Participant();
        auto pubLogger = publisher->GetLogger();
        pubLogger->Info(">>> Created Publisher participant");
        std::vector<ib::sim::data::IDataPublisher*> pubController;
        std::vector<uint8_t> testData = {1, 1, 1};
        pubController.reserve(numberOfTopics);
        bool allPublished = false;
        for (auto i = 0; i < numberOfTopics; i++)
        {
            const auto controllerName = "Pub-" + std::to_string(i);
            const auto topic = "TopicName-" + std::to_string(i);
            pubController.push_back(publisher->CreateDataPublisher(controllerName, topic, "", {}, 0));
        }
        auto&& participantController = publisher->GetParticipantController();
        participantController->SetSimulationTask(
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
            });
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
