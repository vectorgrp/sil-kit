// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <cstdlib>

#include "GetTestPid.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ib/IntegrationBus.hpp"
#include "ib/cfg/string_utils.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/sim/all.hpp"

#include "ib/cfg/Config.hpp"
#include "ib/cfg/ConfigBuilder.hpp"
#include "ib/extensions/CreateExtension.hpp"


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
        DataPublisherInfo(const std::string& newTopic, const std::string& newMimeType, uint8_t newHistory,
                          size_t newMessageSizeInBytes,  uint32_t newNumMsgToPublish)
        {
            topic = newTopic;
            dxf.mimeType = newMimeType;
            history = newHistory;
            messageSizeInBytes = newMessageSizeInBytes;
            numMsgToPublish = newNumMsgToPublish;
        }

        std::string        topic;
        DataExchangeFormat dxf;
        uint8_t            history;
        size_t             messageSizeInBytes;
        uint32_t           numMsgToPublish;
        uint32_t           publishMsgCounter{ 0 };
        bool               allSent{ false };
        IDataPublisher*    dataPublisher;
    };

    struct DataSubscriberInfo
    {
        DataSubscriberInfo(const std::string& newTopic, const std::string& newMimeType, size_t newMessageSizeInBytes,
                           uint32_t newNumMsgToReceive)
        {
            expectIncreasingData = true;
            topic = newTopic;
            dxf.mimeType = newMimeType;
            messageSizeInBytes = newMessageSizeInBytes;
            numMsgToReceive = newNumMsgToReceive;
        }
        DataSubscriberInfo(const std::string& newTopic, const std::string& newMimeType, size_t newMessageSizeInBytes,
                           uint32_t newNumMsgToReceive, const std::vector<std::vector<uint8_t>>& newExpectedDataUnordered)
        {
            expectIncreasingData = false;
            topic = newTopic;
            dxf.mimeType = newMimeType;
            messageSizeInBytes = newMessageSizeInBytes;
            numMsgToReceive = newNumMsgToReceive;
            expectedDataUnordered = newExpectedDataUnordered;
        }

        std::string                       topic;
        DataExchangeFormat                dxf;
        size_t                            messageSizeInBytes;
        uint32_t                          numMsgToReceive;
        bool                              expectIncreasingData;
        std::vector<std::vector<uint8_t>> expectedDataUnordered;
        uint32_t                          receiveMsgCounter{ 0 };
        bool                              allReceived{false};
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

        std::string                    name;
        std::vector<DataPublisherInfo> dataPublishers;
        std::unique_ptr<IComAdapter>   comAdapter;
        std::promise<void>             startedPromise;
        std::promise<void>             allSentPromise;
        bool                           allSent{ false };
    };
    struct SubscriberParticipant
    {
        SubscriberParticipant(const std::string& newName) { name = newName; }
        SubscriberParticipant(const std::string& newName, const std::vector<DataSubscriberInfo>& newDataSubscribers)
        {
            name = newName;
            dataSubscribers = newDataSubscribers;
        }

        std::string                     name;
        std::vector<DataSubscriberInfo> dataSubscribers;
        std::unique_ptr<IComAdapter>    comAdapter;
        std::promise<void>              startedPromise;
        std::promise<void>              allReceivedPromise;
        bool                            allReceived{ false };

    };

    struct SystemMaster
    {
        std::unique_ptr<IComAdapter> comAdapter;
        ISystemController*           systemController;
        ISystemMonitor*              systemMonitor;
    };

    auto BuildConfig(std::vector<PublisherParticipant>& publishers, std::vector<SubscriberParticipant>& subscribers, Middleware middleware, bool sync) -> Config
    {
        const auto level = logging::Level::Info;
        ConfigBuilder config("PubSubTestConfigGenerated");
        auto&& simulationSetup = config.SimulationSetup();

        auto syncType = SyncType::Unsynchronized;
        if (sync)
        {
            syncType = SyncType::DistributedTimeQuantum;
        }

        uint32_t participantCount = static_cast<uint32_t>(publishers.size() + subscribers.size());
        std::vector<ParticipantBuilder*> participants;
        for (const auto& pub : publishers)
        {
            for (const auto& dp : pub.dataPublishers)
            {
                simulationSetup.AddOrGetLink(Link::Type::DataMessage, dp.topic);
            }
            auto&& participant = simulationSetup.AddParticipant(pub.name);
            participant.ConfigureLogger().WithFlushLevel(level).AddSink(Sink::Type::Stdout).WithLogLevel(level);
            if (sync)
            {
                participant.AddParticipantController().WithSyncType(syncType);
            }
            for (const auto& dp : pub.dataPublishers)
            {
                participant.AddDataPublisher(dp.topic).WithLink(dp.topic);
            }
            participants.emplace_back(&participant);
        }
        for (const auto& sub : subscribers)
        {
            for (const auto& dp : sub.dataSubscribers)
            {
                simulationSetup.AddOrGetLink(Link::Type::DataMessage, dp.topic);
            }
            auto&& participant = simulationSetup.AddParticipant(sub.name);
            participant.ConfigureLogger().WithFlushLevel(level).AddSink(Sink::Type::Stdout).WithLogLevel(level);
            if (sync)
            {
                participant.AddParticipantController().WithSyncType(syncType);
            }
            for (const auto& dp : sub.dataSubscribers)
            {
                participant.AddDataSubscriber(dp.topic).WithLink(dp.topic);
            }
            participants.emplace_back(&participant);
        }

        simulationSetup.AddParticipant(systemMasterName);

        config.WithActiveMiddleware(middleware);
        return config.Build();
    }

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
            for (auto&& participant : ibConfig.simulationSetup.participants)
            {
                if (participant.name == systemMasterName)
                    continue;
                systemMaster.systemController->Initialize(participant.name);
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
            participant.comAdapter = ib::CreateComAdapter(ibConfig, participant.name, domainId);

            IParticipantController* participantController;
            if (sync)
            {
                participantController = participant.comAdapter->GetParticipantController();
            }

            for (auto& dp : participant.dataPublishers)
            {
                dp.dataPublisher = participant.comAdapter->CreateDataPublisher(dp.topic, dp.dxf, dp.history);
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
                    publishTask();
                    if (!participant.allSent && 
                        std::all_of(participant.dataPublishers.begin(), participant.dataPublishers.end(),
                                    [](DataPublisherInfo dp) { return dp.allSent; }))
                    {
                        participant.allSent = true;
                        participant.allSentPromise.set_value();
                    }
                });
                participant.startedPromise.set_value();
                auto finalStateFuture = participantController->RunAsync();
                auto finalState = finalStateFuture.get();
            }
            else
            {
                participant.startedPromise.set_value();
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
            participant.comAdapter = ib::CreateComAdapter(ibConfig, participant.name, domainId);
            IParticipantController* participantController;
            if (sync)
            {
                participantController = participant.comAdapter->GetParticipantController();
            }

            for (auto& ds : participant.dataSubscribers)
            {
                if (std::all_of(participant.dataSubscribers.begin(), participant.dataSubscribers.end(),
                    [](DataSubscriberInfo ds) {
                    return ds.numMsgToReceive == 0; }))
                {
                    participant.allReceived = true;
                    participant.allReceivedPromise.set_value();
                }


                ds.dataSubscriber = participant.comAdapter->CreateDataSubscriber(ds.topic, ds.dxf,
                    [this, sync, &participant, &ds, participantController](IDataSubscriber* subscriber, const std::vector<uint8_t>& data, const DataExchangeFormat& dataExchangeFormat)
                    {
                        if (!ds.allReceived)
                        {
                            if (ds.expectIncreasingData)
                            {
                                auto expectedData = std::vector<uint8_t>(ds.messageSizeInBytes, ds.receiveMsgCounter);
                                EXPECT_EQ(data, expectedData);
                            }
                            else
                            {
                                auto foundDataIter = std::find(ds.expectedDataUnordered.begin(), ds.expectedDataUnordered.end(), data);
                                EXPECT_EQ(foundDataIter != ds.expectedDataUnordered.end(), true);
                                if (foundDataIter != ds.expectedDataUnordered.end()) {
                                    ds.expectedDataUnordered.erase(foundDataIter);
                                }
                            }
                            
                            ds.receiveMsgCounter++;
                            if (ds.receiveMsgCounter >= ds.numMsgToReceive)
                            {
                                ds.allReceived = true;
                            }
                        }

                        if (!participant.allReceived && 
                            std::all_of(participant.dataSubscribers.begin(), participant.dataSubscribers.end(),
                                        [](DataSubscriberInfo ds) { return ds.allReceived; }))
                        {
                            participant.allReceived = true;
                            participant.allReceivedPromise.set_value();
                        }
                    }
                    );
            }

            if (sync)
            {
                participantController->SetPeriod(1s);
                participantController->SetSimulationTask([](std::chrono::nanoseconds now) {
                    });

                participant.startedPromise.set_value();
                auto finalStateFuture = participantController->RunAsync();
                auto finalState = finalStateFuture.get();
            }
            else
            {
                participant.startedPromise.set_value();
                auto futureStatus = participant.allReceivedPromise.get_future().wait_for(communicationTimeout);
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

    void RunVasioRegistry(uint32_t domainId)
    {
        try
        {
            registry = ib::extensions::CreateIbRegistry(ibConfig);
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
            systemMaster.comAdapter = ib::CreateComAdapter(ibConfig, systemMasterName, domainId);
            systemMaster.systemController = systemMaster.comAdapter->GetSystemController();
            systemMaster.systemMonitor = systemMaster.comAdapter->GetSystemMonitor();

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
                     std::vector<SubscriberParticipant>& subscribers, Middleware middleware)
    {
        ibConfig = BuildConfig(publishers, subscribers, middleware, sync);
        if (middleware == Middleware::VAsio)
        {
            RunVasioRegistry(domainId);
        }
        if (sync)
        {
            RunSystemMaster(domainId);
        }
    }

    void WaitForAllStarted(std::vector<PublisherParticipant>&                  publishers,
                           std::vector<SubscriberParticipant>& subscribers, bool sync)
    {
        if (sync)
        {
            for (auto& s : subscribers)
            {
                auto futureStatus = s.startedPromise.get_future().wait_for(communicationTimeout);
                EXPECT_EQ(futureStatus, std::future_status::ready)
                    << "Test Failure: Awaiting startup timed out on subscriber";
            }
            for (auto& p : publishers)
            {
                auto futureStatus = p.startedPromise.get_future().wait_for(communicationTimeout);
                EXPECT_EQ(futureStatus, std::future_status::ready)
                    << "Test Failure: Awaiting startup timed out on publisher";
            }
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

protected:
    ib::cfg::Config                              ibConfig;
    std::unique_ptr<ib::extensions::IIbRegistry> registry;
    SystemMaster                                 systemMaster;
    std::vector<std::thread>                     pubSubThreads;


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
    const auto middleware = Middleware::VAsio;
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numMsgToPublish = 3;
    const uint32_t numMsgToReceive = numMsgToPublish;
    const size_t messageSize = 3;
    const bool sync = true;

    std::vector<PublisherParticipant> publishers;
    std::vector<SubscriberParticipant> subscribers;
    publishers.push_back({"Pub1", {{"TopicA", "A", 0, messageSize, numMsgToPublish}}});
    subscribers.push_back({"Sub1", {{"TopicA", "A", messageSize, numMsgToReceive}}});

    SetupSystem(domainId, sync, publishers, subscribers, middleware);

    RunSubscribers(subscribers, domainId, sync);
    RunPublishers(publishers, domainId, sync);

    WaitForAllStarted(publishers, subscribers, sync);

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
    publishers.push_back({"Pub1", {{"TopicA", "A", 0, messageSize, numMsgToPublish}}});
    subscribers.push_back({"Sub1", {{"TopicA", "A", messageSize, numMsgToReceive}}});

    SetupSystem(domainId, sync, publishers, subscribers, middleware);

    RunSubscribers(subscribers, domainId, sync);
    RunPublishers(publishers, domainId, sync);

    WaitForAllStarted(publishers, subscribers, sync);

    StopSimOnAllSentAndReceived(publishers, subscribers, sync);

    JoinPubSubThreads();

    ShutdownSystem();
}

// 100 topics on one publisher/subscriber participant
TEST_F(DataPubSubITest, test_1pub_1sub_100topics_sync_vasio)
{
    const auto     middleware = Middleware::VAsio;
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numMsgToPublish = 1;
    const uint32_t numMsgToReceive = numMsgToPublish;
    const size_t   messageSize = 3;
    const bool     sync = true;
    const int      numTopics = 100;

    std::vector<PublisherParticipant>  publishers;
    std::vector<SubscriberParticipant> subscribers;
    publishers.push_back( { "Pub1" });
    subscribers.push_back({ "Sub1" });
    for (int i = 0; i < numTopics; i++)
    {
        std::string topic = std::to_string(i);
        DataPublisherInfo  pInfo{topic, "A", 1, messageSize, numMsgToPublish};
        DataSubscriberInfo sInfo{topic, "A",    messageSize, numMsgToPublish};
        publishers[0].dataPublishers.push_back(std::move(pInfo));
        subscribers[0].dataSubscribers.push_back(std::move(sInfo));
    }

    SetupSystem(domainId, sync, publishers, subscribers, middleware);

    RunSubscribers(subscribers, domainId, sync);
    RunPublishers(publishers, domainId, sync);

    WaitForAllStarted(publishers, subscribers, sync);

    StopSimOnAllSentAndReceived(publishers, subscribers, sync);

    JoinPubSubThreads();

    ShutdownSystem();
}

// One publisher participant, two subscribers participants on same topic
TEST_F(DataPubSubITest, test_1pub_2sub_sync_vasio)
{
    const auto     middleware = Middleware::VAsio;
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numMsgToPublish = 3;
    const uint32_t numMsgToReceive = numMsgToPublish;
    const size_t   messageSize = 3;
    const bool     sync = true;

    std::vector<PublisherParticipant>  publishers;
    std::vector<SubscriberParticipant> subscribers;
    publishers.push_back({"Pub1", {{"TopicA", "A", 0, messageSize, numMsgToPublish}}});
    subscribers.push_back({"Sub1", {{"TopicA", "A", messageSize, numMsgToReceive}}});
    subscribers.push_back({"Sub2", {{"TopicA", "A", messageSize, numMsgToReceive}}});

    SetupSystem(domainId, sync, publishers, subscribers, middleware);

    RunSubscribers(subscribers, domainId, sync);
    RunPublishers(publishers, domainId, sync);

    WaitForAllStarted(publishers, subscribers, sync);

    StopSimOnAllSentAndReceived(publishers, subscribers, sync);

    JoinPubSubThreads();

    ShutdownSystem();
}

// Two publisher participants, one subscriber participant on same topic: Expect all to arrive but arbitrary reception order
TEST_F(DataPubSubITest, test_2pub_1sub_sync_vasio)
{
    const auto     middleware = Middleware::VAsio;
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numMsgToPublish = 3;
    const uint32_t numMsgToReceive = numMsgToPublish * 2;
    const size_t   messageSize = 3;
    const bool     sync = true;

    std::vector<PublisherParticipant>  publishers;
    std::vector<SubscriberParticipant> subscribers;
    publishers.push_back({"Pub1", {{"TopicA", "A", 0, messageSize, numMsgToPublish}}});
    publishers.push_back({"Pub2", {{"TopicA", "A", 0, messageSize, numMsgToPublish}}});
    std::vector<std::vector<uint8_t>> expectedDataUnordered;
    for (uint32_t d = 0; d < numMsgToPublish; d++)
    {
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(messageSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(messageSize, d));
    }
    subscribers.push_back({"Sub1", {{"TopicA", "A", messageSize, numMsgToReceive, expectedDataUnordered}}});

    SetupSystem(domainId, sync, publishers, subscribers, middleware);

    RunSubscribers(subscribers, domainId, sync);
    RunPublishers(publishers, domainId, sync);

    WaitForAllStarted(publishers, subscribers, sync);

    StopSimOnAllSentAndReceived(publishers, subscribers, sync);

    JoinPubSubThreads();

    ShutdownSystem();
}

// Seven participants, multiple topics
TEST_F(DataPubSubITest, test_3pub_4sub_4topics_sync_vasio)
{
    const auto     middleware = Middleware::VAsio;
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numMsgToPublish = 3;
    const size_t   messageSize = 3;
    const bool     sync = true;

    std::vector<PublisherParticipant>  publishers;
    std::vector<SubscriberParticipant> subscribers;
    publishers.push_back({ "Pub1",  {{"TopicA", "A", 0, messageSize, numMsgToPublish}, {"TopicB", "B", 0, messageSize, numMsgToPublish}} });
    publishers.push_back({ "Pub2",  {{"TopicA", "A", 0, messageSize, numMsgToPublish}, {"TopicC", "C", 0, messageSize, numMsgToPublish}} });
    publishers.push_back({ "Pub3",  {{"TopicA", "A", 0, messageSize, numMsgToPublish}, {"TopicD", "D", 0, messageSize, numMsgToPublish}} });

    std::vector<std::vector<uint8_t>> expectedDataUnordered;
    for (uint32_t d = 0; d < numMsgToPublish; d++)
    {
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(messageSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(messageSize, d));
        expectedDataUnordered.emplace_back(std::vector<uint8_t>(messageSize, d));
    }
    subscribers.push_back({ "Sub1", {{"TopicA", "A", messageSize, numMsgToPublish * 3, expectedDataUnordered }} });
    subscribers.push_back({ "Sub2", {{"TopicB", "B", messageSize, numMsgToPublish}} });
    subscribers.push_back({ "Sub3", {{"TopicC", "C", messageSize, numMsgToPublish}} });
    subscribers.push_back({ "Sub4", {{"TopicD", "D", messageSize, numMsgToPublish}} });

    SetupSystem(domainId, sync, publishers, subscribers, middleware);

    RunSubscribers(subscribers, domainId, sync);
    RunPublishers(publishers, domainId, sync);

    WaitForAllStarted(publishers, subscribers, sync);

    StopSimOnAllSentAndReceived(publishers, subscribers, sync);

    JoinPubSubThreads();

    ShutdownSystem();
}

// Wrong topic -> Expect no reception
TEST_F(DataPubSubITest, test_1pub_1sub_wrong_topic_sync_vasio)
{
    const auto     middleware = Middleware::VAsio;
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numMsgToPublish = 3;
    const uint32_t numMsgToReceive = 0;
    const size_t   messageSize = 3;
    const bool     sync = true;

    std::vector<PublisherParticipant>  publishers;
    std::vector<SubscriberParticipant> subscribers;
    publishers.push_back({"Pub1", {{"TopicA", "A", 0, messageSize, numMsgToPublish}}});
    subscribers.push_back({"Sub1", {{"TopicB", "A", messageSize, numMsgToReceive}}});

    SetupSystem(domainId, sync, publishers, subscribers, middleware);

    RunSubscribers(subscribers, domainId, sync);
    RunPublishers(publishers, domainId, sync);

    WaitForAllStarted(publishers, subscribers, sync);

    StopSimOnAllSentAndReceived(publishers, subscribers, sync);

    JoinPubSubThreads();

    ShutdownSystem();
}

// Wrong dataExchangeFormat -> Expect no reception
TEST_F(DataPubSubITest, test_1pub_1sub_wrong_dxf_sync_vasio)
{
    const auto     middleware = Middleware::VAsio;
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numMsgToPublish = 3;
    const uint32_t numMsgToReceive = 0; 
    const size_t   messageSize = 3;
    const bool     sync = true;

    std::vector<PublisherParticipant>  publishers;
    std::vector<SubscriberParticipant> subscribers;
    publishers.push_back({"Pub1", {{"TopicA", "A", 0, messageSize, numMsgToPublish}}});
    subscribers.push_back({"Sub1", {{"TopicA", "B", messageSize, numMsgToReceive}}});

    SetupSystem(domainId, sync, publishers, subscribers, middleware);

    RunSubscribers(subscribers, domainId, sync);
    RunPublishers(publishers, domainId, sync);

    WaitForAllStarted(publishers, subscribers, sync);

    StopSimOnAllSentAndReceived(publishers, subscribers, sync);

    JoinPubSubThreads();

    ShutdownSystem();
}

// Wildcard dataExchangeFormat on subscriber
TEST_F(DataPubSubITest, test_1pub_1sub_wildcard_dxf_sync_vasio)
{
    const auto     middleware = Middleware::VAsio;
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numMsgToPublish = 3;
    const uint32_t numMsgToReceive = numMsgToPublish;
    const size_t   messageSize = 3;
    const bool     sync = true;

    std::vector<PublisherParticipant>  publishers;
    std::vector<SubscriberParticipant> subscribers;
    publishers.push_back({"Pub1", {{"TopicA", "A", 0, messageSize, numMsgToPublish}}});
    subscribers.push_back({"Sub1", {{"TopicA", "*", messageSize, numMsgToReceive}}});

    SetupSystem(domainId, sync, publishers, subscribers, middleware);

    RunSubscribers(subscribers, domainId, sync);
    RunPublishers(publishers, domainId, sync);

    WaitForAllStarted(publishers, subscribers, sync);

    StopSimOnAllSentAndReceived(publishers, subscribers, sync);

    JoinPubSubThreads();

    ShutdownSystem();
}

//-----------------------------------------------------
// Async tests: No participantController/SimulationTask
//-----------------------------------------------------

// Async: Start subscribers first, publish with delay to ensure reception
TEST_F(DataPubSubITest, test_1pub_1sub_async_vasio)
{
    const auto     middleware = Middleware::VAsio;
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numMsgToPublish = 3;
    const uint32_t numMsgToReceive = numMsgToPublish;
    const size_t   messageSize = 3;
    const bool sync = false;

    std::vector<PublisherParticipant>  publishers;
    std::vector<SubscriberParticipant> subscribers;
    publishers.push_back ({ "Pub1", { { "TopicA", "A", 0, messageSize, numMsgToPublish } } });
    subscribers.push_back({ "Sub1", { { "TopicA", "A",    messageSize, numMsgToReceive } } });

    SetupSystem(domainId, sync, publishers, subscribers, middleware);

    // Async without history: Start subscribers first
    RunSubscribers(subscribers, domainId, sync);
    for (auto& s : subscribers)
    {
        auto futureStatus = s.startedPromise.get_future().wait_for(communicationTimeout);
        EXPECT_EQ(futureStatus, std::future_status::ready)
            << "Test Failure: Awaiting reception timed out on subscriber";
    }
    RunPublishers(publishers, domainId, sync);

    JoinPubSubThreads();

    ShutdownSystem();
}


// Async with history: Wait for publication before starting the subscriber
TEST_F(DataPubSubITest, test_1pub_1sub_async_history_vasio)
{
    
    const auto     middleware = Middleware::VAsio;
    const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

    const uint32_t numMsgToPublish = 1;
    const uint32_t numMsgToReceive = numMsgToPublish;
    const size_t   messageSize = 3;
    const bool     sync = false;

    std::vector<PublisherParticipant>  publishers;
    std::vector<SubscriberParticipant> subscribers;
    publishers.push_back ({ "Pub1", { { "TopicA", "A", 1, messageSize, numMsgToPublish } } });
    subscribers.push_back({ "Sub1", { { "TopicA", "A",    messageSize, numMsgToReceive } } });

    SetupSystem(domainId, sync, publishers, subscribers, middleware);

    RunPublishers(publishers, domainId, sync);
    for (auto& s : publishers)
    {
        auto futureStatus = s.startedPromise.get_future().wait_for(communicationTimeout);
        EXPECT_EQ(futureStatus, std::future_status::ready)
            << "Test Failure: Awaiting startup timed out on publisher";
        futureStatus = s.allSentPromise.get_future().wait_for(communicationTimeout);
        EXPECT_EQ(futureStatus, std::future_status::ready)
            << "Test Failure: Awaiting transmission timed out on publisher";
    }
    // Async with history:
    RunSubscribers(subscribers, domainId, sync);

    JoinPubSubThreads();

    ShutdownSystem();
}
#endif

} // anonymous namespace
