// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <cstdlib>

#include "GetTestPid.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ib/extensions/CreateExtension.hpp"

#include "ib/IntegrationBus.hpp"
#include "ib/cfg/string_utils.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/sim/all.hpp"
#include "ParticipantConfiguration.hpp"

namespace {

using namespace std::chrono_literals;
using namespace ib::mw;
using namespace ib::mw::sync;
using namespace ib::cfg;
using namespace ib::sim::data;

const std::string systemMasterName{ "SystemMaster" };

class DataPubSubITest : public testing::Test
{

protected:
    DataPubSubITest()
    {

    }

    struct DataPublisherInfo
    {
        DataPublisherInfo(const std::string& newTopic, const std::string& newMediaType, const std::map<std::string, std::string>& newLabels, uint8_t newHistory,
                          size_t newMessageSizeInBytes,  uint32_t newNumMsgToPublish)
        {
            topic = newTopic;
            dxf.mediaType = newMediaType;
            labels = newLabels;
            history = newHistory;
            messageSizeInBytes = newMessageSizeInBytes;
            numMsgToPublish = newNumMsgToPublish;
        }

        std::string        topic;
        DataExchangeFormat dxf;
        std::map<std::string, std::string> labels;
        uint8_t            history;
        size_t             messageSizeInBytes;
        uint32_t           numMsgToPublish;
        uint32_t           publishMsgCounter{ 0 };
        bool               allSent{ false };
        IDataPublisher*    dataPublisher;
    };

    struct SpecificDataHandlerInfo
    {
        DataExchangeFormat dataExchangeFormat;
        std::map<std::string, std::string> labels;
    };

    struct DataSubscriberInfo
    {
        DataSubscriberInfo(const std::string& newTopic, const std::string& newMediaType,
                           const std::map<std::string, std::string>& newLabels, size_t newMessageSizeInBytes,
                           uint32_t newNumMsgToReceive, uint32_t newExpectedSources,
                           const std::map<std::string, std::string>& newExpectedLabels,
                           const std::vector<SpecificDataHandlerInfo>& newSpecificDataHandlers)
        {
            expectIncreasingData = true;
            topic = newTopic;
            dxf.mediaType = newMediaType;
            labels = newLabels;
            messageSizeInBytes = newMessageSizeInBytes;
            numMsgToReceive = newNumMsgToReceive;
            expectedSources = newExpectedSources;
            expectedLabels = newExpectedLabels;
            specificDataHandlers = newSpecificDataHandlers;
        }
        DataSubscriberInfo(const std::string& newTopic, const std::string& newMediaType,
                           const std::map<std::string, std::string>& newLabels, size_t newMessageSizeInBytes,
                           uint32_t newNumMsgToReceive, uint32_t newExpectedSources,
                           const std::vector<std::vector<uint8_t>>& newExpectedDataUnordered,
                           const std::map<std::string, std::string>& newExpectedLabels,
                           const std::vector<SpecificDataHandlerInfo>& newSpecificDataHandlers)
        {
            expectIncreasingData = false;
            topic = newTopic;
            dxf.mediaType = newMediaType;
            labels = newLabels;
            messageSizeInBytes = newMessageSizeInBytes;
            numMsgToReceive = newNumMsgToReceive;
            expectedSources = newExpectedSources;
            expectedDataUnordered = newExpectedDataUnordered;
            expectedLabels = newExpectedLabels;
            specificDataHandlers = newSpecificDataHandlers;
        }

        std::string                       topic;
        DataExchangeFormat                dxf;
        std::map<std::string, std::string> labels;
        std::map<std::string, std::string> expectedLabels;
        size_t                            messageSizeInBytes;
        uint32_t                          numMsgToReceive;
        bool                              expectIncreasingData;
        std::vector<std::vector<uint8_t>> expectedDataUnordered;
        std::vector<SpecificDataHandlerInfo> specificDataHandlers;

        uint32_t                          expectedSources;
        uint32_t                          newSourceCounter{ 0 };
        uint32_t                          receiveMsgCounter{ 0 };
        bool                              allReceived{false};
        bool allDiscovered{ false };
        IDataSubscriber*                  dataSubscriber;

    };

    struct PublisherParticipant
    {
        PublisherParticipant(const std::string& newName) { name = newName; }
        PublisherParticipant(const std::string& newName, const std::vector<DataPublisherInfo>& newDataPublishers)
        {
            name = newName;
            dataPublishers = newDataPublishers;
        }

        std::string name;
        std::vector<DataPublisherInfo> dataPublishers;
        std::unique_ptr<IComAdapter> comAdapter;
        std::promise<void> readyToStart;
        std::promise<void> allSentPromise;
        bool allSent{false};
    };
    struct SubscriberParticipant
    {
        SubscriberParticipant(const std::string& newName) { name = newName; }
        SubscriberParticipant(const std::string& newName, const std::vector<DataSubscriberInfo>& newDataSubscribers)
        {
            name = newName;
            dataSubscribers = newDataSubscribers;
        }

        std::string name;
        std::vector<DataSubscriberInfo> dataSubscribers;
        std::unique_ptr<IComAdapter> comAdapter;
        std::promise<void> readyToStart;
        std::promise<void> allDiscoveredPromise;
        bool allDiscovered{false};
        std::promise<void> allReceivedPromise;
        bool allReceived{false};

    };

    struct SystemMaster
    {
        std::unique_ptr<IComAdapter> comAdapter;
        ISystemController*           systemController;
        ISystemMonitor*              systemMonitor;
    };

    void ParticipantStatusHandler(const ParticipantStatus& newStatus)
    {
        switch (newStatus.state)
        {
        case ParticipantState::Error:
            systemMaster.systemController->Shutdown();
            break;

        default:
            break;
        }
    }

    void SystemStateHandler(SystemState newState)
    {
        switch (newState)
        {
        case SystemState::Idle:
            for (auto&& name : syncParticipantNames)
            {
                systemMaster.systemController->Initialize(name);
            }
            break;

        case SystemState::Initialized:
            systemMaster.systemController->Run();
            break;

        case SystemState::Stopped:
            systemMaster.systemController->Shutdown();
            break;

        case SystemState::Error:
            systemMaster.systemController->Shutdown();
            break;

        default:
            break;
        }
    }

    void ShutdownAndFailTest(const std::string& reason)
    {
        systemMaster.systemController->Shutdown();
        FAIL() << reason;
    }

    void PublisherThread(PublisherParticipant& participant, uint32_t domainId, bool sync)
    {
        try
        {
            participant.comAdapter =
                ib::CreateSimulationParticipant(ib::cfg::CreateDummyConfiguration(), participant.name, domainId, sync);

            IParticipantController* participantController;
            if (sync)
            {
                participantController = participant.comAdapter->GetParticipantController();
            }

            for (auto& dp : participant.dataPublishers)
            {
                dp.dataPublisher = participant.comAdapter->CreateDataPublisher(dp.topic, dp.dxf, dp.labels, dp.history);
            }

            auto publishTask =
                [this, participantController, &participant]() {
                for (auto& dp : participant.dataPublishers)
                {
                    if (!dp.allSent)
                    {
                        auto data = std::vector<uint8_t>(dp.messageSizeInBytes, dp.publishMsgCounter);
                        dp.dataPublisher->Publish(data);
                        dp.publishMsgCounter++;
                        if (dp.publishMsgCounter >= dp.numMsgToPublish)
                        {
                            dp.allSent = true;
                        }
                    }
                }
            };

            if (sync)
            {
                participantController->SetPeriod(1s);
                participantController->SetSimulationTask([participantController, &participant, publishTask](std::chrono::nanoseconds now) {
                    if (now >= 10s)
                    {
                        publishTask();
                        if (!participant.allSent &&
                            std::all_of(participant.dataPublishers.begin(), participant.dataPublishers.end(),
                                [](DataPublisherInfo dp) { return dp.allSent; }))
                        {
                            participant.allSent = true;
                            participant.allSentPromise.set_value();
                        }
                    }
                });
                auto futureStatus = participant.readyToStart.get_future().wait_for(communicationTimeout);
                EXPECT_EQ(futureStatus, std::future_status::ready)
                    << "Test Failure: Awaiting startup timed out on publisher";

                auto finalStateFuture = participantController->RunAsync();
                auto finalState = finalStateFuture.get();
            }
            else
            {
                auto futureStatus = participant.readyToStart.get_future().wait_for(communicationTimeout);
                EXPECT_EQ(futureStatus, std::future_status::ready)
                    << "Test Failure: Awaiting startup timed out on publisher";

                while (std::none_of(participant.dataPublishers.begin(), participant.dataPublishers.end(),
                                   [](const DataPublisherInfo& dp) { return dp.allSent; }))
                {
                    // NB: For async, the position of the sleep (before or after publishTask) is critical:
                    //  Before: Allows the discovery to complete and the subscriber gets the first message
                    //  After:  The first message might get lost without history
                    // No way around that without counterpart discovery.
                    std::this_thread::sleep_for(asyncDelayBetweenPublication);
                    publishTask();
                }
                participant.allSentPromise.set_value();
            }

            for (const auto& dp : participant.dataPublishers)
            {
                EXPECT_EQ(dp.publishMsgCounter, dp.numMsgToPublish);
            }
        }
        catch (const Misconfiguration& error)
        {
            std::stringstream ss;
            ss << "Invalid configuration: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
        catch (const std::exception& error)
        {
            std::stringstream ss;
            ss << "Something went wrong: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }

    }

    void SubscriberThread(SubscriberParticipant& participant, uint32_t domainId, bool sync)
    {
        try
        {
            participant.comAdapter =
                ib::CreateSimulationParticipant(ib::cfg::CreateDummyConfiguration(), participant.name, domainId, sync);

            IParticipantController* participantController;
            if (sync)
            {
                participantController = participant.comAdapter->GetParticipantController();
            }

            for (auto& ds : participant.dataSubscribers)
            {
                // Already set the promise if no reception is expected
                if (std::all_of(participant.dataSubscribers.begin(), participant.dataSubscribers.end(),
                    [](const auto& ds) {
                    return ds.numMsgToReceive == 0; }))
                {
                    participant.allReceived = true;
                    participant.allReceivedPromise.set_value();
                }
                if (std::all_of(participant.dataSubscribers.begin(), participant.dataSubscribers.end(),
                                   [](const auto& ds) {
                                       return ds.expectedSources == 0;
                                   }))
                {
                    participant.allDiscovered = true;
                    participant.allDiscoveredPromise.set_value();
                }

                auto receptionHandler = [this, sync, &participant, &ds, participantController](
                                            IDataSubscriber* subscriber, const std::vector<uint8_t>& data) {
                    if (!ds.allReceived)
                    {
                        if (ds.expectIncreasingData)
                        {
                            auto expectedData = std::vector<uint8_t>(ds.messageSizeInBytes, ds.receiveMsgCounter);
                            EXPECT_EQ(data, expectedData);
                        }
                        else
                        {
                            auto foundDataIter =
                                std::find(ds.expectedDataUnordered.begin(), ds.expectedDataUnordered.end(), data);
                            EXPECT_EQ(foundDataIter != ds.expectedDataUnordered.end(), true);
                            if (foundDataIter != ds.expectedDataUnordered.end())
                            {
                                ds.expectedDataUnordered.erase(foundDataIter);
                            }
                        }

                        ds.receiveMsgCounter++;
                        if (ds.receiveMsgCounter >= ds.numMsgToReceive) { ds.allReceived = true; }
                    }

                    if (!participant.allReceived &&
                        std::all_of(participant.dataSubscribers.begin(), participant.dataSubscribers.end(),
                                    [](const auto& ds) { return ds.allReceived; }))
                    {
                        participant.allReceived = true;
                        participant.allReceivedPromise.set_value();
                    }
                };

                auto newDataSourceHandler = [this, sync, &participant, &ds, participantController, receptionHandler](
                                                IDataSubscriber* subscriber, const std::string& topic,
                                                const DataExchangeFormat& dataExchangeFormat,
                                                const std::map<std::string, std::string>& labels) {
                    ds.newSourceCounter++;
                    if (!ds.allDiscovered)
                    {
                        if (ds.newSourceCounter >= ds.expectedSources)
                        {
                            ds.allDiscovered = true;
                        }
                    }

                    for (auto const& kv : labels)
                    {
                        EXPECT_EQ(ds.expectedLabels[kv.first], kv.second);
                    }
                    for (auto const& sp : ds.specificDataHandlers)
                    {
                        if (sp.labels == labels)
                        {
                            subscriber->RegisterSpecificDataHandler(sp.dataExchangeFormat, sp.labels, receptionHandler);
                        }
                    }

                    if (!participant.allDiscovered
                        && std::all_of(participant.dataSubscribers.begin(), participant.dataSubscribers.end(),
                                       [](const auto& ds) {
                                           return ds.allDiscovered;
                                       }))
                    {
                        participant.allDiscovered = true;
                        participant.allDiscoveredPromise.set_value();
                    }
                };
                if (ds.specificDataHandlers.empty())
                {
                    ds.dataSubscriber = participant.comAdapter->CreateDataSubscriber(
                        ds.topic, ds.dxf, ds.labels, receptionHandler, newDataSourceHandler);
                }
                else
                {
                    ds.dataSubscriber = participant.comAdapter->CreateDataSubscriber(
                        ds.topic, ds.dxf, ds.labels, nullptr, newDataSourceHandler);
                }

            }

            if (sync)
            {
                participantController->SetPeriod(1s);
                participantController->SetSimulationTask([](std::chrono::nanoseconds now) {
                    });

                auto futureStatus = participant.readyToStart.get_future().wait_for(communicationTimeout);
                EXPECT_EQ(futureStatus, std::future_status::ready)
                    << "Test Failure: Awaiting startup timed out on subscriber";

                auto finalStateFuture = participantController->RunAsync();
                auto finalState = finalStateFuture.get();
            }
            else
            {
                auto futureStatus = participant.readyToStart.get_future().wait_for(communicationTimeout);
                EXPECT_EQ(futureStatus, std::future_status::ready)
                    << "Test Failure: Awaiting startup timed out on subscriber";

                futureStatus = participant.allReceivedPromise.get_future().wait_for(communicationTimeout);
                EXPECT_EQ(futureStatus, std::future_status::ready) << "Test Failure: Awaiting reception timed out on async participant";
            }

            for (const auto& ds : participant.dataSubscribers)
            {
                EXPECT_EQ(ds.receiveMsgCounter, ds.numMsgToReceive);
            }
        }
        catch (const Misconfiguration& error)
        {
            std::stringstream ss;
            ss << "Invalid configuration: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
        catch (const std::exception& error)
        {
            std::stringstream ss;
            ss << "Something went wrong: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }

    }

    void RunRegistry(uint32_t domainId)
    {
        try
        {
            // TODO: Revise
            registry = ib::extensions::CreateIbRegistry(ib::cfg::Config{});
            registry->ProvideDomain(domainId);
        }
        catch (const Misconfiguration& error)
        {
            std::stringstream ss;
            ss << "Invalid configuration: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
        catch (const std::exception& error)
        {
            std::stringstream ss;
            ss << "Something went wrong: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
    }

    void RunSystemMaster(uint32_t domainId)
    {
        try
        {
            systemMaster.comAdapter =
                ib::CreateSimulationParticipant(ib::cfg::CreateDummyConfiguration(), systemMasterName, domainId, false);

            systemMaster.systemController = systemMaster.comAdapter->GetSystemController();
            systemMaster.systemMonitor = systemMaster.comAdapter->GetSystemMonitor();

            systemMaster.systemMonitor->SetSynchronizedParticipants(syncParticipantNames);

            systemMaster.systemMonitor->RegisterSystemStateHandler(
                [this](SystemState newState) { SystemStateHandler(newState); });

            systemMaster.systemMonitor->RegisterParticipantStatusHandler(
                [this](const ParticipantStatus& newStatus) {
                    ParticipantStatusHandler(newStatus);
                });
        }
        catch (const Misconfiguration& error)
        {
            std::stringstream ss;
            ss << "Invalid configuration: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
        catch (const std::exception& error)
        {
            std::stringstream ss;
            ss << "Something went wrong: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
    }

    void RunPublishers(std::vector<PublisherParticipant>& publishers, uint32_t domainId, bool sync)
    {
        try
        {
            for (auto& publisherParticipant : publishers)
            {
                pubSubThreads.emplace_back(
                    [this, &publisherParticipant, domainId, sync] { PublisherThread(publisherParticipant, domainId, sync); });
            }

        }
        catch (const Misconfiguration& error)
        {
            std::stringstream ss;
            ss << "Invalid configuration: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
        catch (const std::exception& error)
        {
            std::stringstream ss;
            ss << "Something went wrong: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
    }

    void RunSubscribers(std::vector<SubscriberParticipant>& subscribers, uint32_t domainId, bool sync)
    {
        try
        {
            for (auto& subscriberParticipant : subscribers)
            {
                pubSubThreads.emplace_back(
                    [this, &subscriberParticipant, domainId, sync] { SubscriberThread(subscriberParticipant, domainId, sync); });
            }
        }
        catch (const Misconfiguration& error)
        {
            std::stringstream ss;
            ss << "Invalid configuration: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
        catch (const std::exception& error)
        {
            std::stringstream ss;
            ss << "Something went wrong: " << error.what() << std::endl;
            ShutdownAndFailTest(ss.str());
        }
    }

    void JoinPubSubThreads()
    {
        for (auto&& thread : pubSubThreads)
        {
            thread.join();
        }
    }

    void SetupSystem(uint32_t domainId, bool sync, std::vector<PublisherParticipant>& publishers,
                     std::vector<SubscriberParticipant>& subscribers)
    {
        if (sync)
        {
            for (auto& s : subscribers)
            {
                syncParticipantNames.push_back(s.name);
            }
            for (auto& p : publishers)
            {
                syncParticipantNames.push_back(p.name);
            }
        }

        RunRegistry(domainId);

        if (sync)
        {
            RunSystemMaster(domainId);
        }
    }

    void WaitForAllDiscoveredAndStart(std::vector<SubscriberParticipant>& subscribers,
                                      std::vector<PublisherParticipant>& publishers)
    {
        for (auto& s : subscribers)
        {
            auto futureStatus = s.allDiscoveredPromise.get_future().wait_for(communicationTimeout);
            EXPECT_EQ(futureStatus, std::future_status::ready)
                << "Test Failure: Awaiting source discovery timed out on subscriber";
        }

        for (auto& s : subscribers)
        {
            s.readyToStart.set_value();
        }
        for (auto& p : publishers)
        {
            p.readyToStart.set_value();
        }
    }

    void StopSimOnAllSentAndReceived(std::vector<PublisherParticipant>&  publishers,
                                  std::vector<SubscriberParticipant>& subscribers, bool sync)
    {
        if (sync)
        {
            for (auto& p : publishers)
            {
                auto futureStatus = p.allSentPromise.get_future().wait_for(communicationTimeout);
                EXPECT_EQ(futureStatus, std::future_status::ready)
                    << "Test Failure: Awaiting transmission timed out on publisher";
            }
            for (auto& s : subscribers)
            {
                auto futureStatus = s.allReceivedPromise.get_future().wait_for(communicationTimeout);
                EXPECT_EQ(futureStatus, std::future_status::ready)
                    << "Test Failure: Awaiting reception timed out on subscriber";
            }
            systemMaster.systemController->Stop();
        }
    }

    void ShutdownSystem()
    {
        pubSubThreads.clear();
        systemMaster.comAdapter.reset();
        registry.reset();
    }

    ib::cfg::Config DummyCfg(const std::string& participantName, bool sync)
    {
        ib::cfg::Config dummyCfg;
        ib::cfg::Participant dummyParticipant;
        if (sync)
        {
            dummyParticipant.participantController = ib::cfg::ParticipantController{};
        }
        dummyParticipant.name = participantName;
        dummyCfg.simulationSetup.participants.push_back(dummyParticipant);
        return dummyCfg;
    }

protected:
    std::vector<std::string> syncParticipantNames;
    std::unique_ptr<ib::extensions::IIbRegistry> registry;
    SystemMaster systemMaster;
    std::vector<std::thread> pubSubThreads;

    std::chrono::milliseconds communicationTimeout{20000ms};
    std::chrono::milliseconds asyncDelayBetweenPublication{500ms};
};

#if defined(IB_MW_HAVE_VASIO)

//--------------------------------------
// Sync tests: Publish in SimulationTask
//--------------------------------------

// One publisher participant, one subscriber participant
TEST_F(DataPubSubITest, test_1pub_1sub_sync_vasio)
{
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numMsgToPublish = 3;
    const uint32_t numMsgToReceive = numMsgToPublish;
    const size_t messageSize = 3;
    const bool sync = true;

    std::vector<PublisherParticipant> publishers;
    std::vector<SubscriberParticipant> subscribers;

    publishers.push_back({ "Pub1", {{"TopicA",  "A", {}, 0, messageSize, numMsgToPublish}} });
    subscribers.push_back({ "Sub1", {{"TopicA",  "A", {}, messageSize, numMsgToReceive, 1, {}, {} }} });

    SetupSystem(domainId, sync, publishers, subscribers);

    RunSubscribers(subscribers, domainId, sync);
    RunPublishers(publishers, domainId, sync);

    WaitForAllDiscoveredAndStart(subscribers, publishers);

    StopSimOnAllSentAndReceived(publishers, subscribers, sync);

    JoinPubSubThreads();

    ShutdownSystem();
}

// Large messages
TEST_F(DataPubSubITest, test_1pub_1sub_largemsg_sync_vasio)
{
    const auto     middleware = Middleware::VAsio;
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numMsgToPublish = 3;
    const uint32_t numMsgToReceive = numMsgToPublish;
    const size_t   messageSize = 250000;
    const bool     sync = true;

    std::vector<PublisherParticipant>  publishers;
    std::vector<SubscriberParticipant> subscribers;
    publishers.push_back({"Pub1", {{"TopicA",  "A", {}, 0, messageSize, numMsgToPublish}}});
    subscribers.push_back({"Sub1", {{"TopicA",  "A", {}, messageSize, numMsgToReceive, 1, {}, {}}}});

    SetupSystem(domainId, sync, publishers, subscribers);

    RunSubscribers(subscribers, domainId, sync);
    RunPublishers(publishers, domainId, sync);

    WaitForAllDiscoveredAndStart(subscribers, publishers);

    StopSimOnAllSentAndReceived(publishers, subscribers, sync);

    JoinPubSubThreads();

    ShutdownSystem();
}

// 100 topics on one publisher/subscriber participant
TEST_F(DataPubSubITest, test_1pub_1sub_100topics_sync_vasio)
{
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numMsgToPublish = 1;
    const uint32_t numMsgToReceive = numMsgToPublish;
    const size_t   messageSize = 3;
    const bool     sync = true;
    const int      numTopics = 100;

    std::vector<PublisherParticipant>  publishers;
    std::vector<SubscriberParticipant> subscribers;
    publishers.push_back({ "Pub1" });
    subscribers.push_back({ "Sub1" });
    for (int i = 0; i < numTopics; i++)
    {
        std::string topic = std::to_string(i);
        DataPublisherInfo  pInfo{ topic,  "A", {}, 1, messageSize, numMsgToPublish };
        DataSubscriberInfo sInfo{ topic,  "A", {},    messageSize, numMsgToPublish, 1, {}, {} };
        publishers[0].dataPublishers.push_back(std::move(pInfo));
        subscribers[0].dataSubscribers.push_back(std::move(sInfo));
    }

    SetupSystem(domainId, sync, publishers, subscribers);

    RunSubscribers(subscribers, domainId, sync);
    RunPublishers(publishers, domainId, sync);

    WaitForAllDiscoveredAndStart(subscribers, publishers);

    StopSimOnAllSentAndReceived(publishers, subscribers, sync);

    JoinPubSubThreads();

    ShutdownSystem();
}

// One publisher participant, two subscribers participants on same topic
TEST_F(DataPubSubITest, test_1pub_2sub_sync_vasio)
{
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numMsgToPublish = 3;
    const uint32_t numMsgToReceive = numMsgToPublish;
    const size_t   messageSize = 3;
    const bool     sync = true;

    std::vector<PublisherParticipant>  publishers;
    std::vector<SubscriberParticipant> subscribers;
    publishers.push_back({"Pub1", {{"TopicA",  "A", {}, 0, messageSize, numMsgToPublish}}});
    subscribers.push_back({"Sub1", {{"TopicA",  "A", {}, messageSize, numMsgToReceive, 1, {}, {}}}});
    subscribers.push_back({"Sub2", {{"TopicA",  "A", {}, messageSize, numMsgToReceive, 1, {}, {}}}});

    SetupSystem(domainId, sync, publishers, subscribers);

    RunSubscribers(subscribers, domainId, sync);
    RunPublishers(publishers, domainId, sync);

    WaitForAllDiscoveredAndStart(subscribers, publishers);

    StopSimOnAllSentAndReceived(publishers, subscribers, sync);

    JoinPubSubThreads();

    ShutdownSystem();
}

// Two publisher participants, one subscriber participant on same topic: Expect all to arrive but arbitrary reception order
TEST_F(DataPubSubITest, test_2pub_1sub_sync_vasio)
{
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numMsgToPublish = 3;
    const uint32_t numMsgToReceive = numMsgToPublish * 2;
    const size_t   messageSize = 3;
    const bool     sync = true;

    std::vector<PublisherParticipant>  publishers;
    std::vector<SubscriberParticipant> subscribers;
    publishers.push_back({"Pub1", {{"TopicA",  "A", {}, 0, messageSize, numMsgToPublish}}});
    publishers.push_back({"Pub2", {{"TopicA",  "A", {}, 0, messageSize, numMsgToPublish}}});
    std::vector<std::vector<uint8_t>> expectedDataUnordered;
    for (uint32_t d = 0; d < numMsgToPublish; d++)
    {
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(messageSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(messageSize, d));
    }
    subscribers.push_back({ "Sub1", {{"TopicA",  "A", {}, messageSize, numMsgToReceive, 2, expectedDataUnordered, {}, {}}} });

    SetupSystem(domainId, sync, publishers, subscribers);

    RunSubscribers(subscribers, domainId, sync);
    RunPublishers(publishers, domainId, sync);

    WaitForAllDiscoveredAndStart(subscribers, publishers);

    StopSimOnAllSentAndReceived(publishers, subscribers, sync);

    JoinPubSubThreads();

    ShutdownSystem();
}

// Seven participants, multiple topics
TEST_F(DataPubSubITest, test_3pub_4sub_4topics_sync_vasio)
{
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numMsgToPublish = 3;
    const size_t   messageSize = 3;
    const bool     sync = true;

    std::vector<PublisherParticipant>  publishers;
    std::vector<SubscriberParticipant> subscribers;
    publishers.push_back({ "Pub1",  {{"TopicA",  "A", {}, 0, messageSize, numMsgToPublish}, {"TopicB", "B", {}, 0, messageSize, numMsgToPublish}} });
    publishers.push_back({ "Pub2",  {{"TopicA",  "A", {}, 0, messageSize, numMsgToPublish}, {"TopicC", "C", {}, 0, messageSize, numMsgToPublish}} });
    publishers.push_back({ "Pub3",  {{"TopicA",  "A", {}, 0, messageSize, numMsgToPublish}, {"TopicD", "D", {}, 0, messageSize, numMsgToPublish}} });

    std::vector<std::vector<uint8_t>> expectedDataUnordered;
    for (uint32_t d = 0; d < numMsgToPublish; d++)
    {
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(messageSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(messageSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(messageSize, d));
    }
    subscribers.push_back({ "Sub1", {{"TopicA",  "A", {}, messageSize, numMsgToPublish * 3, 3,  expectedDataUnordered, {}, {}}} });
    subscribers.push_back({"Sub2", {{"TopicB", "B", {}, messageSize, numMsgToPublish, 1, {}, {}}} });
    subscribers.push_back({ "Sub3", {{"TopicC", "C", {}, messageSize, numMsgToPublish, 1, {}, {}}} });
    subscribers.push_back({ "Sub4", {{"TopicD", "D", {}, messageSize, numMsgToPublish, 1, {}, {}}}});

    SetupSystem(domainId, sync, publishers, subscribers);

    RunSubscribers(subscribers, domainId, sync);
    RunPublishers(publishers, domainId, sync);

    WaitForAllDiscoveredAndStart(subscribers, publishers);

    StopSimOnAllSentAndReceived(publishers, subscribers, sync);

    JoinPubSubThreads();

    ShutdownSystem();
}

// Wrong topic -> Expect no reception
TEST_F(DataPubSubITest, test_1pub_1sub_wrong_topic_sync_vasio)
{
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numMsgToPublish = 3;
    const uint32_t numMsgToReceive = 0;
    const size_t   messageSize = 3;
    const bool     sync = true;

    std::vector<PublisherParticipant>  publishers;
    std::vector<SubscriberParticipant> subscribers;
    publishers.push_back({"Pub1", {{"TopicA",  "A", {}, 0, messageSize, numMsgToPublish}}});
    subscribers.push_back({"Sub1", {{"TopicB",  "A", {}, messageSize, numMsgToReceive, 0, {}, {}}}});

    SetupSystem(domainId, sync, publishers, subscribers);

    RunSubscribers(subscribers, domainId, sync);
    RunPublishers(publishers, domainId, sync);

    WaitForAllDiscoveredAndStart(subscribers, publishers);

    StopSimOnAllSentAndReceived(publishers, subscribers, sync);

    JoinPubSubThreads();

    ShutdownSystem();
}

// Matching labels
TEST_F(DataPubSubITest, test_1pub_1sub_label_sync_vasio)
{
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numMsgToPublish = 3;
    const uint32_t numMsgToReceive = 3;
    const size_t messageSize = 3;
    const bool sync = true;

    std::vector<PublisherParticipant> publishers;
    std::vector<SubscriberParticipant> subscribers;
    publishers.push_back({"Pub1", {{"TopicA", "A", {{"kA", "vA"}, {"kB", "vB"}}, 0, messageSize, numMsgToPublish}}});
    subscribers.push_back({"Sub1", {{"TopicA", "A", {{"kA", "vA"}, {"kB", ""}}, messageSize, numMsgToReceive, 1, {{"kA", "vA"}, {"kB", "vB"}}, {}}}});

    SetupSystem(domainId, sync, publishers, subscribers);

    RunSubscribers(subscribers, domainId, sync);
    RunPublishers(publishers, domainId, sync);

    WaitForAllDiscoveredAndStart(subscribers, publishers);

    StopSimOnAllSentAndReceived(publishers, subscribers, sync);

    JoinPubSubThreads();

    ShutdownSystem();
}

// Wrong label value -> Expect no reception
TEST_F(DataPubSubITest, test_1pub_1sub_wrong_labels_sync_vasio)
{
    
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numMsgToPublish = 3;
    const uint32_t numMsgToReceive = 0;
    const size_t messageSize = 3;
    const bool sync = true;

    std::vector<PublisherParticipant> publishers;
    std::vector<SubscriberParticipant> subscribers;
    publishers.push_back({"Pub1", {{"TopicA", "A", {{"k", "v"}}, 0, messageSize, numMsgToPublish}}});
    subscribers.push_back({"Sub1", {{"TopicA", "B", {{"k", "wrong"}}, messageSize, numMsgToReceive, 0, {{"k", "v"}}, {}}}});

    SetupSystem(domainId, sync, publishers, subscribers);

    RunSubscribers(subscribers, domainId, sync);
    RunPublishers(publishers, domainId, sync);

    WaitForAllDiscoveredAndStart(subscribers, publishers);

    StopSimOnAllSentAndReceived(publishers, subscribers, sync);

    JoinPubSubThreads();

    ShutdownSystem();
}

// Wrong dataExchangeFormat -> Expect no reception
TEST_F(DataPubSubITest, test_1pub_1sub_wrong_dxf_sync_vasio)
{
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numMsgToPublish = 3;
    const uint32_t numMsgToReceive = 0; 
    const size_t   messageSize = 3;
    const bool     sync = true;

    std::vector<PublisherParticipant>  publishers;
    std::vector<SubscriberParticipant> subscribers;
    publishers.push_back({"Pub1", {{"TopicA",  "A", {}, 0, messageSize, numMsgToPublish}}});
    subscribers.push_back({"Sub1", {{"TopicA", "B", {}, messageSize, numMsgToReceive, 0, {}, {}}}});

    SetupSystem(domainId, sync, publishers, subscribers);

    RunSubscribers(subscribers, domainId, sync);
    RunPublishers(publishers, domainId, sync);

    WaitForAllDiscoveredAndStart(subscribers, publishers);

    StopSimOnAllSentAndReceived(publishers, subscribers, sync);

    JoinPubSubThreads();

    ShutdownSystem();
}

// Wildcard dataExchangeFormat on subscriber
TEST_F(DataPubSubITest, test_1pub_1sub_wildcard_dxf_sync_vasio)
{
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numMsgToPublish = 3;
    const uint32_t numMsgToReceive = numMsgToPublish;
    const size_t   messageSize = 3;
    const bool     sync = true;

    std::vector<PublisherParticipant>  publishers;
    std::vector<SubscriberParticipant> subscribers;
    publishers.push_back({"Pub1", {{"TopicA",  "A", {}, 0, messageSize, numMsgToPublish}}});
    subscribers.push_back({"Sub1", {{"TopicA", "", {}, messageSize, numMsgToReceive, 1, {}, {}}}});

    SetupSystem(domainId, sync, publishers, subscribers);

    RunSubscribers(subscribers, domainId, sync);
    RunPublishers(publishers, domainId, sync);

    WaitForAllDiscoveredAndStart(subscribers, publishers);

    StopSimOnAllSentAndReceived(publishers, subscribers, sync);

    JoinPubSubThreads();

    ShutdownSystem();
}

//-----------------------
// Specific handler tests
//-----------------------

// Check for incoming labels
TEST_F(DataPubSubITest, test_2pub_1sub_expectlabels_sync_vasio)
{
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numMsgToPublish = 3;
    const uint32_t numMsgToReceive = numMsgToPublish * 2;
    const size_t messageSize = 3;
    const bool sync = true;

    std::vector<PublisherParticipant> publishers;
    std::vector<SubscriberParticipant> subscribers;
    publishers.push_back({"Pub1", {{"TopicA", "A", {{"kA", "vA"}}, 0, messageSize, numMsgToPublish}}});
    publishers.push_back({"Pub2", {{"TopicA", "A", {{"kB", "vB"}}, 0, messageSize, numMsgToPublish}}});
    std::vector<std::vector<uint8_t>> expectedDataUnordered;
    for (uint32_t d = 0; d < numMsgToPublish; d++)
    {
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(messageSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(messageSize, d));
    }
    subscribers.push_back({"Sub1", {{"TopicA", "A", {}, messageSize, numMsgToReceive, 1, expectedDataUnordered, {{"kA", "vA"}, {"kB", "vB"}}, {}}}});

    SetupSystem(domainId, sync, publishers, subscribers);

    RunSubscribers(subscribers, domainId, sync);
    RunPublishers(publishers, domainId, sync);

    WaitForAllDiscoveredAndStart(subscribers, publishers);

    StopSimOnAllSentAndReceived(publishers, subscribers, sync);

    JoinPubSubThreads();

    ShutdownSystem();
}

// Use specific handlers and no default handler
TEST_F(DataPubSubITest, test_3pub_1sub_specificHandlers_sync_vasio)
{
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numMsgToPublish = 3;
    const uint32_t numMsgToReceive = numMsgToPublish * 3;
    const size_t messageSize = 3;
    const bool sync = true;

    std::vector<PublisherParticipant> publishers;
    std::vector<SubscriberParticipant> subscribers;
    publishers.push_back({ "Pub1", {{"TopicA", "A", {{"kA", "vA"}}, 0, messageSize, numMsgToPublish}} });
    publishers.push_back({ "Pub2", {{"TopicA", "A", {{"kA", "vA"},{"kB", "vB"}}, 0, messageSize, numMsgToPublish}} });
    // This publisher has the wrong labels and won't be received 
    publishers.push_back({ "Pub3", {{"TopicA", "A", {{"kC", "vC"}}, 0, messageSize, numMsgToPublish}} });
    std::vector<std::vector<uint8_t>> expectedDataUnordered;
    for (uint32_t d = 0; d < numMsgToPublish; d++)
    {
        // First specificDataHandler receives by both publishers, so numMsgToPublish * 3 in total
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(messageSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(messageSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(messageSize, d));
    }
    std::vector<SpecificDataHandlerInfo> specificDataHandlers;
    specificDataHandlers.push_back({ "A", {{"kA", "vA"}} });
    specificDataHandlers.push_back({ "A", {{"kA", "vA"},{"kB", "vB"}} });
    subscribers.push_back({ "Sub1",
        {{"TopicA", "A", {{"kA", ""}}, messageSize, numMsgToReceive, 1, expectedDataUnordered, {{"kA", "vA"}, {"kB", "vB"}, {"kC", "kC"}}, specificDataHandlers}} });

    SetupSystem(domainId, sync, publishers, subscribers);

    RunSubscribers(subscribers, domainId, sync);
    RunPublishers(publishers, domainId, sync);

    WaitForAllDiscoveredAndStart(subscribers, publishers);

    StopSimOnAllSentAndReceived(publishers, subscribers, sync);

    JoinPubSubThreads();

    ShutdownSystem();
}

//-----------------------------------------------------
// Async tests: No participantController/SimulationTask
//-----------------------------------------------------

// Async with history: Wait for publication before starting the subscriber
TEST_F(DataPubSubITest, test_1pub_1sub_async_history_vasio)
{
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numMsgToPublish = 1;
    const uint32_t numMsgToReceive = numMsgToPublish;
    const size_t   messageSize = 3;
    const bool     sync = false;

    std::vector<PublisherParticipant>  publishers;
    std::vector<SubscriberParticipant> subscribers;
    publishers.push_back ({ "Pub1", { { "TopicA",  "A", {}, 1, messageSize, numMsgToPublish } } });
    subscribers.push_back({ "Sub1", { { "TopicA",  "A", {},    messageSize, numMsgToReceive, 1, {}, {} } } });

    SetupSystem(domainId, sync, publishers, subscribers);

    RunPublishers(publishers, domainId, sync);
    for (auto& p : publishers)
    {
        p.readyToStart.set_value();
        auto futureStatus = p.allSentPromise.get_future().wait_for(communicationTimeout);
        EXPECT_EQ(futureStatus, std::future_status::ready)
            << "Test Failure: Awaiting transmission timed out on publisher";
    }
    RunSubscribers(subscribers, domainId, sync);
    for (auto& s : subscribers)
    {
        s.readyToStart.set_value();
    }

    JoinPubSubThreads();

    ShutdownSystem();
}
#endif

} // anonymous namespace
