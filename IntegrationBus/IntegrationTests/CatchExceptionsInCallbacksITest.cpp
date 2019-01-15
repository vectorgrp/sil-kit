// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>

#include "ib/IntegrationBus.hpp"
#include "ib/sim/all.hpp"
#include "ib/util/functional.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "GetTestPid.hpp"

namespace {

using namespace std::chrono_literals;

using testing::_;
using testing::A;
using testing::An;
using testing::InSequence;
using testing::NiceMock;
using testing::Return;

class CatchExceptionsInCallbacksITest : public testing::Test
{
protected:
    CatchExceptionsInCallbacksITest()
        : domainId{static_cast<uint32_t>(GetTestPid())}
    {
        ib::cfg::ConfigBuilder cfgBuilder("CatchMeIfYouCanTestConfig");
        auto&& simulationSetup = cfgBuilder.SimulationSetup();
        simulationSetup.AddParticipant("Sender")
            ->AddGenericPublisher("CrashTopic");
        simulationSetup.AddParticipant("Receiver")
            ->AddGenericSubscriber("CrashTopic");

        ibConfig = cfgBuilder.Build();
        
        pubComAdapter = ib::CreateFastRtpsComAdapter(ibConfig, "Sender", domainId);
        subComAdapter = ib::CreateFastRtpsComAdapter(ibConfig, "Receiver", domainId);
    }


    void Subscribe()
    {
        auto subscriber = subComAdapter->CreateGenericSubscriber("CrashTopic");
        subscriber->SetReceiveMessageHandler(
            [this](auto* subscriber, const std::vector<uint8_t>& /*data*/)
            {
                this->testOk.set_value(true);
                throw std::runtime_error{"CrashTest"};
            }
        );
    }

    void Publish()
    {
        auto publisher = pubComAdapter->CreateGenericPublisher("CrashTopic");
        std::vector<uint8_t> dummyPayload;
        publisher->Publish(std::move(dummyPayload));
    }

protected:
    const uint32_t domainId;
    ib::cfg::Config ibConfig;

    std::promise<bool> testOk;

    std::unique_ptr<ib::mw::IComAdapter> pubComAdapter;
    std::unique_ptr<ib::mw::IComAdapter> subComAdapter;
};
    
TEST_F(CatchExceptionsInCallbacksITest, please_dont_crash)
{
    Subscribe();

    std::thread publishThread{[this]{ this->Publish(); }};

    auto&& future = testOk.get_future();
    auto futureStatus = future.wait_for(5s);
    ASSERT_EQ(futureStatus, std::future_status::ready);
    EXPECT_TRUE(future.get());

    publishThread.join();
}

} // anonymous namespace
