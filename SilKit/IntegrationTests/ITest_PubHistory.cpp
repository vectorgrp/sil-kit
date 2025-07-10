// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "silkit/SilKit.hpp"
#include "silkit/config/IParticipantConfiguration.hpp"
#include "silkit/participant/IParticipant.hpp"
#include "silkit/services/pubsub/IDataPublisher.hpp"
#include "silkit/services/pubsub/IDataSubscriber.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"

#include <atomic>
#include <chrono>
#include <future>
#include <thread>

namespace {

using namespace std::chrono_literals;

constexpr const char* PUBLISHER_NAME = "P";

constexpr const char* EMPTY_PART_CONF = R"(
)";

constexpr const char* HISTORY_ZERO_PART_CONF = R"(
DataPublishers:
  - Name: P
    History: 0
)";

constexpr const char* HISTORY_ONE_PART_CONF = R"(
DataPublishers:
  - Name: P
    History: 1
)";

constexpr const char* HISTORY_TWO_PART_CONF = R"(
DataPublishers:
  - Name: P
    History: 2
)";

struct Participant
{
    std::shared_ptr<SilKit::Config::IParticipantConfiguration> configuration;
    std::unique_ptr<SilKit::IParticipant> participant;

    void SetupParticipant(const char* configurationString, const std::string& participantName,
                          const std::string& registryUri)
    {
        this->configuration = SilKit::Config::ParticipantConfigurationFromString(configurationString);
        this->participant = SilKit::CreateParticipant(configuration, participantName, registryUri);
    }
};

struct PublisherParticipant : Participant
{
    SilKit::Services::PubSub::IDataPublisher* publisher{nullptr};

    void SetupPublisher(const std::string& name, const SilKit::Services::PubSub::PubSubSpec& spec, std::size_t history)
    {
        this->publisher = this->participant->CreateDataPublisher(name, spec, history);
    }
};

struct SubscriberParticipant : Participant
{
    SilKit::Services::PubSub::IDataSubscriber* subscriber{nullptr};
    std::atomic<std::uint64_t> counter{0};

    void SetupSubscriber(const std::string& name, const SilKit::Services::PubSub::PubSubSpec& spec)
    {
        this->subscriber =
            this->participant->CreateDataSubscriber(name, spec, [this](auto, const auto&) { this->counter += 1; });
    }
};

void ExpectMessage(SubscriberParticipant& s, std::chrono::nanoseconds timeout)
{
    const auto deadline = std::chrono::steady_clock::now() + timeout;

    while (std::chrono::steady_clock::now() < deadline)
    {
        if (s.counter > 0)
        {
            break;
        }

        std::this_thread::sleep_for(100ms);
    }

    ASSERT_GT(s.counter, 0);
}

void ExpectNoMessage(SubscriberParticipant& s, std::chrono::nanoseconds timeout)
{
    const auto deadline = std::chrono::steady_clock::now() + timeout;

    while (std::chrono::steady_clock::now() < deadline)
    {
        ASSERT_EQ(s.counter, 0);
        std::this_thread::sleep_for(100ms);
    }
}

struct ITest_PubHistory : testing::Test
{
    void SetUp() override
    {
        this->registryConfiguration = SilKit::Config::ParticipantConfigurationFromString("");
        this->spec = SilKit::Services::PubSub::PubSubSpec{"P", "application/octet-stream"};
        this->data = std::vector<std::uint8_t>{0xCA, 0xFE, 0xAF, 0xFE};
    }

    std::shared_ptr<SilKit::Config::IParticipantConfiguration> registryConfiguration;
    SilKit::Services::PubSub::PubSubSpec spec;
    std::vector<std::uint8_t> data;
};

TEST_F(ITest_PubHistory, history_api_one_conf_empty)
{
    const auto registry = SilKit::Vendor::Vector::CreateSilKitRegistry(registryConfiguration);
    const auto registryUri = registry->StartListening("silkit://127.0.0.1:0");

    PublisherParticipant a;
    a.SetupParticipant(EMPTY_PART_CONF, "A", registryUri);
    a.SetupPublisher(PUBLISHER_NAME, spec, 1);

    // publish a single message, which will be historized and picked up by the subscriber
    a.publisher->Publish(SilKit::Util::ToSpan(data));

    SubscriberParticipant b;
    b.SetupParticipant(EMPTY_PART_CONF, "B", registryUri);
    b.SetupSubscriber("S", spec);

    ExpectMessage(b, 2s);
}

TEST_F(ITest_PubHistory, history_api_zero_conf_empty)
{
    const auto registry = SilKit::Vendor::Vector::CreateSilKitRegistry(registryConfiguration);
    const auto registryUri = registry->StartListening("silkit://127.0.0.1:0");

    PublisherParticipant a;
    a.SetupParticipant(EMPTY_PART_CONF, "A", registryUri);
    a.SetupPublisher(PUBLISHER_NAME, spec, 0);

    // publish a single message, which will not be historized
    a.publisher->Publish(SilKit::Util::ToSpan(data));

    SubscriberParticipant b;
    b.SetupParticipant(EMPTY_PART_CONF, "B", registryUri);
    b.SetupSubscriber("S", spec);

    ExpectNoMessage(b, 2s);
}

TEST_F(ITest_PubHistory, history_api_one_conf_one)
{
    const auto registry = SilKit::Vendor::Vector::CreateSilKitRegistry(registryConfiguration);
    const auto registryUri = registry->StartListening("silkit://127.0.0.1:0");

    PublisherParticipant a;
    a.SetupParticipant(HISTORY_ONE_PART_CONF, "A", registryUri);
    a.SetupPublisher(PUBLISHER_NAME, spec, 1);

    // publish a single message, which will be historized and picked up by the subscriber
    a.publisher->Publish(SilKit::Util::ToSpan(data));

    SubscriberParticipant b;
    b.SetupParticipant(EMPTY_PART_CONF, "B", registryUri);
    b.SetupSubscriber("S", spec);

    ExpectMessage(b, 2s);
}

TEST_F(ITest_PubHistory, history_api_zero_conf_one)
{
    const auto registry = SilKit::Vendor::Vector::CreateSilKitRegistry(registryConfiguration);
    const auto registryUri = registry->StartListening("silkit://127.0.0.1:0");

    PublisherParticipant a;
    a.SetupParticipant(HISTORY_ONE_PART_CONF, "A", registryUri);
    a.SetupPublisher(PUBLISHER_NAME, spec, 0);

    // publish a single message, which will be historized and picked up by the subscriber
    a.publisher->Publish(SilKit::Util::ToSpan(data));

    SubscriberParticipant b;
    b.SetupParticipant(EMPTY_PART_CONF, "B", registryUri);
    b.SetupSubscriber("S", spec);

    ExpectMessage(b, 2s);
}

TEST_F(ITest_PubHistory, history_api_one_conf_zero)
{
    const auto registry = SilKit::Vendor::Vector::CreateSilKitRegistry(registryConfiguration);
    const auto registryUri = registry->StartListening("silkit://127.0.0.1:0");

    PublisherParticipant a;
    a.SetupParticipant(HISTORY_ZERO_PART_CONF, "A", registryUri);
    a.SetupPublisher(PUBLISHER_NAME, spec, 1);

    // publish a single message, which will not be historized
    a.publisher->Publish(SilKit::Util::ToSpan(data));

    SubscriberParticipant b;
    b.SetupParticipant(EMPTY_PART_CONF, "B", registryUri);
    b.SetupSubscriber("S", spec);

    ExpectNoMessage(b, 2s);
}

TEST_F(ITest_PubHistory, history_api_zero_conf_zero)
{
    const auto registry = SilKit::Vendor::Vector::CreateSilKitRegistry(registryConfiguration);
    const auto registryUri = registry->StartListening("silkit://127.0.0.1:0");

    PublisherParticipant a;
    a.SetupParticipant(HISTORY_ZERO_PART_CONF, "A", registryUri);
    a.SetupPublisher(PUBLISHER_NAME, spec, 0);

    // publish a single message, which will not be historized
    a.publisher->Publish(SilKit::Util::ToSpan(data));

    SubscriberParticipant b;
    b.SetupParticipant(EMPTY_PART_CONF, "B", registryUri);
    b.SetupSubscriber("S", spec);

    ExpectNoMessage(b, 2s);
}

TEST_F(ITest_PubHistory, history_api_zero_conf_two_error)
{
    const auto registry = SilKit::Vendor::Vector::CreateSilKitRegistry(registryConfiguration);
    const auto registryUri = registry->StartListening("silkit://127.0.0.1:0");

    PublisherParticipant a;
    a.SetupParticipant(HISTORY_TWO_PART_CONF, "A", registryUri);
    EXPECT_THROW(a.SetupPublisher(PUBLISHER_NAME, spec, 0), SilKit::ConfigurationError);
}

} //end namespace
