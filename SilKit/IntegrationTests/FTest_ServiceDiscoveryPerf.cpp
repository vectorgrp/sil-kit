// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <iostream>

#include "silkit/services/all.hpp"
#include "silkit/services/logging/ILogger.hpp"
#include "silkit/services/pubsub/PubSubSpec.hpp"

#include "SimTestHarness.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using namespace std::chrono_literals;

auto Now()
{
    auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now);
}

class FTest_ServiceDiscoveryPerf : public testing::Test
{
protected:
    FTest_ServiceDiscoveryPerf() {}

    void ExecuteTest(int numberOfServices, std::chrono::seconds timeout)
    {
        auto start = Now();

        std::vector<std::string> syncParticipantNames = {"Publisher", "Subscriber", "Subscriber2"};

        SilKit::Tests::SimTestHarness testHarness(syncParticipantNames, "silkit://localhost:0", true);
        auto&& publisher = testHarness.GetParticipant("Publisher");

        for (auto i = 0; i < numberOfServices; i++)
        {
            const auto topic = "TopicName-" + std::to_string(i);
            const auto controllerName = "PubCtrl" + std::to_string(i);
            SilKit::Services::PubSub::PubSubSpec dataSpec{topic, {}};
            (void)publisher->Participant()->CreateDataPublisher(controllerName, dataSpec, 0);
        }

        auto logger = publisher->GetLogger();
        auto* lifecycleService = publisher->GetOrCreateLifecycleService();
        auto* timeSyncService = publisher->GetOrCreateTimeSyncService();

        timeSyncService->SetSimulationStepHandler([&logger, &lifecycleService](auto, auto) {
            logger->Info("::::::::::: Sending STOP");
            lifecycleService->Stop("Test complete");
        }, 1ms);

        auto makeSubscriber = [&](auto subscriberName) {
            auto&& subscriber = testHarness.GetParticipant(subscriberName)->Participant();

            for (auto i = 0; i < numberOfServices; i++)
            {
                const auto topic = "TopicName-" + std::to_string(i);
                const auto controllerName = "SubCtrl" + std::to_string(i);
                SilKit::Services::PubSub::PubSubSpec dataSpec{topic, {}};

                (void)subscriber->CreateDataSubscriber(
                    controllerName, dataSpec,
                    [](SilKit::Services::PubSub::IDataSubscriber* /*subscriber*/,
                       const SilKit::Services::PubSub::DataMessageEvent& /*data*/) {});
            }
        };
        //ensure the subscriber is created after the publisher, to check announcements, not just incremental notifications
        std::this_thread::sleep_for(100ms);
        makeSubscriber("Subscriber");
        std::this_thread::sleep_for(100ms);
        makeSubscriber("Subscriber2");

        std::chrono::duration<double> duration = Now() - start;
        std::cout << "Test with " << numberOfServices << " services startup time: " << duration.count() << "sec"
                  << std::endl;

        ASSERT_LT(duration, timeout) << "ServiceDiscovery should not have substantial impact on startup time"
                                     << ": duration=" << duration.count();

        auto ok = testHarness.Run(timeout); // short timeout is significant for this test
        ASSERT_TRUE(ok) << " Expected a short startup time, not blocked by service discovery: timeout="
                        << timeout.count();
    }
};


// Stress testing the discovery mechanism, it shouldn't slow down the SIL Kit performance

TEST_F(FTest_ServiceDiscoveryPerf, test_discovery_performance_10services)
{
    ExecuteTest(10, 1s);
}
TEST_F(FTest_ServiceDiscoveryPerf, test_discovery_performance_100services)
{
    ExecuteTest(100, 5s);
}

TEST_F(FTest_ServiceDiscoveryPerf, test_discovery_performance_200services)
{
    ExecuteTest(200, 25s);
}
} // anonymous namespace
