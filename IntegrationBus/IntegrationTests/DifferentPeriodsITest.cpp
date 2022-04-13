// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <thread>
#include <future>
#include <string>
#include <sstream>
#include <numeric>

#include "CreateParticipant.hpp"
#include "VAsioRegistry.hpp"
#include "MockParticipantConfiguration.hpp"

#include "ib/mw/sync/all.hpp"
#include "ib/sim/all.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "GetTestPid.hpp"

namespace {

using namespace std::chrono;
using namespace ib::cfg;
using namespace ib::mw;
using namespace ib::mw::sync;
using namespace ib::sim::data;

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
    Publisher(const uint32_t domainId, const uint32_t publisherIndex, const uint32_t numMessages, std::chrono::nanoseconds period)
        : _numMessages{numMessages}
    {
        std::string participantName = "Publisher" + std::to_string(publisherIndex);
        _participant =
            ib::mw::CreateParticipantImpl(ib::cfg::MockParticipantConfiguration(), participantName, true);

        _participant->joinIbDomain(domainId);

        const auto topicName = "Topic" + std::to_string(publisherIndex);
        auto&& participantController = _participant->GetParticipantController();
        auto* publisher = _participant->CreateDataPublisher(topicName, {}, {}, 0);

        participantController->SetPeriod(period);
        participantController->SetSimulationTask(
            [this, publisher, period](const nanoseconds now, nanoseconds /*duration*/) {
                ASSERT_TRUE((now.count() % period.count()) == 0);
                if (_messageIndex < _numMessages)
                {
                    PublishMessage(publisher, now, _messageIndex++);
                }
            });
    }

    void RunAsync()
    {
        auto&& participantController = _participant->GetParticipantController();
        _simulationFuture = participantController->RunAsync();
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
    std::unique_ptr<IParticipantInternal> _participant{nullptr};

    uint32_t _messageIndex{0u};
    uint32_t _numMessages{0u};
    std::future<ParticipantState> _simulationFuture;
};

class Subscriber
{
public:
    Subscriber(const std::vector<std::string>& syncParticipantNames, const std::string& participantName,
               const uint32_t domainId, const uint32_t& publisherCount, const uint32_t numMessages)
        : _publisherCount{publisherCount}
        , _messageIndexes(publisherCount, 0u)
        , _numMessages{numMessages}
        , _syncParticipantNames { syncParticipantNames }
    {
        _participant = ib::mw::CreateParticipantImpl(
            ib::cfg::MockParticipantConfiguration(), participantName, true);
        _participant->joinIbDomain(domainId);

        _systemController = _participant->GetSystemController();
        _systemController->SetRequiredParticipants(_syncParticipantNames);

        _monitor = _participant->GetSystemMonitor();
        _monitor->RegisterSystemStateHandler(
            [this](SystemState newState) {
            this->OnSystemStateChanged(newState);
        });

        auto&& participantController = _participant->GetParticipantController();

        for (auto publisherIndex = 0u; publisherIndex < _publisherCount; publisherIndex++)
        {
            _participant->CreateDataSubscriber(
                "Topic" + std::to_string(publisherIndex), {}, {},
                [this, publisherIndex](IDataSubscriber* subscriber, const std::vector<uint8_t>& data) {
                    ReceiveMessage(subscriber, data, publisherIndex);
                });
        }
        participantController->SetPeriod(subscriberPeriod);
        participantController->SetSimulationTask(
            [this](const nanoseconds now, nanoseconds /*duration*/) {
            _currentTime = now;
            ASSERT_TRUE((_currentTime.count() % subscriberPeriod.count()) == 0);
        });
    }

    std::future<ParticipantState> RunAsync() const
    {
        auto&& participantController = _participant->GetParticipantController();
        return participantController->RunAsync();
    }

    uint32_t NumMessagesReceived(const uint32_t publisherIndex)
    {
        return _messageIndexes[publisherIndex];
    }

private:
    void OnSystemStateChanged(SystemState newState)
    {
        if (newState == SystemState::Idle)
        {
            InitializeAllParticipants();
        }
        else if (newState == SystemState::Initialized)
        {
            _systemController->Run();
        }
        else if (newState == SystemState::Stopped)
        {
            _systemController->Shutdown();
        }
    }

    void InitializeAllParticipants()
    {
        for (auto&& name : _syncParticipantNames)
        {
            _systemController->Initialize(name);
        }
    }

    void ReceiveMessage(IDataSubscriber* /*subscriber*/, const std::vector<uint8_t>& data,
                        const uint32_t publisherIndex)
    {
        auto& messageIndex = _messageIndexes[publisherIndex];
        const std::string message{data.begin(), data.end()};

        std::string receivedMessage;
        nanoseconds sentTime = {};
        uint32_t receivedIndex;
        std::stringstream stream{message};

        try
        {
            stream >> receivedMessage >> sentTime >> receivedIndex;
        }
        catch (std::runtime_error& /*error*/)
        {
            std::cout << "ERROR: Received message does not match the expected format" << std::endl;
            _systemController->Stop();
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
            _systemController->Stop();
        }
    }

private:
    uint32_t _publisherCount{0u};
    std::vector<uint32_t> _messageIndexes;
    uint32_t _numMessages{0u};
    std::vector<std::string> _syncParticipantNames;
    std::unique_ptr<IParticipantInternal> _participant{nullptr};
    ISystemController* _systemController{nullptr};
    ISystemMonitor* _monitor{nullptr};

    std::chrono::nanoseconds _currentTime{0ns};
};

class DifferentPeriodsITest : public testing::Test
{
protected:
    DifferentPeriodsITest()
    {
        domainId = static_cast<uint32_t>(GetTestPid());
    }

protected:
    uint32_t domainId;

};


// Tests for deterministic reception of messages using participants
// with different periods
TEST_F(DifferentPeriodsITest, different_simtask_periods)
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

    VAsioRegistry registry{ib::cfg::MockParticipantConfiguration()};
    registry.ProvideDomain(domainId);

    // The subscriber assumes the role of the system controller and initiates simulation state changes
    Subscriber subscriber(syncParticipantNames, subscriberName, domainId, publisherCount, numMessages);
    auto subscriberFuture = subscriber.RunAsync();

    std::vector<Publisher> publishers;
    publishers.reserve(publisherCount);
    for (auto i = 0u; i < publisherCount; i++)
    {
        publishers.emplace_back(domainId, i, numMessages, publisherPeriods[i]);
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
