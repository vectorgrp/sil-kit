// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>

#include "ComAdapter.hpp"
#include "ComAdapter_impl.hpp"
#include "Registry.hpp"
#include "ib/cfg/ConfigBuilder.hpp"
#include "ib/sim/all.hpp"
#include "ib/util/functional.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "GetTestPid.hpp"

namespace {

using namespace std::chrono_literals;
using namespace ib::mw;
using namespace ib::mw::registry;

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
    {
        ib::cfg::ConfigBuilder cfgBuilder("CatchMeIfYouCanTestConfig");
        auto&& simulationSetup = cfgBuilder.SimulationSetup();
        simulationSetup.AddParticipant("Sender")
            ->AddGenericPublisher("CrashTopic");
        simulationSetup.AddParticipant("Receiver")
            ->AddGenericSubscriber("CrashTopic");

        ibConfig = cfgBuilder.Build();
    }

    void Subscribe()
    {
        subscriber->SetReceiveMessageHandler(
            [this](auto* /*subscriber*/, const std::vector<uint8_t>& /*data*/)
            {
                this->testOk.set_value(true);
                throw std::runtime_error{"CrashTest"};
            }
        );
    }

    void Publish()
    {
        std::vector<uint8_t> dummyPayload;
        publisher->Publish(std::move(dummyPayload));
    }

protected:
    ib::cfg::Config ibConfig;
    ib::sim::generic::IGenericPublisher* publisher{nullptr};
    ib::sim::generic::IGenericSubscriber* subscriber{nullptr};
    std::promise<bool> testOk;
};
    
TEST_F(CatchExceptionsInCallbacksITest, please_dont_crash)
{
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    std::unique_ptr<ComAdapter<FastRtpsConnection>> pubComAdapter = std::make_unique<ComAdapter<FastRtpsConnection>>(ibConfig, "Sender");
    pubComAdapter->joinIbDomain(domainId);

    std::unique_ptr<ComAdapter<FastRtpsConnection>> subComAdapter = std::make_unique<ComAdapter<FastRtpsConnection>>(ibConfig, "Receiver");
    subComAdapter->joinIbDomain(domainId);

    publisher = pubComAdapter->CreateGenericPublisher("CrashTopic");
    subscriber = subComAdapter->CreateGenericSubscriber("CrashTopic");

    Subscribe();

    std::thread publishThread{[this]{ this->Publish(); }};

    auto&& future = testOk.get_future();
    auto futureStatus = future.wait_for(5s);
    ASSERT_EQ(futureStatus, std::future_status::ready);
    EXPECT_TRUE(future.get());

    publishThread.join();
}

TEST_F(CatchExceptionsInCallbacksITest, please_dont_crash_vasio)
{
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    std::unique_ptr<Registry> registry = std::make_unique<Registry>(ibConfig);
    registry->ProvideDomain(domainId);

    std::unique_ptr<ComAdapter<VAsioConnection>> pubComAdapter = std::make_unique<ComAdapter<VAsioConnection>>(ibConfig, "Sender");
    pubComAdapter->joinIbDomain(domainId);

    std::unique_ptr<ComAdapter<VAsioConnection>> subComAdapter = std::make_unique<ComAdapter<VAsioConnection>>(ibConfig, "Receiver");
    subComAdapter->joinIbDomain(domainId);

    publisher = pubComAdapter->CreateGenericPublisher("CrashTopic");
    subscriber = subComAdapter->CreateGenericSubscriber("CrashTopic");

    Subscribe();
    std::this_thread::sleep_for(500ms);

    std::thread publishThread{[this] { this->Publish(); }};

    auto&& future = testOk.get_future();
    auto futureStatus = future.wait_for(5s);
    ASSERT_EQ(futureStatus, std::future_status::ready);
    EXPECT_TRUE(future.get());

    publishThread.join();
}

} // anonymous namespace
