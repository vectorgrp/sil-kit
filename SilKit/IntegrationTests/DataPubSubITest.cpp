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

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "DataPubSubITest.hpp"

namespace {

//--------------------------------------
// Sync tests: Publish in SimulationTask
//--------------------------------------

// One publisher participant, one subscriber participant
TEST_F(DataPubSubITest, test_1pub_1sub_delayed_default_handler)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = numMsgToPublish;

    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({"Pub1", {{"PubCtrl1", "TopicA", {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}}, {}});
    PubSubParticipant subscriber{
        "Sub1", {}, {{"SubCtrl1", "TopicA", {"A"}, {}, defaultMsgSize, numMsgToReceive, 1, {}, {}}}};
    subscriber.delayedDefaultDataHandler = true;
    pubsubs.push_back(std::move(subscriber));

    RunSyncTest(pubsubs);
}

// One publisher participant, one subscriber participant
TEST_F(DataPubSubITest, test_1pub_1sub_sync)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = numMsgToPublish;

    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({ "Pub1", {{ "PubCtrl1", "TopicA",  {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}}, {} });
    pubsubs.push_back({ "Sub1", {}, {{"SubCtrl1", "TopicA",  {"A"}, {}, defaultMsgSize, numMsgToReceive, 1, {}, {} }} });

    RunSyncTest(pubsubs);
}

// Two mixed pub/sub participants
TEST_F(DataPubSubITest, test_2_mixed_participants)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = numMsgToPublish;

    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({
        "PubSub1",
        {{"PubCtrl1", "TopicB", {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}}, // Pub info
        {{"SubCtrl1", "TopicA", {"A"}, {}, defaultMsgSize, numMsgToReceive, 1, {}, {}}}, // Sub info
    });

    pubsubs.push_back({
        "PubSub2",
        {{"PubCtrl1", "TopicA", {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}}, // Pub info
        {{"SubCtrl1", "TopicB", {"A"}, {}, defaultMsgSize, numMsgToReceive, 1, {}, {}}}, // Sub info
    });

    RunSyncTest(pubsubs);
}

// Two mixed pub/sub participants with configuration
TEST_F(DataPubSubITest, test_2_mixed_participants_configured)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = numMsgToPublish;

    const auto configStringPubSub1 = R"raw(
ParticipantName: PubSub1
DataPublishers:
- Name: PubCtrl1
  Topic: TopicB
- Name: PubCtrl2
DataSubscribers:
- Name: SubCtrl1
  Topic: TopicA
)raw";

    const auto configStringPubSub2 = R"raw(
ParticipantName: PubSub2
DataPublishers:
- Name: PubCtrl1
  Topic: TopicA
DataSubscribers:
- Name: SubCtrl1
  Topic: TopicB
- Name: SubCtrl2
)raw";

    auto configPubSub1 = SilKit::Config::ParticipantConfigurationFromString(configStringPubSub1);
    auto configPubSub2 = SilKit::Config::ParticipantConfigurationFromString(configStringPubSub2);

    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({
        "PubSub1",
        {
            {"PubCtrl1", "ShouldBeOverwritten", {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}, // Publishes for PubSub2->SubCtrl1
            {"PubCtrl2", "TopicC", {"A"}, {}, 0, defaultMsgSize, numMsgToPublish} // Has no topic configured.
        }, 
        {{"SubCtrl1", "ShouldBeOverwritten", {"A"}, {}, defaultMsgSize, numMsgToReceive, 1, {}, {}}},  // Receives by PubSub2->PubCtrl1
        configPubSub1
    });

    pubsubs.push_back({
        "PubSub2",
         {{"PubCtrl1", "ShouldBeOverwritten", {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}}, // Publishes for PubSub1->SubCtrl1
         {
             {"SubCtrl1", "ShouldBeOverwritten", {"A"}, {}, defaultMsgSize, numMsgToReceive, 1, {}, {}}, // Receives by PubSub1->PubCtrl1
             {"SubCtrl2", "TopicC", {"A"}, {}, defaultMsgSize, numMsgToReceive, 1, {}, {}}, // Has no topic configured.
         },
        configPubSub2
    });

    RunSyncTest(pubsubs);
}

// Large messages
TEST_F(DataPubSubITest, test_1pub_1sub_largemsg_sync)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = numMsgToPublish;
    const size_t   messageSize = 250000;

    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({ "Pub1", {{"PubCtrl1", "TopicA",  {"A"}, {}, 0, messageSize, numMsgToPublish}}, {} });
    pubsubs.push_back({ "Sub1", {},  {{"SubCtrl1", "TopicA",  {"A"}, {}, messageSize, numMsgToReceive, 1, {}, {}}} });

    RunSyncTest(pubsubs);
}

// Two publishers/subscribers with same topic on one participant
TEST_F(DataPubSubITest, test_1pub_1sub_sametopic_sync)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = numMsgToPublish * 2;
    // PubCtrl1 and PubCtrl2 have the same Topic, mediatype and labels,
    // so only one call of the newSourceDiscoveryHandler is expected
    const uint32_t numNewSouceDiscoveries = 1; 
    
    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({"Pub1",
                       {{"PubCtrl1", "TopicA", {"A"}, {}, 0, defaultMsgSize, numMsgToPublish},
                        {"PubCtrl2", "TopicA", {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}},
                       {}});

    std::vector<std::vector<uint8_t>> expectedDataUnordered;
    for (uint8_t d = 0; d < numMsgToPublish; d++)
    {
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
    }
    pubsubs.push_back({"Sub1",
                       {},
                       {{"SubCtrl1", "TopicA", {"A"}, {}, defaultMsgSize, numMsgToReceive, numNewSouceDiscoveries, expectedDataUnordered, {}, {}},
                        {"SubCtrl2", "TopicA", {"A"}, {}, defaultMsgSize, numMsgToReceive, numNewSouceDiscoveries, expectedDataUnordered, {}, {}}}});

    RunSyncTest(pubsubs);
}


// 100 topics on one publisher/subscriber participant
TEST_F(DataPubSubITest, test_1pub_1sub_100topics_sync)
{
    const uint32_t numMsgToPublish = 1;
    const uint32_t numMsgToReceive = numMsgToPublish;
    const int      numTopics = 100;

    std::vector<PubSubParticipant> pubsubs;
    
    pubsubs.push_back({ "Pub1" });
    pubsubs.push_back({ "Sub1" });
    for (int i = 0; i < numTopics; i++)
    {
        std::string topic = std::to_string(i);
        std::string pubControllerName = "PubCtrl" + std::to_string(i);
        std::string subControllerName = "SubCtrl" + std::to_string(i);
        DataPublisherInfo  pInfo{ pubControllerName, topic,  {"A"}, {}, 1, defaultMsgSize, numMsgToPublish };
        DataSubscriberInfo sInfo{ subControllerName, topic,  {"A"}, {},    defaultMsgSize, numMsgToReceive, 1, {}, {} };
        pubsubs[0].dataPublishers.push_back(std::move(pInfo));
        pubsubs[1].dataSubscribers.push_back(std::move(sInfo));
    }

    RunSyncTest(pubsubs);
}


// One publisher participant, two subscribers participants on same topic
TEST_F(DataPubSubITest, test_1pub_2sub_sync)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = numMsgToPublish;

    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({"Pub1", {{"PubCtrl1", "TopicA",  {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}},{}});
    pubsubs.push_back({"Sub1", {}, {{"SubCtrl1", "TopicA",  {"A"}, {}, defaultMsgSize, numMsgToReceive, 1, {}, {}}}});
    pubsubs.push_back({"Sub2", {}, {{"SubCtrl1", "TopicA",  {"A"}, {}, defaultMsgSize, numMsgToReceive, 1, {}, {}}}});

    RunSyncTest(pubsubs);
}

// Two publisher participants, one subscriber participant on same topic: Expect all to arrive but arbitrary reception order
TEST_F(DataPubSubITest, test_2pub_1sub_sync)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = numMsgToPublish * 2;
    // PubCtrl1 and PubCtrl2 have the same Topic, mediatype and labels,
    // so only one call of the newSourceDiscoveryHandler is expected
    const uint32_t numNewSouceDiscoveries = 1; 

    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({"Pub1", {{"PubCtrl1", "TopicA",  {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}}, {}});
    pubsubs.push_back({"Pub2", {{"PubCtrl2", "TopicA",  {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}}, {}});
    std::vector<std::vector<uint8_t>> expectedDataUnordered;
    for (uint8_t d = 0; d < numMsgToPublish; d++)
    {
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
    }
    pubsubs.push_back({"Sub1",
                       {},
                       {{"SubCtrl1",
                         "TopicA",
                         {"A"},
                         {},
                         defaultMsgSize,
                         numMsgToReceive,
                         numNewSouceDiscoveries,
                         expectedDataUnordered,
                         {},
                         {}}}});

    RunSyncTest(pubsubs);
}

// Seven participants, multiple topics
TEST_F(DataPubSubITest, test_3pub_4sub_4topics_sync)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = numMsgToPublish;
    // PubCtrl1 on Pub1,2,3 have the same Topic, mediatype and labels,
    // so only one call of the newSourceDiscoveryHandler is expected
    const uint32_t numNewSouceDiscoveries = 1; 

    std::vector<PubSubParticipant> pubsubs;
    
    pubsubs.push_back({ "Pub1",  {{"PubCtrl1", "TopicA",  {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}, {"PubCtrl2", "TopicB", "B", {}, 0, defaultMsgSize, numMsgToPublish}}, {} });
    pubsubs.push_back({ "Pub2",  {{"PubCtrl1", "TopicA",  {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}, {"PubCtrl2", "TopicC", "C", {}, 0, defaultMsgSize, numMsgToPublish}}, {} });
    pubsubs.push_back({ "Pub3",  {{"PubCtrl1", "TopicA",  {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}, {"PubCtrl2", "TopicD", "D", {}, 0, defaultMsgSize, numMsgToPublish}}, {}});

    std::vector<std::vector<uint8_t>> expectedDataUnordered;
    for (uint8_t d = 0; d < numMsgToPublish; d++)
    {
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
    }
    pubsubs.push_back({"Sub1",
                       {},
                       {{"SubCtrl1",
                         "TopicA",
                         {"A"},
                         {},
                         defaultMsgSize,
                         numMsgToPublish * 3,
                         numNewSouceDiscoveries,
                         expectedDataUnordered,
                         {},
                         {}}}});
    pubsubs.push_back({"Sub2", {}, {{"SubCtrl1", "TopicB", {"B"}, {}, defaultMsgSize, numMsgToReceive, 1, {}, {}}}});
    pubsubs.push_back({"Sub3", {}, {{"SubCtrl1", "TopicC", {"C"}, {}, defaultMsgSize, numMsgToReceive, 1, {}, {}}}});
    pubsubs.push_back({"Sub4", {}, {{"SubCtrl1", "TopicD", {"D"}, {}, defaultMsgSize, numMsgToReceive, 1, {}, {}}}});

    RunSyncTest(pubsubs);
}

// Wrong topic -> Expect no reception
TEST_F(DataPubSubITest, test_1pub_1sub_wrong_topic_sync)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = 0;

    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({"Pub1", {{"PubCtrl1", "TopicA", {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}}, {}});
    pubsubs.push_back({"Sub1", {}, {{"SubCtrl1", "TopicB", {"A"}, {}, defaultMsgSize, numMsgToReceive, 0, {}, {}}}});

    RunSyncTest(pubsubs);
}

// Matching labels
TEST_F(DataPubSubITest, test_1pub_1sub_label_sync)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = numMsgToPublish;

    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({"Pub1", {{"PubCtrl1", "TopicA", {"A"}, {{"kA", "vA"}, {"kB", "vB"}}, 0, defaultMsgSize, numMsgToPublish}},{}});
    pubsubs.push_back({"Sub1", {}, {{"SubCtrl1", "TopicA", {"A"}, {{"kA", "vA"}, {"kB", ""}}, defaultMsgSize, numMsgToReceive, 1, {{"kA", "vA"}, {"kB", "vB"}}, {}}}});

    RunSyncTest(pubsubs);
}

// Wrong label value -> Expect no reception
TEST_F(DataPubSubITest, test_1pub_1sub_wrong_labels_sync)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = 0;

    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({"Pub1", {{"PubCtrl1", "TopicA", {"A"}, {{"k", "v"}}, 0, defaultMsgSize, numMsgToPublish}},{}});
    pubsubs.push_back({ "Sub1", {},  {{"SubCtrl1", "TopicA", {"B"}, {{"k", "wrong"}}, defaultMsgSize, numMsgToReceive, 0, {{"k", "v"}}, {}}} });

    RunSyncTest(pubsubs);
}

// Wrong mediatype -> Expect no reception
TEST_F(DataPubSubITest, test_1pub_1sub_wrong_mediatype_sync)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = 0;

    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({"Pub1", {{"PubCtrl1", "TopicA",  {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}},{}});
    pubsubs.push_back({"Sub1", {}, {{"SubCtrl1", "TopicA", {"B"}, {}, defaultMsgSize, numMsgToReceive, 0, {}, {}}}});

    RunSyncTest(pubsubs);
}

// Wildcard mediatype on subscriber
TEST_F(DataPubSubITest, test_1pub_1sub_wildcard_mediatype_sync)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = numMsgToPublish;

    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({"Pub1", {{"PubCtrl1", "TopicA",  {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}}, {}});
    pubsubs.push_back({ "Sub1", {}, {{"SubCtrl1", "TopicA", {""}, {}, defaultMsgSize, numMsgToReceive, 1, {}, {}}} });

    RunSyncTest(pubsubs);
}

//-----------------------
// Specific handler tests
//-----------------------

// Check for incoming labels
TEST_F(DataPubSubITest, test_2pub_1sub_expectlabels_sync)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = numMsgToPublish * 2;

    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({"Pub1", {{"PubCtrl1", "TopicA", {"A"}, {{"kA", "vA"}}, 0, defaultMsgSize, numMsgToPublish}},{}});
    pubsubs.push_back({"Pub2", {{"PubCtrl1", "TopicA", {"A"}, {{"kB", "vB"}}, 0, defaultMsgSize, numMsgToPublish}},{}});
    std::vector<std::vector<uint8_t>> expectedDataUnordered;
    for (uint8_t d = 0; d < numMsgToPublish; d++)
    {
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
    }
    pubsubs.push_back({"Sub1", {}, {{"SubCtrl1", "TopicA", {"A"}, {}, defaultMsgSize, numMsgToReceive, 1, expectedDataUnordered, {{"kA", "vA"}, {"kB", "vB"}}, {}}}});

    RunSyncTest(pubsubs);
}

// Use specific handlers and no default handler
TEST_F(DataPubSubITest, test_3pub_1sub_specificHandlers_sync)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = numMsgToPublish * 3;

    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({ "Pub1", {{"PubCtrl1", "TopicA", {"A"}, {{"kA", "vA"}}, 0, defaultMsgSize, numMsgToPublish}}, {}});
    pubsubs.push_back({ "Pub2", {{"PubCtrl1", "TopicA", {"A"}, {{"kA", "vA"},{"kB", "vB"}}, 0, defaultMsgSize, numMsgToPublish}}, {}});
    // This publisher has the wrong labels and won't be received 
    pubsubs.push_back({ "Pub3", {{"PubCtrl1", "TopicA", {"A"}, {{"kC", "vC"}}, 0, defaultMsgSize, numMsgToPublish}}, {}});
    std::vector<std::vector<uint8_t>> expectedDataUnordered;
    for (uint8_t d = 0; d < numMsgToPublish; d++)
    {
        // First explicitDataMessageHandler receives by both publishers, so numMsgToPublish * 3 in total
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
    }
    std::vector<SpecificDataHandlerInfo> specificDataHandlers;
    specificDataHandlers.push_back({ {"A"}, {{"kA", "vA"}} });
    specificDataHandlers.push_back({ {"A"}, {{"kA", "vA"},{"kB", "vB"}} });
    pubsubs.push_back({ "Sub1", {}, 
        {{"SubCtrl1", "TopicA", {"A"}, {{"kA", ""}}, defaultMsgSize, numMsgToReceive, 1, expectedDataUnordered, {{"kA", "vA"}, {"kB", "vB"}, {"kC", "kC"}}, specificDataHandlers}} });

    RunSyncTest(pubsubs);
}

//--------------------
// Self-delivery tests
//--------------------

// 1 pub, 1 sub on a single participant
TEST_F(DataPubSubITest, test_1_participant_selfdelivery)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = numMsgToPublish;

    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({"PubSub1",
                       {{"PubCtrl1", "TopicA", {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}}, // Pub
                       {{"SubCtrl1", "TopicA", {"A"}, {}, defaultMsgSize, numMsgToReceive, 1, {}, {}}}}); // Sub

    RunSyncTest(pubsubs);
}

// 2 pub, 2 sub on a single participant with same topic
TEST_F(DataPubSubITest, test_1_participant_selfdelivery_same_topic)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = numMsgToPublish;
    // PubCtrl1 and PubCtrl2 have the same Topic, mediatype and labels,
    // so only one call of the newSourceDiscoveryHandler is expected
    const uint32_t numNewSouceDiscoveries = 1; 

    std::vector<PubSubParticipant> pubsubs;
    std::vector<std::vector<uint8_t>> expectedDataUnordered;
    for (uint8_t d = 0; d < numMsgToPublish; d++)
    {
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
    }
    pubsubs.push_back({"PubSub1",
                       {{"PubCtrl1", "TopicA", {"A"}, {}, 0, defaultMsgSize, numMsgToPublish},
                        {"PubCtrl2", "TopicA", {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}},
                       {{"SubCtrl1",
                         "TopicA",
                         {"A"},
                         {},
                         defaultMsgSize,
                         numMsgToReceive,
                         numNewSouceDiscoveries,
                         expectedDataUnordered,
                         {},
                         {}},
                        {"SubCtrl2",
                         "TopicA",
                         {"A"},
                         {},
                         defaultMsgSize,
                         numMsgToReceive,
                         numNewSouceDiscoveries,
                         expectedDataUnordered,
                         {},
                         {}}}});

    RunSyncTest(pubsubs);
}

//-----------------------------------------------------
// Async tests: No TimeSyncService/SimulationTask
//-----------------------------------------------------

// Async with history: Wait for publication before starting the subscriber
TEST_F(DataPubSubITest, test_1pub_1sub_async_history)
{
    const uint32_t numMsgToPublish = 1;
    const uint32_t numMsgToReceive = numMsgToPublish;

    std::vector<PubSubParticipant> publishers;
    publishers.push_back({"Pub1", {{"PubCtrl1", "TopicA", {"A"}, {}, 1, defaultMsgSize, numMsgToPublish}}, {}});
    std::vector<PubSubParticipant> subscribers;
    subscribers.push_back({"Sub1", {}, {{"SubCtrl1", "TopicA", {"A"}, {}, defaultMsgSize, numMsgToReceive, 1, {}, {}}}});

    RunAsyncTest(publishers, subscribers);
}

// Async with history and specific data handler
TEST_F(DataPubSubITest, test_1pub_1sub_async_history_specifichandler)
{
    const uint32_t numMsgToPublish = 1;
    const uint32_t numMsgToReceive = numMsgToPublish;

    std::vector<PubSubParticipant> publishers;
    publishers.push_back({"Pub1", {{"PubCtrl1", "TopicA", {"A"}, {}, 1, defaultMsgSize, numMsgToPublish}}, {}});

    std::vector<PubSubParticipant> subscribers;
    std::vector<SpecificDataHandlerInfo> specificDataHandlers;
    specificDataHandlers.push_back({{"A"}, {}});
    subscribers.push_back({"Sub1", {}, {{"SubCtrl1", "TopicA", {"A"}, {}, defaultMsgSize, numMsgToReceive, 1, {}, specificDataHandlers}}});

    RunAsyncTest(publishers, subscribers);
}

// Async rejoin
TEST_F(DataPubSubITest, test_1pub_1sub_async_rejoin)
{
    const uint32_t numMsgToPublish = 1;
    const uint32_t numSpecificDataHandlers = 10;
    const uint32_t numRejoins = 10;
    const uint32_t numMsgToReceive = numSpecificDataHandlers * numMsgToPublish;
    
    std::vector<PubSubParticipant> publishers;
    publishers.push_back({"Pub1", {{"PubCtrl1", "TopicA", {"A"}, {}, 1, defaultMsgSize, numMsgToPublish}}, {}});

    std::vector<PubSubParticipant> subscribers;
    std::vector<SpecificDataHandlerInfo> specificDataHandlers;
    for (uint32_t i = 0; i < numSpecificDataHandlers; i++)
    {
        specificDataHandlers.push_back({{"A"}, {}});
    }
    std::vector<std::vector<uint8_t>> expectedDataUnordered;
    expectedDataUnordered.reserve(numMsgToReceive);
    for (uint32_t d = 0; d < numMsgToReceive; d++)
    {
        // Receive the same blob several times (once from every publisher)
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, 0));
    }
    subscribers.push_back({"Sub1",
                           {},
                           {{"SubCtrl1",
                             "TopicA",
                             {"A"},
                             {},
                             defaultMsgSize,
                             numMsgToReceive,
                             1,
                             expectedDataUnordered,
                             {},
                             specificDataHandlers}}});
    
    auto registryUri = MakeTestRegistryUri();

    _testSystem.SetupRegistryAndSystemMaster(registryUri, false, {});
    RunParticipants(subscribers, registryUri, false);
    for (auto& s : subscribers)
    {
        s.WaitForCreateParticipant();
    }

    for (uint32_t i = 0; i < numRejoins; i++)
    {
        // Start publishers
        RunParticipants(publishers, registryUri, false);
        for (auto& p : publishers)
        {
            p.WaitForAllSent();
        }
        // Publishers are done after AllSent, join the thread to destruct and disconnect publishers
        _pubSubThreads.at(_pubSubThreads.size() - 1).join();
        if (i < numRejoins - 1)
        {
            // Recreate publisher
            publishers.clear();
            publishers.push_back({"Pub1", {{"PubCtrl1", "TopicA", {"A"}, {}, 1, defaultMsgSize, numMsgToPublish}}, {}});
        }
    }

    // Subscriber internally waits for receptions
    JoinPubSubThreads();
    ShutdownSystem();
}

} // anonymous namespace
