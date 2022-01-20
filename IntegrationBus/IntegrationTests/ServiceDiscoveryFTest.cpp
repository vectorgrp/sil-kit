// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>

#include "ib/cfg/ConfigBuilder.hpp"
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

class ServiceDiscoveryITest : public testing::Test
{
protected:
    void BuildConfig(int numberOfServices)
    {
        domainId = static_cast<uint32_t>(GetTestPid());
        //generate significant amount of service descriptors that need to be transferred

        ib::cfg::ConfigBuilder builder{"TestConfig"};
        auto&& setup = builder.SimulationSetup();
        auto&& publisher = setup.AddParticipant("Publisher");
        auto&& subscriber = setup.AddParticipant("Subscriber");
        auto&& subscriber2 = setup.AddParticipant("Subscriber2");
        for(auto i = 0; i < numberOfServices; i++)
        {
            const auto topic = "TopicName-" + std::to_string(i);
            publisher->AddGenericPublisher(topic);
            subscriber->AddGenericSubscriber(topic);
            subscriber2->AddGenericSubscriber(topic);
        }
        publisher->ConfigureLogger()
            .AddSink(ib::cfg::Sink::Type::Stdout)
            .WithLogLevel(ib::mw::logging::Level::Info)
            ;
        ibConfig = builder.Build();
        ibConfig.middlewareConfig.activeMiddleware = ib::cfg::Middleware::VAsio;
    }

    void ExecuteTest(int numberOfServices, std::chrono::seconds timeout)
    {
        auto start = Now();

        ib::test::SimTestHarness testHarness(ibConfig, domainId, true);
        auto&& publisher = testHarness.GetParticipant("Publisher")->ComAdapter();
        
        for (auto i = 0; i < numberOfServices; i++)
        {
            const auto topic = "TopicName-" + std::to_string(i);
            (void)publisher->CreateGenericPublisher(topic);//ensure the service discovery is engaged
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
                (void)subscriber->CreateGenericSubscriber(topic);//ensure the service discovery is engaged
            }
        };
        //ensure the subscriber is created after the publisher, to check announcements, not just incremental notifications
        std::this_thread::sleep_for(100ms);
        makeSubscriber("Subscriber");
        std::this_thread::sleep_for(100ms);
        makeSubscriber("Subscriber2");

        std::chrono::duration<double> duration = Now() - start;
        std::cout << "Test with " << numberOfServices << " services startup time: " << duration.count() << "sec" << std::endl;

        ASSERT_LT(duration, timeout) << "ServiceDiscovery shout not have substantial impact on startup time"
            << ": duration=" << duration.count();

        auto ok = testHarness.Run(timeout); // short timeout is significant for this test
        ASSERT_TRUE(ok)
            << " Expected a short startup time, not blocked by service discovery: timeout="
            << timeout.count();
    }
protected:
    uint32_t domainId;
    ib::cfg::Config ibConfig;
};


// Stress testing the discovery mechanism, it shouldn't slow down the IB performance

TEST_F(ServiceDiscoveryITest, test_discovery_performance_100services)
{
    BuildConfig(100);
    ExecuteTest(100, 1s);
}
TEST_F(ServiceDiscoveryITest, test_discovery_performance_1000services)
{
    BuildConfig(1000);
    ExecuteTest(1000, 3s);
}

TEST_F(ServiceDiscoveryITest, test_discovery_performance_2000services)
{
    BuildConfig(2000);
    ExecuteTest(2000, 5s);
}
} // anonymous namespace
