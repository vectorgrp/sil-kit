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
#include "silkit/services/all.hpp"
#include "silkit/config/IParticipantConfiguration.hpp"
#include "silkit/SilKit.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"
#include "silkit/experimental/participant/ParticipantExtensions.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using namespace std::chrono;
using namespace SilKit::Config;
using namespace SilKit::Services::Orchestration;
using namespace SilKit::Services::PubSub;

const std::string testMessage{"TestMessage"};
std::vector<std::string> syncParticipantNames;
auto period = std::chrono::milliseconds(1);

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
    Publisher(const std::string& registryUri, const uint32_t publisherIndex, const uint32_t testSize)
        : _testSize{testSize}
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
        timeSyncService->SetSimulationStepHandler([this, publisher](const nanoseconds now, nanoseconds /*duration*/) {
            if (_messageIndex < _testSize)
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
    SilKit::Services::Orchestration::ILifecycleService* _lifecycleService{nullptr};

    uint32_t _messageIndex{0u};
    uint32_t _testSize{0u};
    std::future<ParticipantState> _simulationFuture;
    std::string _participantName;
};

class Subscriber
{
public:
    Subscriber(const std::string& participantName, const std::string& registryUri, const uint32_t& publisherCount,
               const uint32_t testSize)
        : _publisherCount{publisherCount}
        , _messageIndexes(publisherCount, 0u)
        , _testSize{testSize}
        , _participantName{participantName}
    {
        _participant = SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(""),
                                                 participantName, registryUri);

        _systemController = SilKit::Experimental::Participant::CreateSystemController(_participant.get());
        _systemController->SetWorkflowConfiguration({syncParticipantNames});

        _lifecycleService =
            _participant->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Coordinated});

        _monitor = _participant->CreateSystemMonitor();

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

        timeSyncService->SetSimulationStepHandler(
            [this](const nanoseconds now, nanoseconds /*duration*/) { _currentTick = now; }, period);
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
        nanoseconds sentTick = {};
        uint32_t receivedIndex;
        std::stringstream stream{message};

        try
        {
            stream >> receivedMessage >> sentTick >> receivedIndex;
        }
        catch (SilKit::SilKitError& /*error*/)
        {
            std::cout << "ERROR: Received message does not match the expected format" << std::endl;
            _systemController->AbortSimulation();
            return;
        }

        // The constant testMessage should have been sent and received correctly
        EXPECT_EQ(testMessage, receivedMessage);

        // Exactly one message per publisher should be sent and received in the same simulation tick.

        // currentTick is incremented by one millisecond with every simulation tick.
        // So sentTick should be equal to currentTick or one millisecond (=tick) further,
        // if currentTick has not yet been updated in the current tick.
        ASSERT_TRUE(_currentTick == sentTick || _currentTick + period == sentTick);

        // This expectation tests the order of the messages per publisher.
        // For each publisher each send message should come in order. The order is tested
        // by incrementing the messageIndex for each message in the Publisher and Subscriber.
        ASSERT_EQ(messageIndex, receivedIndex);

        messageIndex++;

        const auto sumOfIndexes = std::accumulate(_messageIndexes.begin(), _messageIndexes.end(), 0u);
        if (sumOfIndexes == _testSize * _publisherCount)
        {
            _lifecycleService->Stop("End Test");
        }
    }

private:
    std::unique_ptr<SilKit::IParticipant> _participant{nullptr};
    SilKit::Experimental::Services::Orchestration::ISystemController* _systemController{nullptr};
    ILifecycleService* _lifecycleService{nullptr};
    ISystemMonitor* _monitor{nullptr};

    uint32_t _publisherCount{0u};
    std::vector<uint32_t> _messageIndexes;
    uint32_t _testSize{0u};

    nanoseconds _currentTick{0ns};
    std::string _participantName;
};

class ITest_DeterministicSimVAsio : public testing::Test
{
protected:
    ITest_DeterministicSimVAsio() = default;
};

TEST_F(ITest_DeterministicSimVAsio, deterministic_simulation_vasio)
{
    const uint32_t publisherCount = 3;
    const uint32_t testSize = 5000;

    std::string subscriberName = "Subscriber";
    syncParticipantNames.push_back(subscriberName);
    for (auto i = 0u; i < publisherCount; i++)
    {
        syncParticipantNames.push_back("Publisher" + std::to_string(i));
    }

    auto registry =
        SilKit::Vendor::Vector::CreateSilKitRegistry(SilKit::Config::ParticipantConfigurationFromString(""));
    auto registryUri = registry->StartListening("silkit://localhost:0");

    // The subscriber assumes the role of the system controller and initiates simulation state changes
    Subscriber subscriber(subscriberName, registryUri, publisherCount, testSize);
    auto subscriberFuture = subscriber.RunAsync();

    std::vector<Publisher> publishers;
    publishers.reserve(publisherCount);
    for (auto i = 0u; i < publisherCount; i++)
    {
        publishers.emplace_back(registryUri, i, testSize);
        publishers[i].RunAsync();
    }

    // The important expectations of this test are in the ReceiveMessage method of the Subscriber:
    //
    //   1. The sent message (from Publisher1, 2, 3 ...) has to be received in the same tick.
    //   2. The received messages have to come in order (Index 1 before 2 before 3, ...):
    //(e.g.): PubA - Test Message: 1
    //        PubB - Test Message: 1
    //        PubB - Test Message: 2
    //        PubA - Test Message: 2
    //        PubB - Test Message: 3
    //        PubA - Test Message: 3
    //        ...

    auto finalState = subscriberFuture.get();
    EXPECT_EQ(ParticipantState::Shutdown, finalState);

    for (auto publisherIndex = 0u; publisherIndex < publisherCount; publisherIndex++)
    {
        auto& publisher = publishers[publisherIndex];

        EXPECT_EQ(ParticipantState::Shutdown, publisher.WaitForShutdown());
        EXPECT_EQ(testSize, publisher.NumMessagesSent());
        EXPECT_EQ(testSize, subscriber.NumMessagesReceived(publisherIndex));
    }
}

} // anonymous namespace
