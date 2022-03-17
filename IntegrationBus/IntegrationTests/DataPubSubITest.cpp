// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "DataPubSubITest.hpp"

namespace {

#if defined(IB_MW_HAVE_VASIO)

//--------------------------------------
// Sync tests: Publish in SimulationTask
//--------------------------------------

// One publisher participant, one subscriber participant
TEST_F(DataPubSubITest, test_1pub_1sub_sync_vasio)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = numMsgToPublish;

    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({ "Pub1", {{"TopicA",  {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}}, {} });
    pubsubs.push_back({ "Sub1", {},  { {"TopicA",  {"A"}, {}, defaultMsgSize, numMsgToReceive, 1, {}, {} }} });

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
        {{"TopicB", {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}}, // Pub info
        {{"TopicA", {"A"}, {}, defaultMsgSize, numMsgToReceive, 1, {}, {}}}, // Sub info
    });

    pubsubs.push_back({
        "PubSub2",
        {{"TopicA", {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}}, // Pub info
        {{"TopicB", {"A"}, {}, defaultMsgSize, numMsgToReceive, 1, {}, {}}}, // Sub info
    });

    RunSyncTest(pubsubs);
}

// Large messages
TEST_F(DataPubSubITest, test_1pub_1sub_largemsg_sync_vasio)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = numMsgToPublish;
    const size_t   messageSize = 250000;

    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({ "Pub1", {{"TopicA",  {"A"}, {}, 0, messageSize, numMsgToPublish}}, {} });
    pubsubs.push_back({ "Sub1", {},  { {"TopicA",  {"A"}, {}, messageSize, numMsgToReceive, 1, {}, {}}} });

    RunSyncTest(pubsubs);
}

// Two publishers/subscribers with same topic on one participant
TEST_F(DataPubSubITest, test_1pub_1sub_sametopic_sync_vasio)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = numMsgToPublish * 2;

    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({"Pub1",
                       {{"TopicA", {"A"}, {}, 0, defaultMsgSize, numMsgToPublish},
                        {"TopicA", {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}},
                       {}});

    std::vector<std::vector<uint8_t>> expectedDataUnordered;
    for (uint8_t d = 0; d < numMsgToPublish; d++)
    {
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
    }
    pubsubs.push_back({"Sub1",
                       {},
                       {{"TopicA", {"A"}, {}, defaultMsgSize, numMsgToReceive, 2, expectedDataUnordered, {}, {}},
                        {"TopicA", {"A"}, {}, defaultMsgSize, numMsgToReceive, 2, expectedDataUnordered, {}, {}}}});

    RunSyncTest(pubsubs);
}


// 100 topics on one publisher/subscriber participant
TEST_F(DataPubSubITest, test_1pub_1sub_100topics_sync_vasio)
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
        DataPublisherInfo  pInfo{ topic,  {"A"}, {}, 1, defaultMsgSize, numMsgToPublish };
        DataSubscriberInfo sInfo{ topic,  {"A"}, {},    defaultMsgSize, numMsgToReceive, 1, {}, {} };
        pubsubs[0].dataPublishers.push_back(std::move(pInfo));
        pubsubs[1].dataSubscribers.push_back(std::move(sInfo));
    }

    RunSyncTest(pubsubs);
}


// One publisher participant, two subscribers participants on same topic
TEST_F(DataPubSubITest, test_1pub_2sub_sync_vasio)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = numMsgToPublish;

    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({"Pub1", {{"TopicA",  {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}},{}});
    pubsubs.push_back({"Sub1", {}, {{"TopicA",  {"A"}, {}, defaultMsgSize, numMsgToReceive, 1, {}, {}}}});
    pubsubs.push_back({"Sub2", {}, {{"TopicA",  {"A"}, {}, defaultMsgSize, numMsgToReceive, 1, {}, {}}}});

    RunSyncTest(pubsubs);
}

// Two publisher participants, one subscriber participant on same topic: Expect all to arrive but arbitrary reception order
TEST_F(DataPubSubITest, test_2pub_1sub_sync_vasio)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = numMsgToPublish * 2;

    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({"Pub1", {{"TopicA",  {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}}, {}});
    pubsubs.push_back({"Pub2", {{"TopicA",  {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}}, {}});
    std::vector<std::vector<uint8_t>> expectedDataUnordered;
    for (uint8_t d = 0; d < numMsgToPublish; d++)
    {
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
    }
    pubsubs.push_back({ "Sub1", {}, {{"TopicA",  {"A"}, {}, defaultMsgSize, numMsgToReceive, 2, expectedDataUnordered, {}, {}}} });

    RunSyncTest(pubsubs);
}

// Seven participants, multiple topics
TEST_F(DataPubSubITest, test_3pub_4sub_4topics_sync_vasio)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = numMsgToPublish;

    std::vector<PubSubParticipant> pubsubs;
    
    pubsubs.push_back({ "Pub1",  {{"TopicA",  {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}, {"TopicB", "B", {}, 0, defaultMsgSize, numMsgToPublish}}, {}});
    pubsubs.push_back({ "Pub2",  {{"TopicA",  {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}, {"TopicC", "C", {}, 0, defaultMsgSize, numMsgToPublish}}, {}});
    pubsubs.push_back({ "Pub3",  {{"TopicA",  {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}, {"TopicD", "D", {}, 0, defaultMsgSize, numMsgToPublish}}, {}});

    std::vector<std::vector<uint8_t>> expectedDataUnordered;
    for (uint8_t d = 0; d < numMsgToPublish; d++)
    {
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
    }
    pubsubs.push_back({"Sub1", {}, {{"TopicA", {"A"}, {}, defaultMsgSize, numMsgToPublish * 3, 3,  expectedDataUnordered, {}, {}}} });
    pubsubs.push_back({"Sub2", {}, {{"TopicB", {"B"}, {}, defaultMsgSize, numMsgToReceive, 1, {}, {}}}});
    pubsubs.push_back({"Sub3", {}, {{"TopicC", {"C"}, {}, defaultMsgSize, numMsgToReceive, 1, {}, {}}}});
    pubsubs.push_back({"Sub4", {}, {{"TopicD", {"D"}, {}, defaultMsgSize, numMsgToReceive, 1, {}, {}}}});

    RunSyncTest(pubsubs);
}

// Wrong topic -> Expect no reception
TEST_F(DataPubSubITest, test_1pub_1sub_wrong_topic_sync_vasio)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = 0;

    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({"Pub1", {{"TopicA", {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}}, {}});
    pubsubs.push_back({"Sub1", {}, {{"TopicB", {"A"}, {}, defaultMsgSize, numMsgToReceive, 0, {}, {}}}});

    RunSyncTest(pubsubs);
}

// Matching labels
TEST_F(DataPubSubITest, test_1pub_1sub_label_sync_vasio)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = numMsgToPublish;

    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({"Pub1", {{"TopicA", {"A"}, {{"kA", "vA"}, {"kB", "vB"}}, 0, defaultMsgSize, numMsgToPublish}},{}});
    pubsubs.push_back({"Sub1", {}, {{"TopicA", {"A"}, {{"kA", "vA"}, {"kB", ""}}, defaultMsgSize, numMsgToReceive, 1, {{"kA", "vA"}, {"kB", "vB"}}, {}}}});

    RunSyncTest(pubsubs);
}

// Wrong label value -> Expect no reception
TEST_F(DataPubSubITest, test_1pub_1sub_wrong_labels_sync_vasio)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = 0;

    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({"Pub1", {{"TopicA", {"A"}, {{"k", "v"}}, 0, defaultMsgSize, numMsgToPublish}},{}});
    pubsubs.push_back({ "Sub1", {},  {{"TopicA", {"B"}, {{"k", "wrong"}}, defaultMsgSize, numMsgToReceive, 0, {{"k", "v"}}, {}}} });

    RunSyncTest(pubsubs);
}

// Wrong dataExchangeFormat -> Expect no reception
TEST_F(DataPubSubITest, test_1pub_1sub_wrong_dxf_sync_vasio)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = 0;

    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({"Pub1", {{"TopicA",  {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}},{}});
    pubsubs.push_back({"Sub1", {}, {{"TopicA", {"B"}, {}, defaultMsgSize, numMsgToReceive, 0, {}, {}}}});

    RunSyncTest(pubsubs);
}

// Wildcard dataExchangeFormat on subscriber
TEST_F(DataPubSubITest, test_1pub_1sub_wildcard_dxf_sync_vasio)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = numMsgToPublish;

    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({"Pub1", {{"TopicA",  {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}}, {}});
    pubsubs.push_back({ "Sub1", {}, {{"TopicA", {""}, {}, defaultMsgSize, numMsgToReceive, 1, {}, {}}} });

    RunSyncTest(pubsubs);
}

//-----------------------
// Specific handler tests
//-----------------------

// Check for incoming labels
TEST_F(DataPubSubITest, test_2pub_1sub_expectlabels_sync_vasio)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = numMsgToPublish * 2;

    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({"Pub1", {{"TopicA", {"A"}, {{"kA", "vA"}}, 0, defaultMsgSize, numMsgToPublish}},{}});
    pubsubs.push_back({"Pub2", {{"TopicA", {"A"}, {{"kB", "vB"}}, 0, defaultMsgSize, numMsgToPublish}},{}});
    std::vector<std::vector<uint8_t>> expectedDataUnordered;
    for (uint8_t d = 0; d < numMsgToPublish; d++)
    {
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
    }
    pubsubs.push_back({"Sub1", {}, {{"TopicA", {"A"}, {}, defaultMsgSize, numMsgToReceive, 1, expectedDataUnordered, {{"kA", "vA"}, {"kB", "vB"}}, {}}}});

    RunSyncTest(pubsubs);
}

// Use specific handlers and no default handler
TEST_F(DataPubSubITest, test_3pub_1sub_specificHandlers_sync_vasio)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = numMsgToPublish * 3;

    std::vector<PubSubParticipant> pubsubs;
    pubsubs.push_back({ "Pub1", {{"TopicA", {"A"}, {{"kA", "vA"}}, 0, defaultMsgSize, numMsgToPublish}}, {}});
    pubsubs.push_back({ "Pub2", {{"TopicA", {"A"}, {{"kA", "vA"},{"kB", "vB"}}, 0, defaultMsgSize, numMsgToPublish}}, {}});
    // This publisher has the wrong labels and won't be received 
    pubsubs.push_back({ "Pub3", {{"TopicA", {"A"}, {{"kC", "vC"}}, 0, defaultMsgSize, numMsgToPublish}}, {}});
    std::vector<std::vector<uint8_t>> expectedDataUnordered;
    for (uint8_t d = 0; d < numMsgToPublish; d++)
    {
        // First specificDataHandler receives by both publishers, so numMsgToPublish * 3 in total
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
    }
    std::vector<SpecificDataHandlerInfo> specificDataHandlers;
    specificDataHandlers.push_back({ {"A"}, {{"kA", "vA"}} });
    specificDataHandlers.push_back({ {"A"}, {{"kA", "vA"},{"kB", "vB"}} });
    pubsubs.push_back({ "Sub1", {}, 
        {{"TopicA", {"A"}, {{"kA", ""}}, defaultMsgSize, numMsgToReceive, 1, expectedDataUnordered, {{"kA", "vA"}, {"kB", "vB"}, {"kC", "kC"}}, specificDataHandlers}} });

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
                       {{"TopicA", {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}}, // Pub
                       {{"TopicA", {"A"}, {}, defaultMsgSize, numMsgToReceive, 1, {}, {}}}}); // Sub

    RunSyncTest(pubsubs);
}

// 2 pub, 2 sub on a single participant with same topic
TEST_F(DataPubSubITest, test_1_participant_selfdelivery_same_topic)
{
    const uint32_t numMsgToPublish = defaultNumMsgToPublish;
    const uint32_t numMsgToReceive = numMsgToPublish;

    std::vector<PubSubParticipant> pubsubs;
    std::vector<std::vector<uint8_t>> expectedDataUnordered;
    for (uint8_t d = 0; d < numMsgToPublish; d++)
    {
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(defaultMsgSize, d));
    }
    pubsubs.push_back({"PubSub1",
                       {{"TopicA", {"A"}, {}, 0, defaultMsgSize, numMsgToPublish},
                        {"TopicA", {"A"}, {}, 0, defaultMsgSize, numMsgToPublish}},
                       {{"TopicA", {"A"}, {}, defaultMsgSize, numMsgToReceive, 2, expectedDataUnordered, {}, {}},
                        {"TopicA", {"A"}, {}, defaultMsgSize, numMsgToReceive, 2, expectedDataUnordered, {}, {}}}});

    RunSyncTest(pubsubs);
}

//-----------------------------------------------------
// Async tests: No participantController/SimulationTask
//-----------------------------------------------------

// Async with history: Wait for publication before starting the subscriber
TEST_F(DataPubSubITest, test_1pub_1sub_async_history_vasio)
{
    const uint32_t numMsgToPublish = 1;
    const uint32_t numMsgToReceive = numMsgToPublish;

    std::vector<PubSubParticipant> publishers;
    publishers.push_back({"Pub1", {{"TopicA", {"A"}, {}, 1, defaultMsgSize, numMsgToPublish}}, {}});
    std::vector<PubSubParticipant> subscribers;
    subscribers.push_back({"Sub1", {}, {{"TopicA", {"A"}, {}, defaultMsgSize, numMsgToReceive, 1, {}, {}}}});

    RunAsyncTest(publishers, subscribers);
}

// Async with history and specific data handler
TEST_F(DataPubSubITest, test_1pub_1sub_async_history_specifichandler_vasio)
{
    const uint32_t numMsgToPublish = 1;
    const uint32_t numMsgToReceive = numMsgToPublish;

    std::vector<PubSubParticipant> publishers;
    publishers.push_back({"Pub1", {{"TopicA", {"A"}, {}, 1, defaultMsgSize, numMsgToPublish}}, {}});

    std::vector<PubSubParticipant> subscribers;
    std::vector<SpecificDataHandlerInfo> specificDataHandlers;
    specificDataHandlers.push_back({{"A"}, {}});
    subscribers.push_back({"Sub1", {}, {{"TopicA", {"A"}, {}, defaultMsgSize, numMsgToReceive, 1, {}, specificDataHandlers}}});

    RunAsyncTest(publishers, subscribers);
}

#endif

} // anonymous namespace
