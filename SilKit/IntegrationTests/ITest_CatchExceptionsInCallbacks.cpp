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

#include <chrono>
#include <cstdlib>
#include <thread>
#include <future>

#include "silkit/SilKit.hpp"
#include "silkit/services/all.hpp"
#include "silkit/participant/exception.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using namespace std::chrono_literals;

using testing::_;
using testing::A;
using testing::An;
using testing::InSequence;
using testing::NiceMock;
using testing::Return;

class ITest_CatchExceptionsInCallbacks : public testing::Test
{
protected:
    ITest_CatchExceptionsInCallbacks() {}

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


TEST_F(ITest_CatchExceptionsInCallbacks, please_dont_crash_vasio)
{

    auto registry =
        SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::ParticipantConfigurationFromString(""));
    auto registryUri = registry->StartListening("silkit://localhost:0");

    auto pubParticipant =
        SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(""), "Sender", registryUri);

    auto subParticipant =
        SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(""), "Receiver", registryUri);

    SilKit::Services::PubSub::PubSubSpec dataSpec{"CrashTopic", {}};
    SilKit::Services::PubSub::PubSubSpec matchingDataSpec{"CrashTopic", {}};
    publisher = pubParticipant->CreateDataPublisher("PubCtrl1", dataSpec, 0);
    subscriber = subParticipant->CreateDataSubscriber(
        "SubCtrl1", matchingDataSpec,
        [this](auto* /*subscriber*/, const SilKit::Services::PubSub::DataMessageEvent& /*data*/) {
        this->testOk.set_value(true);
        throw SilKit::SilKitError{"CrashTest"};
    });

    std::this_thread::sleep_for(500ms);
    std::thread publishThread{[this] { this->Publish(); }};

    auto&& future = testOk.get_future();
    auto futureStatus = future.wait_for(5s);
    ASSERT_EQ(futureStatus, std::future_status::ready);
    EXPECT_TRUE(future.get());

    publishThread.join();
}

} // anonymous namespace
