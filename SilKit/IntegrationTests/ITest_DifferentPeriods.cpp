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

#include <chrono>
#include <thread>
#include <future>
#include <string>
#include <sstream>
#include <numeric>

#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/pubsub/PubSubSpec.hpp"
#include "silkit/services/all.hpp"
#include "silkit/participant/exception.hpp"
#include "silkit/SilKit.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using namespace std::chrono;
using namespace SilKit::Config;
using namespace SilKit::Services::Orchestration;
using namespace SilKit::Services::PubSub;

const std::string testMessage{"TestMessage"};
const std::chrono::nanoseconds subscriberPeriod = 7ns;
const std::vector<std::chrono::nanoseconds> publisherPeriods = {3ns, 7ns, 17ns};

std::ostream& operator<<(std::ostream& out, const nanoseconds& timestamp)
{
    out << timestamp.count();
    return out;
}

std::istream& operator>>(std::istream& in, nanoseconds& timestamp)
{
    nanoseconds::rep timeRep;
    in >> timeRep;
    timestamp = nanoseconds{timeRep};
    return in;
}

class Publisher
{
public:
    Publisher(const std::string& registryUri, const uint32_t publisherIndex, const uint32_t numMessages,
              std::chrono::nanoseconds period)
        : _numMessages{numMessages}
    {
        _participantName = "Publisher" + std::to_string(publisherIndex);
        _participant = SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(""),
                                                 _participantName, registryUri);

        const auto topicName = "Topic" + std::to_string(publisherIndex);
        _lifecycleService =
            _participant->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated});
        auto* timeSyncService = _lifecycleService->CreateTimeSyncService();
        SilKit::Services::PubSub::PubSubSpec dataSpec{topicName, {}};
        auto* publisher = _participant->CreateDataPublisher("PubCtrl1", dataSpec, 0);

        timeSyncService->SetSimulationStepHandler(
            [this, publisher, period](const nanoseconds now, nanoseconds /*duration*/) {
            ASSERT_TRUE((now.count() % period.count()) == 0);
            if (_messageIndex < _numMessages)
            {
                PublishMessage(publisher, now, _messageIndex++);
            }
        }, period);
    }

    void RunAsync()
    {
        _simulationFuture = _lifecycleService->StartLifecycle();
    }

    auto WaitForShutdown() -> ParticipantState
    {
        return _simulationFuture.get();
    }

    uint32_t NumMessagesSent() const
    {
        return _messageIndex;
    }

private:
    void PublishMessage(IDataPublisher* publisher, const nanoseconds now, const uint32_t index) const
    {
        // Send testMessage with current tick (now) and current index
        std::stringstream stream;
        stream << testMessage << ' ' << now << ' ' << index;
        auto message = stream.str();
        std::vector<uint8_t> testData{message.begin(), message.end()};

        publisher->Publish(std::move(testData));
    }

private:
    std::unique_ptr<SilKit::IParticipant> _participant{nullptr};

    uint32_t _messageIndex{0u};
    uint32_t _numMessages{0u};
    std::future<ParticipantState> _simulationFuture;
    std::string _participantName;
    SilKit::Services::Orchestration::ILifecycleService* _lifecycleService{nullptr};
};

class Subscriber
{
public:
    Subscriber(const std::vector<std::string>& syncParticipantNames, const std::string& participantName,
               const std::string& registryUri, const uint32_t& publisherCount, const uint32_t numMessages)
        : _publisherCount{publisherCount}
        , _messageIndexes(publisherCount, 0u)
        , _numMessages{numMessages}
        , _syncParticipantNames{syncParticipantNames}
        , _participantName{participantName}
    {
        _participant = SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(""),
                                                 participantName, registryUri);

        _systemController = SilKit::Experimental::Participant::CreateSystemController(_participant.get());
        _systemController->SetWorkflowConfiguration({_syncParticipantNames});

        _monitor = _participant->CreateSystemMonitor();

        _lifecycleService =
            _participant->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated});

        auto* timeSyncService = _lifecycleService->CreateTimeSyncService();

        for (auto publisherIndex = 0u; publisherIndex < _publisherCount; publisherIndex++)
        {
            SilKit::Services::PubSub::PubSubSpec dataSpec{"Topic" + std::to_string(publisherIndex), {}};
            _participant->CreateDataSubscriber(
                "SubCtrl" + std::to_string(publisherIndex), dataSpec,
                [this, publisherIndex](IDataSubscriber* subscriber, const DataMessageEvent& dataMessageEvent) {
                ReceiveMessage(subscriber, dataMessageEvent, publisherIndex);
            });
        }
        timeSyncService->SetSimulationStepHandler([this](const nanoseconds now, nanoseconds /*duration*/) {
            _currentTime = now;
            ASSERT_TRUE((_currentTime.count() % subscriberPeriod.count()) == 0);
        }, subscriberPeriod);
    }

    std::future<ParticipantState> RunAsync() const
    {
        return _lifecycleService->StartLifecycle();
    }

    uint32_t NumMessagesReceived(const uint32_t publisherIndex)
    {
        return _messageIndexes[publisherIndex];
    }

private:
    void ReceiveMessage(IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent,
                        const uint32_t publisherIndex)
    {
        auto& messageIndex = _messageIndexes[publisherIndex];
        const std::string message{dataMessageEvent.data.begin(), dataMessageEvent.data.end()};

        std::string receivedMessage;
        nanoseconds sentTime = {};
        uint32_t receivedIndex;
        std::stringstream stream{message};

        try
        {
            stream >> receivedMessage >> sentTime >> receivedIndex;
        }
        catch (SilKit::SilKitError& /*error*/)
        {
            std::cout << "ERROR: Received message does not match the expected format" << std::endl;
            _lifecycleService->Stop("Test");
            return;
        }

        // The constant testMessage should have been sent and received correctly
        EXPECT_EQ(testMessage, receivedMessage);

        ASSERT_TRUE(sentTime >= _currentTime - subscriberPeriod);
        ASSERT_TRUE(sentTime <= _currentTime + subscriberPeriod);

        // Publisher sentTime must be in steps of publisher period
        if (receivedIndex > 0)
        {
            ASSERT_TRUE(sentTime / receivedIndex == publisherPeriods[publisherIndex]);
        }

        messageIndex++;

        const auto sumOfIndexes = std::accumulate(_messageIndexes.begin(), _messageIndexes.end(), 0u);
        if (sumOfIndexes == _numMessages * _publisherCount)
        {
            _lifecycleService->Stop("Test");
        }
    }

private:
    uint32_t _publisherCount{0u};
    std::vector<uint32_t> _messageIndexes;
    uint32_t _numMessages{0u};
    std::vector<std::string> _syncParticipantNames;
    std::unique_ptr<SilKit::IParticipant> _participant{nullptr};
    SilKit::Experimental::Services::Orchestration::ISystemController* _systemController{nullptr};
    SilKit::Services::Orchestration::ILifecycleService* _lifecycleService{nullptr};
    ISystemMonitor* _monitor{nullptr};

    std::chrono::nanoseconds _currentTime{0ns};
    std::string _participantName;
};

class ITest_DifferentPeriods : public testing::Test
{
};


// Tests for deterministic reception of messages using participants
// with different periods
TEST_F(ITest_DifferentPeriods, different_simtask_periods)
{
    const uint32_t publisherCount = 3;
    const uint32_t numMessages = 350;

    assert(publisherPeriods.size() >= publisherCount);

    std::string subscriberName = "Subscriber";

    std::vector<std::string> syncParticipantNames;
    syncParticipantNames.push_back(subscriberName);
    for (auto i = 0u; i < publisherCount; i++)
    {
        syncParticipantNames.push_back("Publisher" + std::to_string(i));
    }

    auto registry =
        SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::ParticipantConfigurationFromString(""));
    auto registryUri = registry->StartListening("silkit://localhost:0");

    // The subscriber assumes the role of the system controller and initiates simulation state changes
    Subscriber subscriber(syncParticipantNames, subscriberName, registryUri, publisherCount, numMessages);
    auto subscriberFuture = subscriber.RunAsync();

    std::vector<Publisher> publishers;
    publishers.reserve(publisherCount);
    for (auto i = 0u; i < publisherCount; i++)
    {
        publishers.emplace_back(registryUri, i, numMessages, publisherPeriods[i]);
        publishers[i].RunAsync();
    }


    auto finalState = subscriberFuture.get();
    EXPECT_EQ(ParticipantState::Shutdown, finalState);

    for (auto publisherIndex = 0u; publisherIndex < publisherCount; publisherIndex++)
    {
        auto& publisher = publishers[publisherIndex];

        EXPECT_EQ(ParticipantState::Shutdown, publisher.WaitForShutdown());
        EXPECT_EQ(numMessages, publisher.NumMessagesSent());
        EXPECT_EQ(numMessages, subscriber.NumMessagesReceived(publisherIndex));
    }
}

} // anonymous namespace
