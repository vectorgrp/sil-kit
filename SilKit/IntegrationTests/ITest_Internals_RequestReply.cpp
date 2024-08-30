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
#include <functional>

#include "silkit/SilKit.hpp"
#include "silkit/services/all.hpp"
#include "silkit/services/logging/ILogger.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"
#include "silkit/experimental/services/orchestration/ISystemController.hpp"

#include "IRequestReplyService.hpp"
#include "procs/IParticipantReplies.hpp"

#include "ConfigurationTestUtils.hpp"
#include "VAsioRegistry.hpp"
#include "CreateParticipantImpl.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using namespace std::chrono_literals;
using namespace SilKit::Core;

class ITest_Internals_RequestReply : public testing::Test
{
protected:
    ITest_Internals_RequestReply() {}
};

TEST_F(ITest_Internals_RequestReply, participant_replies)
{
    // Registry
    auto registry = SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::MakeEmptyParticipantConfiguration());
    auto registryUri = registry->StartListening("silkit://localhost:0");

    // Create participants
    auto&& p1 =
        SilKit::CreateParticipantImpl(SilKit::Config::MakeEmptyParticipantConfigurationImpl(), "P1", registryUri);

    auto&& p2 =
        SilKit::CreateParticipantImpl(SilKit::Config::MakeEmptyParticipantConfigurationImpl(), "P2", registryUri);

    auto&& p3 =
        SilKit::CreateParticipantImpl(SilKit::Config::MakeEmptyParticipantConfigurationImpl(), "P3", registryUri);

    auto participantReplies_p1 = dynamic_cast<IParticipantInternal*>(p1.get())->GetParticipantRepliesProcedure();
    auto participantReplies_p2 = dynamic_cast<IParticipantInternal*>(p2.get())->GetParticipantRepliesProcedure();
    auto participantReplies_p3 = dynamic_cast<IParticipantInternal*>(p3.get())->GetParticipantRepliesProcedure();

    std::promise<void> receivedReplies_p1{};
    participantReplies_p1->CallAfterAllParticipantsReplied([&receivedReplies_p1]() { receivedReplies_p1.set_value(); });
    std::promise<void> receivedReplies_p2{};
    participantReplies_p2->CallAfterAllParticipantsReplied([&receivedReplies_p2]() { receivedReplies_p2.set_value(); });
    std::promise<void> receivedReplies_p3{};
    participantReplies_p3->CallAfterAllParticipantsReplied([&receivedReplies_p3]() { receivedReplies_p3.set_value(); });

    receivedReplies_p1.get_future().wait();
    receivedReplies_p2.get_future().wait();
    receivedReplies_p3.get_future().wait();
}

} // anonymous namespace
