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
        auto domainId = static_cast<uint32_t>(GetTestPid());

        ib::test::SimTestHarness testHarness(syncParticipantNames, domainId, true);
        auto&& publisher = testHarness.GetParticipant("Publisher")->ComAdapter();
        
        for (auto i = 0; i < numberOfServices; i++)
        {
            const auto topic = "TopicName-" + std::to_string(i);
            (void)publisher->CreateDataPublisher(topic, ib::sim::data::DataExchangeFormat{}, {},
                                                 0); 
        }
        auto logger = publisher->GetLogger();
        auto&& participantController = publisher->GetParticipantController();
        participantController->SetSimulationTask([&logger, &participantController](auto, auto) {
            logger->Info("::::::::::: Sending STOP");
            participantController->Stop("Test complete");
        });
    
        auto makeSubscriber = [&](auto subscriberName)
        {
            auto&& subscriber = testHarness.GetParticipant(subscriberName)->ComAdapter();

            for (auto i = 0; i < numberOfServices; i++)
            {
                const auto topic = "TopicName-" + std::to_string(i);
                (void)subscriber->CreateDataSubscriber(
                    topic, ib::sim::data::DataExchangeFormat{}, {},
                    [](ib::sim::data::IDataSubscriber* subscriber, const std::vector<uint8_t>& data) {
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
protected:
    std::vector<std::string> syncParticipantNames;
};


// Stress testing the discovery mechanism, it shouldn't slow down the IB performance

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
