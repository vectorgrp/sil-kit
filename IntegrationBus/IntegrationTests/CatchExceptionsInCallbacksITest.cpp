// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>

#include "CreateParticipant.hpp"

#include "silkit/services/all.hpp"
#include "silkit/util/functional.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "GetTestPid.hpp"
#include "ConfigurationTestUtils.hpp"

#include "VAsioRegistry.hpp"

namespace {

using namespace std::chrono_literals;
using namespace SilKit::Core;

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
    }

    void Publish()
    {
        std::vector<uint8_t> dummyPayload;
        publisher->Publish(std::move(dummyPayload));
    }

protected:
    SilKit::Services::PubSub::IDataPublisher* publisher{nullptr};
    SilKit::Services::PubSub::IDataSubscriber* subscriber{nullptr};
    std::promise<bool> testOk;
};


TEST_F(CatchExceptionsInCallbacksITest, please_dont_crash_vasio)
{
    auto registryUri = MakeTestRegistryUri();

    auto registry = std::make_unique<VAsioRegistry>(SilKit::Config::MakeEmptyParticipantConfiguration());
    registry->ProvideDomain(registryUri);

    std::string participantNameSender = "Sender";
    auto pubParticipant = SilKit::Core::CreateParticipantImpl(
        SilKit::Config::MakeEmptyParticipantConfiguration(), participantNameSender);
    pubParticipant->JoinSilKitDomain(registryUri);

    std::string participantNameReceiver = "Receiver";
    auto subParticipant = SilKit::Core::CreateParticipantImpl(
        SilKit::Config::MakeEmptyParticipantConfiguration(), participantNameReceiver);
    subParticipant->JoinSilKitDomain(registryUri);

    publisher = pubParticipant->CreateDataPublisher("PubCtrl1", "CrashTopic", {}, {}, 0);
    subscriber = subParticipant->CreateDataSubscriber(
        "SubCtrl1", "CrashTopic", {}, {},
        [this](auto* /*subscriber*/, const SilKit::Services::PubSub::DataMessageEvent& /*data*/) {
            this->testOk.set_value(true);
            throw std::runtime_error{"CrashTest"};
        },
        nullptr);

    std::this_thread::sleep_for(500ms);
    std::thread publishThread{[this] { this->Publish(); }};

    auto&& future = testOk.get_future();
    auto futureStatus = future.wait_for(5s);
    ASSERT_EQ(futureStatus, std::future_status::ready);
    EXPECT_TRUE(future.get());

    publishThread.join();
}

} // anonymous namespace
