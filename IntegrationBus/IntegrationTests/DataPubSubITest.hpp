// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "ib/IntegrationBus.hpp"
#include "ib/mw/sync/all.hpp"
#include "ib/sim/all.hpp"

#include "MockParticipantConfiguration.hpp"

#include "GetTestPid.hpp"
#include "IntegrationTestInfrastructure.hpp"

using namespace std::chrono_literals;
using namespace ib::mw;
using namespace ib::mw::sync;
using namespace ib::cfg;
using namespace ib::sim::data;

const size_t defaultMsgSize = 3;
const uint32_t defaultNumMsgToPublish = 3;

class DataPubSubITest : public testing::Test
{
protected:
    DataPubSubITest() {}

    struct DataPublisherInfo
    {
        DataPublisherInfo(const std::string& newControllerName, const std::string& newTopic, const std::string& newMediaType,
                          const std::map<std::string, std::string>& newLabels, uint8_t newHistory,
                          size_t newMessageSizeInBytes, uint32_t newNumMsgToPublish)
        {
            controllerName = newControllerName;
            topic = newTopic;
            mediaType = newMediaType;
            labels = newLabels;
            history = newHistory;
            messageSizeInBytes = newMessageSizeInBytes;
            numMsgToPublish = newNumMsgToPublish;
        }

        std::string controllerName;
        std::string topic;
        std::string mediaType;
        std::map<std::string, std::string> labels;
        uint8_t history;
        size_t messageSizeInBytes;
        uint32_t numMsgToPublish;
        uint32_t publishMsgCounter{0};
        bool allSent{false};
        IDataPublisher* dataPublisher;

        void Publish()
        {
            if (!allSent)
            {
                auto data = std::vector<uint8_t>(messageSizeInBytes, static_cast<uint8_t>(publishMsgCounter));
                dataPublisher->Publish(data);
                publishMsgCounter++;
                if (publishMsgCounter >= numMsgToPublish)
                {
                    allSent = true;
                }
            }
        }
    };

    struct SpecificDataHandlerInfo
    {
        std::string mediaType;
        std::map<std::string, std::string> labels;
    };

    struct DataSubscriberInfo
    {
        DataSubscriberInfo(const std::string& newControllerName, const std::string& newTopic, const std::string& newMediaType,
                           const std::map<std::string, std::string>& newLabels, size_t newMessageSizeInBytes,
                           uint32_t newNumMsgToReceive, uint32_t newExpectedSources,
                           const std::map<std::string, std::string>& newExpectedLabels,
                           const std::vector<SpecificDataHandlerInfo>& newSpecificDataHandlers)
        {
            expectIncreasingData = true;
            controllerName = newControllerName;
            topic = newTopic;
            mediaType = newMediaType;
            labels = newLabels;
            messageSizeInBytes = newMessageSizeInBytes;
            numMsgToReceive = newNumMsgToReceive;
            expectedSources = newExpectedSources;
            expectedLabels = newExpectedLabels;
            specificDataHandlers = newSpecificDataHandlers;
        }
        DataSubscriberInfo(const std::string& newControllerName, const std::string& newTopic, const std::string& newMediaType,
                           const std::map<std::string, std::string>& newLabels, size_t newMessageSizeInBytes,
                           uint32_t newNumMsgToReceive, uint32_t newExpectedSources,
                           const std::vector<std::vector<uint8_t>>& newExpectedDataUnordered,
                           const std::map<std::string, std::string>& newExpectedLabels,
                           const std::vector<SpecificDataHandlerInfo>& newSpecificDataHandlers)
        {
            expectIncreasingData = false;
            controllerName = newControllerName;
            topic = newTopic;
            mediaType = newMediaType;
            labels = newLabels;
            messageSizeInBytes = newMessageSizeInBytes;
            numMsgToReceive = newNumMsgToReceive;
            expectedSources = newExpectedSources;
            expectedDataUnordered = newExpectedDataUnordered;
            expectedLabels = newExpectedLabels;
            specificDataHandlers = newSpecificDataHandlers;
        }

        std::string controllerName;
        std::string topic;
        std::string mediaType;
        std::map<std::string, std::string> labels;
        std::map<std::string, std::string> expectedLabels;
        size_t messageSizeInBytes;
        uint32_t numMsgToReceive;
        bool expectIncreasingData;
        std::vector<std::vector<uint8_t>> expectedDataUnordered;
        std::vector<SpecificDataHandlerInfo> specificDataHandlers;

        uint32_t expectedSources;
        uint32_t newSourceCounter{0};
        uint32_t receiveMsgCounter{0};
        bool allReceived{false};
        bool allDiscovered{false};
        IDataSubscriber* dataSubscriber;

        void OnDataReception(const DataMessageEvent& dataMessageEvent)
        {
            if (!allReceived)
            {
                if (expectIncreasingData)
                {
                    auto expectedData =
                        std::vector<uint8_t>(messageSizeInBytes, static_cast<uint8_t>(receiveMsgCounter));
                    EXPECT_EQ(dataMessageEvent.data, expectedData);
                }
                else
                {
                    auto foundDataIter = std::find(expectedDataUnordered.begin(), expectedDataUnordered.end(), dataMessageEvent.data);
                    EXPECT_EQ(foundDataIter != expectedDataUnordered.end(), true);
                    if (foundDataIter != expectedDataUnordered.end())
                    {
                        expectedDataUnordered.erase(foundDataIter);
                    }
                }

                receiveMsgCounter++;
                if (receiveMsgCounter >= numMsgToReceive)
                {
                    allReceived = true;
                }
            }
        }

        void OnNewDataSource(IDataSubscriber* subscriber, const NewDataPublisherEvent& dataSource,
                             ib::sim::data::DataMessageHandlerT receptionHandler)
        {
            newSourceCounter++;
            if (!allDiscovered)
            {
                if (newSourceCounter >= expectedSources)
                {
                    allDiscovered = true;
                }
            }

            for (auto const& kv : dataSource.labels)
            {
                EXPECT_EQ(expectedLabels[kv.first], kv.second);
            }

            for (auto const& sp : specificDataHandlers)
            {
                if (sp.labels == dataSource.labels)
                {
                    subscriber->AddExplicitDataMessageHandler(receptionHandler, sp.mediaType, sp.labels);
                }
            }
        }
    };

    struct PubSubParticipant
    {
        PubSubParticipant(const std::string& newName) { name = newName; }
        PubSubParticipant(const std::string& newName, const std::vector<DataPublisherInfo>& newDataPublishers,
            const std::vector<DataSubscriberInfo>& newDataSubscribers,
            std::shared_ptr<ib::cfg::IParticipantConfiguration> newConfig = ib::cfg::MockParticipantConfiguration())
        {
            config = newConfig;
            name = newName;
            dataSubscribers = newDataSubscribers;
            dataPublishers = newDataPublishers;
        }

        std::shared_ptr<ib::cfg::IParticipantConfiguration> config;
        bool delayedDefaultDataHandler = false;
        std::string name;
        std::vector<DataSubscriberInfo> dataSubscribers;
        std::vector<DataPublisherInfo> dataPublishers;
        std::unique_ptr<IParticipant> participant;

        // Common
        std::promise<void> participantCreatedPromise;

        // Sub
        std::promise<void> allDiscoveredPromise;
        bool allDiscovered{false};
        std::promise<void> allReceivedPromise;
        bool allReceived{false};
        // Pub
        std::promise<void> allSentPromise;
        bool allSent{false};

        std::chrono::milliseconds communicationTimeout{20000ms};

        void PrepareAllReceivedPromise()
        {
            if (std::all_of(dataSubscribers.begin(), dataSubscribers.end(), [](const auto& dsInfo) {
                    return dsInfo.numMsgToReceive == 0;
                }))
            {
                allReceived = true;
                allReceivedPromise.set_value();
            }
        }
        void PrepareAllDiscoveredPromise()
        {
            if (std::all_of(dataSubscribers.begin(), dataSubscribers.end(), [](const auto& dsInfo) {
                    return dsInfo.expectedSources == 0;
                }))
            {
                allDiscovered = true;
                allDiscoveredPromise.set_value();
            }
        }

        void CheckAllReceivedPromise()
        {
            if (!allReceived && std::all_of(dataSubscribers.begin(), dataSubscribers.end(), [](const auto& dsInfo) {
                    return dsInfo.allReceived;
                }))
            {
                allReceived = true;
                allReceivedPromise.set_value();
            }
        }
        void CheckAllDiscoveredPromise()
        {
            if (!allDiscovered && std::all_of(dataSubscribers.begin(), dataSubscribers.end(), [](const auto& dsInfo) {
                    return dsInfo.allDiscovered;
                }))
            {
                allDiscovered = true;
                allDiscoveredPromise.set_value();
            }
        }
        void CheckAllSentPromise()
        {
            if (!allSent && std::all_of(dataPublishers.begin(), dataPublishers.end(), [](DataPublisherInfo dp) {
                    return dp.allSent;
                }))
            {
                allSent = true;
                allSentPromise.set_value();
            }
        }

        void WaitForCreateParticipant()
        {
            auto futureStatus = participantCreatedPromise.get_future().wait_for(communicationTimeout);
            EXPECT_EQ(futureStatus, std::future_status::ready) << "Test Failure: Awaiting participant creation timed out";
        }
        void WaitForAllSent()
        {
            auto futureStatus = allSentPromise.get_future().wait_for(communicationTimeout);
            EXPECT_EQ(futureStatus, std::future_status::ready) << "Test Failure: Awaiting transmission timed out";
        }
        void WaitForAllReceived()
        {
            auto futureStatus = allReceivedPromise.get_future().wait_for(communicationTimeout);
            EXPECT_EQ(futureStatus, std::future_status::ready) << "Test Failure: Awaiting reception timed out";
        }
        void WaitForAllDiscovered()
        {
            auto futureStatus = allDiscoveredPromise.get_future().wait_for(communicationTimeout);
            EXPECT_EQ(futureStatus, std::future_status::ready) << "Test Failure: Awaiting source discovery timed out";
        }
    };

    // Specific for this test
    void ParticipantThread(PubSubParticipant& participant, uint32_t domainId, bool sync)
    {
        try
        {
            participant.participant = ib::CreateParticipant(participant.config, participant.name, domainId, sync);

            participant.participantCreatedPromise.set_value();

            // Already set the promise if no reception/discovery is expected
            participant.PrepareAllReceivedPromise();
            participant.PrepareAllDiscoveredPromise();

            // Setup/Create Subscribers
            for (auto& ds : participant.dataSubscribers)
            {
                auto receptionHandler = [&participant, &ds](IDataSubscriber* /*subscriber*/,
                                                            const DataMessageEvent& dataMessageEvent) {
                    ds.OnDataReception(dataMessageEvent);
                    participant.CheckAllReceivedPromise();
                };

                auto newDataSourceHandler = [&participant, &ds, receptionHandler](
                                                IDataSubscriber* subscriber, const NewDataPublisherEvent& dataSource) {
                    ds.OnNewDataSource(subscriber, dataSource, receptionHandler);
                    participant.CheckAllDiscoveredPromise();
                };

                if (ds.specificDataHandlers.empty())
                {
                    // Create DataSubscriber with default handler
                    if(participant.delayedDefaultDataHandler)
                    {
                        ds.dataSubscriber = participant.participant->CreateDataSubscriber(
                            ds.controllerName, ds.topic, ds.mediaType, ds.labels, nullptr,
                            newDataSourceHandler);
                        ds.dataSubscriber->SetDefaultDataMessageHandler(receptionHandler);
                    }
                    else
                    {
                        ds.dataSubscriber = participant.participant->CreateDataSubscriber(
                            ds.controllerName, ds.topic, ds.mediaType, ds.labels, receptionHandler,
                            newDataSourceHandler);
                    }
                }
                else
                {
                    // Create DataSubscriber without default handler
                    ds.dataSubscriber = participant.participant->CreateDataSubscriber(ds.controllerName, ds.topic, ds.mediaType, ds.labels,
                                                                                     nullptr, newDataSourceHandler);
                }
            }

            // Setup/Create Publishers
            for (auto& dp : participant.dataPublishers)
            {
                dp.dataPublisher = participant.participant->CreateDataPublisher(dp.controllerName, dp.topic, dp.mediaType, dp.labels, dp.history);
            }
            auto publishTask = [&participant]() {
                for (auto& dp : participant.dataPublishers)
                {
                    dp.Publish();
                }
            };

            if (sync)
            {
                IParticipantController* participantController = participant.participant->GetParticipantController();
                participantController->SetPeriod(1s);
                participantController->SetSimulationTask([&participant, publishTask](std::chrono::nanoseconds /*now*/) {
                    if (!participant.dataPublishers.empty())
                    {
                        publishTask();
                        participant.CheckAllSentPromise();
                    }
                });
                auto finalStateFuture = participantController->RunAsync();
                finalStateFuture.get();
            }
            else
            {
                if (!participant.dataPublishers.empty())
                {
                    while (std::none_of(participant.dataPublishers.begin(), participant.dataPublishers.end(),
                                        [](const DataPublisherInfo& dp) {
                                            return dp.allSent;
                                        }))
                    {
                        //std::this_thread::sleep_for(10ms);
                        publishTask();
                    }
                    participant.allSentPromise.set_value();
                }

                if (!participant.dataSubscribers.empty())
                {
                    participant.WaitForAllReceived();
                }
            }

            for (const auto& ds : participant.dataSubscribers)
            {
                EXPECT_EQ(ds.receiveMsgCounter, ds.numMsgToReceive);
            }
        }
        catch (const std::exception& error)
        {
            _testSystem.ShutdownOnException(error);
        }
    }

    void RunParticipants(std::vector<PubSubParticipant>& pubsubs, uint32_t domainId, bool sync)
    {
        try
        {
            for (auto& pubsubParticipant : pubsubs)
            {
                _pubSubThreads.emplace_back([this, &pubsubParticipant, domainId, sync] {
                    ParticipantThread(pubsubParticipant, domainId, sync);
                });
            }
        }
        catch (const std::exception& error)
        {
            _testSystem.ShutdownOnException(error);
        }
    }

    void JoinPubSubThreads()
    {
        for (auto&& thread : _pubSubThreads)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }
    }

    void WaitForAllDiscovered(std::vector<PubSubParticipant>& pubsubs)
    {
        for (auto& ps : pubsubs)
        {
            if (!ps.dataSubscribers.empty())
            {
                ps.WaitForAllDiscovered();
            }
        }
    }

    void StopSimOnAllSentAndReceived(std::vector<PubSubParticipant>& pubsubs, bool sync)
    {
        if (sync)
        {
            for (auto& ps : pubsubs)
            {
                if (!ps.dataPublishers.empty())
                {
                    ps.WaitForAllSent();
                }
            }

            for (auto& ps : pubsubs)
            {
                if (!ps.dataSubscribers.empty())
                {
                    ps.WaitForAllReceived();
                }
            }
            _testSystem.SystemMasterStop();
        }
    }

    void ShutdownSystem()
    {
        _pubSubThreads.clear();
        _testSystem.ShutdownInfrastructure();
    }

    void RunSyncTest(std::vector<PubSubParticipant>& pubsubs)
    {
        const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

        std::vector<std::string> requiredParticipantNames;
        for (const auto& p : pubsubs)
        {
            requiredParticipantNames.push_back(p.name);
        }

        _testSystem.SetupRegistryAndSystemMaster(domainId, true, requiredParticipantNames);
        RunParticipants(pubsubs, domainId, true);
        WaitForAllDiscovered(pubsubs);
        StopSimOnAllSentAndReceived(pubsubs, true);
        JoinPubSubThreads();
        ShutdownSystem();
    }

    void RunAsyncTest(std::vector<PubSubParticipant>& publishers, std::vector<PubSubParticipant>& subscribers)
    {
        const uint32_t domainId = static_cast<uint32_t>(GetTestPid());

        _testSystem.SetupRegistryAndSystemMaster(domainId, false, {});

        //Start publishers first
        RunParticipants(publishers, domainId, false);
        for (auto& p : publishers)
        {
            p.WaitForAllSent();
        }
        //Start subscribers afterwards
        RunParticipants(subscribers, domainId, false);
        JoinPubSubThreads();
        ShutdownSystem();
    }

protected:
    std::vector<std::thread> _pubSubThreads;
    TestInfrastructure _testSystem;
};
