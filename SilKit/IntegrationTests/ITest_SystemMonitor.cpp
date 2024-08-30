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
#include <thread>

#include "silkit/services/all.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"

#include "functional.hpp"

#include "SimTestHarness.hpp"
#include "ConfigurationTestUtils.hpp"
#include "SyncDatatypeUtils.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using namespace std::chrono_literals;
using namespace SilKit::Core;

class ITest_SystemMonitor : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD(void, ParticipantConnectedHandler,
                    (const SilKit::Services::Orchestration::ParticipantConnectionInformation&), (const));

        MOCK_METHOD(void, ParticipantDisconnectedHandler,
                    (const SilKit::Services::Orchestration::ParticipantConnectionInformation&), (const));
    };

    struct SequencePoints
    {
        MOCK_METHOD(void, A_BeforeCreateThirdParticipant, ());
        MOCK_METHOD(void, B_AfterCreateThirdParticipant, ());
        MOCK_METHOD(void, C_AfterThirdParticipantDestroyed, ());
    };

    ITest_SystemMonitor() {}

    Callbacks callbacks;
    Callbacks secondCallbacks;
    SequencePoints sequencePoints;
};

// Tests that the service discovery handler fires for created services
// All created should be removed as well if a participant leaves
TEST_F(ITest_SystemMonitor, discover_services)
{
    const SilKit::Services::Orchestration::ParticipantConnectionInformation& firstParticipantConnection{"First"};
    const SilKit::Services::Orchestration::ParticipantConnectionInformation& secondParticipantConnection{"Second"};
    const SilKit::Services::Orchestration::ParticipantConnectionInformation& thirdParticipantConnection{"Third"};

    // Registry
    auto registry = SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::MakeEmptyParticipantConfiguration());
    auto registryUri = registry->StartListening("silkit://localhost:0");

    // Create the first participant and register the connect and disconnect callbacks
    auto&& firstParticipant = SilKit::CreateParticipant(SilKit::Config::MakeEmptyParticipantConfiguration(),
                                                        firstParticipantConnection.participantName, registryUri);

    auto* firstSystemMonitor = firstParticipant->CreateSystemMonitor();
    firstSystemMonitor->SetParticipantConnectedHandler(
        [this](const SilKit::Services::Orchestration::ParticipantConnectionInformation& participantInformation) {
        callbacks.ParticipantConnectedHandler(participantInformation);
    });
    firstSystemMonitor->SetParticipantDisconnectedHandler(
        [this](const SilKit::Services::Orchestration::ParticipantConnectionInformation& participantInformation) {
        callbacks.ParticipantDisconnectedHandler(participantInformation);
    });

    SilKit::Services::Orchestration::ISystemMonitor* secondSystemMonitor = nullptr;

    testing::Sequence sequence, secondSequence;

    std::promise<void> thirdParticipantDisconnectedPromiseA, thirdParticipantDisconnectedPromiseB;
    auto thirdParticipantDisconnectedFutureA = thirdParticipantDisconnectedPromiseA.get_future();
    auto thirdParticipantDisconnectedFutureB = thirdParticipantDisconnectedPromiseB.get_future();

    std::promise<void> secondParticipantDisconnectedPromise;
    auto secondParticipantDisconnectedFuture = secondParticipantDisconnectedPromise.get_future();

    EXPECT_CALL(callbacks, ParticipantConnectedHandler(secondParticipantConnection)).Times(1);

    EXPECT_CALL(sequencePoints, A_BeforeCreateThirdParticipant()).Times(1).InSequence(sequence, secondSequence);
    {
        EXPECT_CALL(callbacks, ParticipantConnectedHandler(thirdParticipantConnection)).Times(1).InSequence(sequence);
        EXPECT_CALL(secondCallbacks, ParticipantConnectedHandler(thirdParticipantConnection))
            .Times(1)
            .InSequence(secondSequence);
    }
    EXPECT_CALL(sequencePoints, B_AfterCreateThirdParticipant()).Times(1).InSequence(sequence, secondSequence);
    {
        EXPECT_CALL(callbacks, ParticipantDisconnectedHandler(thirdParticipantConnection))
            .Times(1)
            .InSequence(sequence)
            .WillOnce([&] {
            ASSERT_FALSE(firstSystemMonitor->IsParticipantConnected(thirdParticipantConnection.participantName));
            thirdParticipantDisconnectedPromiseA.set_value();
        });

        EXPECT_CALL(secondCallbacks, ParticipantDisconnectedHandler(thirdParticipantConnection))
            .Times(1)
            .InSequence(secondSequence)
            .WillOnce([&] {
            ASSERT_NE(secondSystemMonitor, nullptr);
            ASSERT_FALSE(secondSystemMonitor->IsParticipantConnected(thirdParticipantConnection.participantName));
            thirdParticipantDisconnectedPromiseB.set_value();
        });
    }
    EXPECT_CALL(sequencePoints, C_AfterThirdParticipantDestroyed()).Times(1).InSequence(sequence, secondSequence);
    {
        EXPECT_CALL(callbacks, ParticipantDisconnectedHandler(secondParticipantConnection))
            .Times(1)
            .InSequence(sequence)
            .WillOnce([&] {
            ASSERT_FALSE(firstSystemMonitor->IsParticipantConnected(secondParticipantConnection.participantName));
            secondParticipantDisconnectedPromise.set_value();
        });
    }

    ASSERT_FALSE(firstSystemMonitor->IsParticipantConnected(secondParticipantConnection.participantName));
    {
        // Create the second participant which should trigger the callbacks of the first
        auto&& secondParticipant = SilKit::CreateParticipant(SilKit::Config::MakeEmptyParticipantConfiguration(),
                                                             secondParticipantConnection.participantName, registryUri);

        secondSystemMonitor = secondParticipant->CreateSystemMonitor();
        secondSystemMonitor->SetParticipantConnectedHandler(
            [this](const SilKit::Services::Orchestration::ParticipantConnectionInformation&
                       participantConnectionInformation) {
            secondCallbacks.ParticipantConnectedHandler(participantConnectionInformation);
        });
        secondSystemMonitor->SetParticipantDisconnectedHandler(
            [this](const SilKit::Services::Orchestration::ParticipantConnectionInformation&
                       participantConnectionInformation) {
            secondCallbacks.ParticipantDisconnectedHandler(participantConnectionInformation);
        });

        ASSERT_FALSE(firstSystemMonitor->IsParticipantConnected(thirdParticipantConnection.participantName));
        ASSERT_FALSE(secondSystemMonitor->IsParticipantConnected(thirdParticipantConnection.participantName));

        sequencePoints.A_BeforeCreateThirdParticipant();

        // Create the third participant which should trigger the callbacks of the first and second
        auto&& thirdParticipant = SilKit::CreateParticipant(SilKit::Config::MakeEmptyParticipantConfiguration(),
                                                            thirdParticipantConnection.participantName, registryUri);

        ASSERT_TRUE(firstSystemMonitor->IsParticipantConnected(thirdParticipantConnection.participantName));
        ASSERT_TRUE(secondSystemMonitor->IsParticipantConnected(thirdParticipantConnection.participantName));

        sequencePoints.B_AfterCreateThirdParticipant();

        // Destroy the third participant
        thirdParticipant.reset();

        ASSERT_EQ(thirdParticipantDisconnectedFutureA.wait_for(std::chrono::seconds{10}), std::future_status::ready);
        ASSERT_EQ(thirdParticipantDisconnectedFutureB.wait_for(std::chrono::seconds{10}), std::future_status::ready);

        sequencePoints.C_AfterThirdParticipantDestroyed();

        // Destroy the second participant
        ASSERT_TRUE(firstSystemMonitor->IsParticipantConnected(secondParticipantConnection.participantName));
        secondParticipant.reset();

        ASSERT_EQ(secondParticipantDisconnectedFuture.wait_for(std::chrono::seconds{10}), std::future_status::ready);
    }
}

} // anonymous namespace
