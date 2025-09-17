// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ITestFixture.hpp"

#include "silkit/services/pubsub/all.hpp"
#include "silkit/services/rpc/all.hpp"

namespace {
using namespace SilKit::Tests;
using namespace SilKit::Config;
using namespace SilKit::Services;
using namespace SilKit::Services::PubSub;
using namespace SilKit::Services::Rpc;
using namespace std::chrono_literals;

struct ITest_LabelMatching : ITest_SimTestHarness
{
    using ITest_SimTestHarness::ITest_SimTestHarness;
};

TEST_F(ITest_LabelMatching, pubsub_multiple_controllers_same_topic_different_labels)
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
        }, 1ms);
    }


    _simTestHarness->Run(1s);

    EXPECT_EQ(numReceivedSubCtrl1, 1u);
    EXPECT_EQ(numReceivedSubCtrl2, 1u);
    EXPECT_EQ(numReceivedSubCtrl3, 1u);
}

TEST_F(ITest_LabelMatching, rpc_multiple_controllers_same_topic_different_labels)
{
    // This test ensures that we do not regress on SILKIT-1691

    SetupFromParticipantList({"Client1", "Server1"});

    auto functionName = "F";
    auto mediaType = "M";

    size_t numReceivedCallResults1{0};

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
        participant->CreateRpcServer("ServerCtrl1", spec1,
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

        timeSyncService->SetSimulationStepHandler([client1, lifecycleService](auto now, auto) {
            if (now == 0ms)
            {
                client1->Call(std::vector<uint8_t>{1});
            }
            else if (now == 10ms)
            {
                lifecycleService->Stop("Stopping test");
            }
        }, 1ms);
    }

    _simTestHarness->Run(1s);

    EXPECT_EQ(numReceivedCallResults1, 1u);

    EXPECT_EQ(numReceivedCalls1, 1u);
    EXPECT_EQ(numReceivedCalls2, 0u);
    EXPECT_EQ(numReceivedCalls3, 0u);
}

} //end namespace
