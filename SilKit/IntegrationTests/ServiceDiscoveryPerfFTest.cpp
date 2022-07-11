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

class ServiceDiscoveryPerfFTest : public testing::Test
{
protected:

    ServiceDiscoveryPerfFTest()
    {
    }

    void ExecuteTest(int numberOfServices, std::chrono::seconds timeout)
    {
        auto start = Now();

        std::vector<std::string> syncParticipantNames = {"Publisher", "Subscriber", "Subscriber2"};
        auto registryUri = MakeTestRegistryUri();

        SilKit::Tests::SimTestHarness testHarness(syncParticipantNames, registryUri, true);
        auto&& publisher = testHarness.GetParticipant("Publisher")->Participant();
        
        for (auto i = 0; i < numberOfServices; i++)
        {
            const auto topic = "TopicName-" + std::to_string(i);
            const auto controllerName = "PubCtrl" + std::to_string(i);
            (void)publisher->CreateDataPublisher(controllerName, topic, {}, {}, 0); 
        }

        auto logger = publisher->GetLogger();
        auto* lifecycleService = publisher->GetLifecycleService();
        auto* timeSyncService = lifecycleService->GetTimeSyncService();

        timeSyncService->SetSimulationTask([&logger, &lifecycleService](auto, auto) {
            logger->Info("::::::::::: Sending STOP");
            lifecycleService->Stop("Test complete");
        });
    
        auto makeSubscriber = [&](auto subscriberName)
        {
            auto&& subscriber = testHarness.GetParticipant(subscriberName)->Participant();

            for (auto i = 0; i < numberOfServices; i++)
            {
                const auto topic = "TopicName-" + std::to_string(i);
                const auto controllerName = "SubCtrl" + std::to_string(i);
                (void)subscriber->CreateDataSubscriber(
                    controllerName, topic, {}, {},
                    [](SilKit::Services::PubSub::IDataSubscriber* /*subscriber*/, const SilKit::Services::PubSub::DataMessageEvent& /*data*/) {
                    }); 
            }
        };
        //ensure the subscriber is created after the publisher, to check announcements, not just incremental notifications
        std::this_thread::sleep_for(100ms);
        makeSubscriber("Subscriber");
        std::this_thread::sleep_for(100ms);
        makeSubscriber("Subscriber2");

        std::chrono::duration<double> duration = Now() - start;
        std::cout << "Test with " << numberOfServices << " services startup time: " << duration.count() << "sec" << std::endl;

        ASSERT_LT(duration, timeout) << "ServiceDiscovery should not have substantial impact on startup time"
            << ": duration=" << duration.count();

        auto ok = testHarness.Run(timeout); // short timeout is significant for this test
        ASSERT_TRUE(ok)
            << " Expected a short startup time, not blocked by service discovery: timeout="
            << timeout.count();
    }
};


// Stress testing the discovery mechanism, it shouldn't slow down the SIL Kit performance

TEST_F(ServiceDiscoveryPerfFTest, test_discovery_performance_10services)
{
    ExecuteTest(10, 1s);
}
TEST_F(ServiceDiscoveryPerfFTest, test_discovery_performance_100services)
{
    ExecuteTest(100, 5s);
}

TEST_F(ServiceDiscoveryPerfFTest, test_discovery_performance_200services)
{
    ExecuteTest(200, 25s);
}
} // anonymous namespace
