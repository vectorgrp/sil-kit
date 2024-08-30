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

#pragma once

#include "silkit/SilKit.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/all.hpp"

#include "ConfigurationTestUtils.hpp"
#include "IParticipantInternal.hpp"
#include "IServiceDiscovery.hpp"
#include "ServiceDatatypes.hpp"

#include "IntegrationTestInfrastructure.hpp"

using namespace std::chrono_literals;
using namespace SilKit;
using namespace SilKit::Services::Orchestration;
using namespace SilKit::Config;
using namespace SilKit::Services::PubSub;

const size_t defaultMsgSize = 3;
const uint32_t defaultNumMsgToPublish = 3;

class ITest_Internals_DataPubSub : public testing::Test
{
protected:
    ITest_Internals_DataPubSub() {}

    struct DataPublisherInfo
    {
        DataPublisherInfo(const std::string& newControllerName, const std::string& newTopic,
                          const std::string& newMediaType,
                          const std::vector<SilKit::Services::MatchingLabel>& newLabels, uint8_t newHistory,
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
        std::vector<SilKit::Services::MatchingLabel> labels;
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

    struct DataSubscriberInfo
    {
        DataSubscriberInfo(const std::string& newControllerName, const std::string& newTopic,
                           const std::string& newMediaType,
                           const std::vector<SilKit::Services::MatchingLabel>& newLabels, size_t newMessageSizeInBytes,
                           uint32_t newNumMsgToReceive, uint32_t newExpectedSources)
        {
            expectIncreasingData = true;
            controllerName = newControllerName;
            topic = newTopic;
            mediaType = newMediaType;
            labels = newLabels;
            messageSizeInBytes = newMessageSizeInBytes;
            numMsgToReceive = newNumMsgToReceive;
            expectedSources = newExpectedSources;
        }
        DataSubscriberInfo(const std::string& newControllerName, const std::string& newTopic,
                           const std::string& newMediaType,
                           const std::vector<SilKit::Services::MatchingLabel>& newLabels, size_t newMessageSizeInBytes,
                           uint32_t newNumMsgToReceive, uint32_t newExpectedSources,
                           const std::vector<std::vector<uint8_t>>& newExpectedDataUnordered)
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
        }

        std::string controllerName;
        std::string topic;
        std::string mediaType;
        std::vector<SilKit::Services::MatchingLabel> labels;
        size_t messageSizeInBytes;
        uint32_t numMsgToReceive;
        bool expectIncreasingData;
        std::vector<std::vector<uint8_t>> expectedDataUnordered;

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
                    EXPECT_TRUE(SilKit::Util::ItemsAreEqual(dataMessageEvent.data, SilKit::Util::ToSpan(expectedData)));
                }
                else
                {
                    auto foundDataIter = std::find_if(expectedDataUnordered.begin(), expectedDataUnordered.end(),
                                                      [&dataMessageEvent](const auto& expectedData) -> bool {
                        return SilKit::Util::ItemsAreEqual(SilKit::Util::ToSpan(expectedData), dataMessageEvent.data);
                    });
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

        void OnNewServiceDiscovery(const SilKit::Core::ServiceDescriptor /*sd*/)
        {
            newSourceCounter++;
            if (!allDiscovered)
            {
                if (newSourceCounter >= expectedSources)
                {
                    allDiscovered = true;
                }
            }
        }
    };

    struct PubSubParticipant
    {
        PubSubParticipant(const std::string& newName)
        {
            name = newName;
        }
        PubSubParticipant(const std::string& newName, const std::vector<DataPublisherInfo>& newDataPublishers,
                          const std::vector<DataSubscriberInfo>& newDataSubscribers,
                          std::shared_ptr<SilKit::Config::IParticipantConfiguration> newConfig =
                              SilKit::Config::MakeEmptyParticipantConfigurationImpl())
        {
            config = newConfig;
            name = newName;
            dataSubscribers = newDataSubscribers;
            dataPublishers = newDataPublishers;
        }

        std::shared_ptr<SilKit::Config::IParticipantConfiguration> config = MakeEmptyParticipantConfigurationImpl();
        bool delayedDefaultDataHandler = false;
        std::string name;
        std::vector<DataSubscriberInfo> dataSubscribers;
        std::vector<DataPublisherInfo> dataPublishers;
        std::unique_ptr<SilKit::IParticipant> participant;
        SilKit::Core::IParticipantInternal* participantImpl;

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
            if (std::all_of(dataSubscribers.begin(), dataSubscribers.end(),
                            [](const auto& dsInfo) { return dsInfo.numMsgToReceive == 0; }))
            {
                allReceived = true;
                allReceivedPromise.set_value();
            }
        }
        void PrepareAllDiscoveredPromise()
        {
            if (std::all_of(dataSubscribers.begin(), dataSubscribers.end(),
                            [](const auto& dsInfo) { return dsInfo.expectedSources == 0; }))
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
            if (!allSent
                && std::all_of(dataPublishers.begin(), dataPublishers.end(), [](const auto& dp) { return dp.allSent; }))
            {
                allSent = true;
                allSentPromise.set_value();
            }
        }

        void WaitForCreateParticipant()
        {
            auto futureStatus = participantCreatedPromise.get_future().wait_for(communicationTimeout);
            EXPECT_EQ(futureStatus, std::future_status::ready)
                << "Test Failure: Awaiting participant creation timed out";
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
    void ParticipantThread(PubSubParticipant& participant, const std::string& registryUri, bool sync)
    {
        try
        {
            participant.participant = SilKit::CreateParticipantImpl(participant.config, participant.name, registryUri);
            participant.participantImpl =
                dynamic_cast<SilKit::Core::IParticipantInternal*>(participant.participant.get());

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

                participant.participantImpl->GetServiceDiscovery()->RegisterServiceDiscoveryHandler(
                    [&ds, &participant](auto type, auto&& serviceDescr) {
                    if (type == SilKit::Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated)
                    {
                        if (serviceDescr.GetNetworkType() == SilKit::Config::NetworkType::Data)
                        {
                            ds.OnNewServiceDiscovery(serviceDescr);
                            participant.CheckAllDiscoveredPromise();
                        }
                    }
                });

                SilKit::Services::PubSub::PubSubSpec dataSpec{ds.topic, ds.mediaType};
                for (auto label : ds.labels)
                {
                    dataSpec.AddLabel(label);
                }

                // Create DataSubscriber with default handler
                if (participant.delayedDefaultDataHandler)
                {
                    ds.dataSubscriber =
                        participant.participant->CreateDataSubscriber(ds.controllerName, dataSpec, nullptr);
                    ds.dataSubscriber->SetDataMessageHandler(receptionHandler);
                }
                else
                {
                    ds.dataSubscriber =
                        participant.participant->CreateDataSubscriber(ds.controllerName, dataSpec, receptionHandler);
                }
            }

            // Setup/Create Publishers
            for (auto& dp : participant.dataPublishers)
            {
                SilKit::Services::PubSub::PubSubSpec dataSpec{dp.topic, dp.mediaType};
                for (auto label : dp.labels)
                {
                    dataSpec.AddLabel(label);
                }
                dp.dataPublisher =
                    participant.participant->CreateDataPublisher(dp.controllerName, dataSpec, dp.history);
            }
            auto publishTask = [&participant]() {
                for (auto& dp : participant.dataPublishers)
                {
                    dp.Publish();
                }
            };

            if (sync)
            {
                auto* lifecycleService = participant.participant->CreateLifecycleService(
                    {SilKit::Services::Orchestration::OperationMode::Coordinated});
                auto* timeSyncService = lifecycleService->CreateTimeSyncService();

                timeSyncService->SetSimulationStepHandler(
                    [&participant, publishTask](std::chrono::nanoseconds /*now*/,
                                                std::chrono::nanoseconds /*duration*/) {
                    if (!participant.dataPublishers.empty())
                    {
                        publishTask();
                        participant.CheckAllSentPromise();
                    }
                },
                    1s);
                auto finalStateFuture = lifecycleService->StartLifecycle();
                finalStateFuture.get();
            }
            else
            {
                if (!participant.dataPublishers.empty())
                {
                    while (std::none_of(participant.dataPublishers.begin(), participant.dataPublishers.end(),
                                        [](const DataPublisherInfo& dp) { return dp.allSent; }))
                    {
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

    void RunParticipants(std::vector<PubSubParticipant>& pubsubs, const std::string& registryUri, bool sync)
    {
        try
        {
            for (auto& pubsubParticipant : pubsubs)
            {
                _pubSubThreads.emplace_back([this, &pubsubParticipant, registryUri, sync] {
                    ParticipantThread(pubsubParticipant, registryUri, sync);
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

        std::vector<std::string> requiredParticipantNames;
        for (const auto& p : pubsubs)
        {
            requiredParticipantNames.push_back(p.name);
        }

        _testSystem.SetupRegistryAndSystemMaster("silkit://localhost:0", true, std::move(requiredParticipantNames));
        RunParticipants(pubsubs, _testSystem.GetRegistryUri(), true);
        WaitForAllDiscovered(pubsubs);
        StopSimOnAllSentAndReceived(pubsubs, true);
        JoinPubSubThreads();
        ShutdownSystem();
    }

    void RunAsyncTest(std::vector<PubSubParticipant>& publishers, std::vector<PubSubParticipant>& subscribers)
    {
        _testSystem.SetupRegistryAndSystemMaster("silkit://localhost:0", false, {});

        //Start publishers first
        RunParticipants(publishers, _testSystem.GetRegistryUri(), false);
        for (auto& p : publishers)
        {
            p.WaitForAllSent();
        }
        //Start subscribers afterwards
        RunParticipants(subscribers, _testSystem.GetRegistryUri(), false);
        JoinPubSubThreads();
        ShutdownSystem();
    }

protected:
    std::vector<std::thread> _pubSubThreads;
    TestInfrastructure _testSystem;
};
