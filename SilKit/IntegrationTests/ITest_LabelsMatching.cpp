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

#include <memory>
#include <thread>
#include <string>
#include <chrono>
#include <iostream>
#include <atomic>

#include <unordered_map> //remove this after rebase on cmake-cleanup-branch

#include "ITestFixture.hpp"
#include "ITestThreadSafeLogger.hpp"

#include "silkit/services/pubsub/all.hpp"
#include "silkit/services/rpc/all.hpp"

#include "gtest/gtest.h"

namespace {
using namespace SilKit::Tests;
using namespace SilKit::Config;
using namespace SilKit::Services;
using namespace SilKit::Services::PubSub;
using namespace SilKit::Services::Rpc;

struct ITest_LabelMatching: ITest_SimTestHarness
{
    using ITest_SimTestHarness::ITest_SimTestHarness;
};

TEST_F(ITest_LabelMatching,  pubsub_multiple_controllers_same_topic_different_labels)
{
    SetupFromParticipantList({"Pub1", "Sub1"});

    auto topic = "T";
    auto mediaType = "M";

    size_t numReceivedSubCtrl1{0};
    size_t numReceivedSubCtrl2{0};
    size_t numReceivedSubCtrl3{0};

    {
        /////////////////////////////////////////////////////////////////////////
        // Sub1
        /////////////////////////////////////////////////////////////////////////
        const auto participantName = "Sub1";
        auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
        auto&& participant = simParticipant->Participant();

        PubSubSpec spec1{topic, mediaType};
        spec1.AddLabel("K", "L1", MatchingLabel::Kind::Mandatory);
        participant->CreateDataSubscriber(
            "SubCtrl1", spec1,
            [&numReceivedSubCtrl1](IDataSubscriber* /*subscriber*/, const DataMessageEvent& /*dataMessageEvent*/) {
            numReceivedSubCtrl1++;
        });

        PubSubSpec spec2{topic, mediaType};
        spec2.AddLabel("K", "L2", MatchingLabel::Kind::Mandatory);
        participant->CreateDataSubscriber(
            "SubCtrl2", spec2,
            [&numReceivedSubCtrl2](IDataSubscriber* /*subscriber*/, const DataMessageEvent& /*dataMessageEvent*/) {
            numReceivedSubCtrl2++;
        });

        PubSubSpec spec3{topic, mediaType};
        spec3.AddLabel("K", "L3", MatchingLabel::Kind::Mandatory);
        participant->CreateDataSubscriber(
            "SubCtrl3", spec3,
            [&numReceivedSubCtrl3](IDataSubscriber* /*subscriber*/, const DataMessageEvent& /*dataMessageEvent*/) {
            numReceivedSubCtrl3++;
        });
    }

    {
        /////////////////////////////////////////////////////////////////////////
        // Pub1
        /////////////////////////////////////////////////////////////////////////
        const auto participantName = "Pub1";
        auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
        auto&& participant = simParticipant->Participant();
        auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
        auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();

        PubSubSpec spec1{topic, mediaType};
        spec1.AddLabel("K", "L1", MatchingLabel::Kind::Optional);
        auto&& dataPublisher1 = participant->CreateDataPublisher("PubCtrl1", spec1, 0);

        PubSubSpec spec2{topic, mediaType};
        spec2.AddLabel("K", "L2", MatchingLabel::Kind::Optional);
        auto&& dataPublisher2 = participant->CreateDataPublisher("PubCtrl2", spec2, 0);

        PubSubSpec spec3{topic, mediaType};
        spec3.AddLabel("K", "L3", MatchingLabel::Kind::Optional);
        auto&& dataPublisher3 = participant->CreateDataPublisher("PubCtrl3", spec3, 0);


        timeSyncService->SetSimulationStepHandler(
            [dataPublisher1, dataPublisher2, dataPublisher3, lifecycleService](auto now, auto) {
            if (now == 0ms)
            {
                dataPublisher1->Publish(std::vector<uint8_t>{1});
                dataPublisher2->Publish(std::vector<uint8_t>{2});
                dataPublisher3->Publish(std::vector<uint8_t>{3});
            }
            else if (now == 10ms)
            {
                lifecycleService->Stop("Stopping test");
            }
        },
            1ms);
    }


    _simTestHarness->Run(1s);

    EXPECT_EQ(numReceivedSubCtrl1, 1);
    EXPECT_EQ(numReceivedSubCtrl2, 1);
    EXPECT_EQ(numReceivedSubCtrl3, 1);
}

TEST_F(ITest_LabelMatching, rpc_multiple_controllers_same_topic_different_labels)
{
    SetupFromParticipantList({"Client1", "Server1"});

    auto functionName = "F";
    auto mediaType = "M";

    size_t numReceivedCallResults1{0};
    size_t numReceivedCallResults2{0};
    size_t numReceivedCallResults3{0};

    size_t numReceivedCalls1{0};
    size_t numReceivedCalls2{0};
    size_t numReceivedCalls3{0};

    {
        /////////////////////////////////////////////////////////////////////////
        // Server1
        /////////////////////////////////////////////////////////////////////////
        const auto participantName = "Server1";
        auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
        auto&& participant = simParticipant->Participant();

        RpcSpec spec1{functionName, mediaType};
        spec1.AddLabel("K", "L1", MatchingLabel::Kind::Mandatory);
        participant->CreateRpcServer(
            "ServerCtrl1", spec1,
            [&numReceivedCalls1](IRpcServer* server, RpcCallEvent event) {
            numReceivedCalls1++;
            server->SubmitResult(event.callHandle, std::vector<uint8_t>{1});
        });

        RpcSpec spec2{functionName, mediaType};
        spec2.AddLabel("K", "L2", MatchingLabel::Kind::Mandatory);
        participant->CreateRpcServer("ServerCtrl2", spec2,
                                     [&numReceivedCalls2](IRpcServer* server, RpcCallEvent event) {
            numReceivedCalls2++;
            server->SubmitResult(event.callHandle, std::vector<uint8_t>{2});
        });

        RpcSpec spec3{functionName, mediaType};
        spec3.AddLabel("K", "L3", MatchingLabel::Kind::Mandatory);
        participant->CreateRpcServer("ServerCtrl3", spec3,
                                     [&numReceivedCalls3](IRpcServer* server, RpcCallEvent event) {
            numReceivedCalls3++;
            server->SubmitResult(event.callHandle, std::vector<uint8_t>{3});
        });
    }
    
    
    {
        /////////////////////////////////////////////////////////////////////////
        // Client1
        /////////////////////////////////////////////////////////////////////////
        const auto participantName = "Client1";
        auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
        auto&& participant = simParticipant->Participant();
        auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
        auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();

        RpcSpec spec1{functionName, mediaType};
        spec1.AddLabel("K", "L1", MatchingLabel::Kind::Optional);
        auto&& client1 = participant->CreateRpcClient(
            "ClientCtrl1", spec1, [&numReceivedCallResults1](auto*, const auto&) { numReceivedCallResults1++; });

        RpcSpec spec2{functionName, mediaType};
        spec2.AddLabel("K", "L2", MatchingLabel::Kind::Optional);
        auto&& client2 = participant->CreateRpcClient(
            "ClientCtrl2", spec2, [&numReceivedCallResults2](auto*, const auto&) { numReceivedCallResults2++; });

        RpcSpec spec3{functionName, mediaType};
        spec3.AddLabel("K", "L3", MatchingLabel::Kind::Optional);
        auto&& client3 = participant->CreateRpcClient(
            "ClientCtrl3", spec3, [&numReceivedCallResults3](auto*, const auto&) { numReceivedCallResults3++; });

        timeSyncService->SetSimulationStepHandler(
            [client1, client2, client3, lifecycleService](auto now, auto) {
            if (now == 0ms)
            {
                client1->Call(std::vector<uint8_t>{1});
                client2->Call(std::vector<uint8_t>{2});
                client3->Call(std::vector<uint8_t>{3});
            }
            else if (now == 10ms)
            {
                lifecycleService->Stop("Stopping test");
            }
        }, 1ms);
    }

    _simTestHarness->Run(1s);

    EXPECT_EQ(numReceivedCallResults1, 1);
    EXPECT_EQ(numReceivedCallResults2, 1);
    EXPECT_EQ(numReceivedCallResults3, 1);

    EXPECT_EQ(numReceivedCalls1, 1);
    EXPECT_EQ(numReceivedCalls2, 1);
    EXPECT_EQ(numReceivedCalls3, 1);
}

} //end namespace
