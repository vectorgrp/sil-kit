// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// Integration test for the experimental service-discovery C API
// (SilKit_Experimental_ServiceDiscovery_*), driven against real participants over an
// in-process registry.
//
// It validates the motivating use case: an observer correlating subscribers to the publisher
// they are connected to. The authoritative connection edge inside SIL Kit is the per-publisher
// transport UUID, carried by the internal DataSubscriberInternal service. The C API deliberately
// does not expose that internal service or the UUID.
//
// The intended design (follow-up) keeps the public struct as-is and instead does bookkeeping at
// the C API boundary over the internal DataPublisher/DataSubscriberInternal announcements,
// synthesizing a user-facing DataSubscriber ServiceCreated/ServiceRemoved event *if and only if*
// a valid internal match exists (i.e. the subscriber is actually connected to a publisher).
//
// Therefore:
//   - The enabled tests below cover the matched happy-path observable today AND after the
//     follow-up (the gated events fire in a matched scenario either way).
//   - The DISABLED_ tests are the validators for the follow-up: they assert the *gating*
//     (a subscriber with no matching publisher must produce no event) and the *removal
//     synthesis* (destroying the publisher disconnects the subscriber). They fail today and are
//     the acceptance criteria for the follow-up. Run with --gtest_also_run_disabled_tests.

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "silkit/SilKit.hpp"
#include "silkit/capi/SilKit.h"
#include "silkit/config/IParticipantConfiguration.hpp"
#include "silkit/services/pubsub/all.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"

#include "gtest/gtest.h"

namespace {

using namespace std::chrono_literals;
using SilKit::Services::PubSub::PubSubSpec;
using SilKit::Services::PubSub::DataMessageEvent;
using SilKit::Services::PubSub::IDataSubscriber;

constexpr auto kWaitTimeout = 10s;

// A single service-discovery event as observed through the C API (all borrowed C data copied
// out, since the descriptor and its pointers are only valid for the duration of the callback).
struct ObservedLabel
{
    std::string key;
    std::string value;
    SilKit_LabelKind kind;
};

struct ObservedEvent
{
    SilKit_Experimental_ServiceDiscoveryEvent_Type eventType;
    SilKit_Experimental_ServiceKind serviceKind;
    std::string participantName;
    std::string serviceName;
    std::string primaryIdentifier;
    std::string mediaType;
    std::vector<ObservedLabel> labels;
};

// Shared state between the C callback thread and the test thread.
struct ObserverContext
{
    std::mutex mutex;
    std::condition_variable cv;
    std::vector<ObservedEvent> events;
};

// The C trampoline registered with SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler.
// Copies the descriptor into the context and wakes any waiter.
void SilKitCALL OnServiceDiscovery(void* context, SilKit_Experimental_ServiceDiscoveryEvent_Type eventType,
                                   const SilKit_Experimental_ServiceDescriptor* descriptor)
{
    auto* ctx = static_cast<ObserverContext*>(context);

    ObservedEvent event{};
    event.eventType = eventType;
    event.serviceKind = descriptor->serviceKind;
    event.participantName = descriptor->participantName;
    event.serviceName = descriptor->serviceName;
    event.primaryIdentifier = descriptor->primaryIdentifier;
    event.mediaType = descriptor->mediaType;
    for (size_t i = 0; i < descriptor->labelList.numLabels; ++i)
    {
        const auto& label = descriptor->labelList.labels[i];
        event.labels.push_back({label.key, label.value, label.kind});
    }

    {
        std::lock_guard<std::mutex> lock{ctx->mutex};
        ctx->events.push_back(std::move(event));
    }
    ctx->cv.notify_all();
}

class ITest_CapiServiceDiscovery : public testing::Test
{
protected:
    void SetUp() override
    {
        _registry = SilKit::Vendor::Vector::CreateSilKitRegistry(
            SilKit::Config::ParticipantConfigurationFromString(""));
        _registryUri = _registry->StartListening("silkit://127.0.0.1:0");

        // The observer is created through the C API so we get a genuine SilKit_Participant* to
        // pass to the experimental discovery API (no casting between the C and C++ worlds).
        SilKit_ParticipantConfiguration* config{nullptr};
        ASSERT_EQ(SilKit_ParticipantConfiguration_FromString(&config, ""), SilKit_ReturnCode_SUCCESS);
        ASSERT_EQ(SilKit_Participant_Create(&_observer, config, "Observer", _registryUri.c_str()),
                  SilKit_ReturnCode_SUCCESS);
        SilKit_ParticipantConfiguration_Destroy(config);

        ASSERT_EQ(SilKit_Experimental_ServiceDiscovery_Create(&_serviceDiscovery, _observer),
                  SilKit_ReturnCode_SUCCESS);
        ASSERT_EQ(SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(_serviceDiscovery, &_ctx,
                                                                                  &OnServiceDiscovery),
                  SilKit_ReturnCode_SUCCESS);
    }

    void TearDown() override
    {
        // The service discovery observer is owned by the participant; destroying the participant
        // tears it down. Participants created by the individual tests are destroyed at the end of
        // the test body (before this runs). _ctx outlives all of them.
        if (_observer != nullptr)
        {
            SilKit_Participant_Destroy(_observer);
            _observer = nullptr;
        }
        _registry.reset();
    }

    auto CreateParticipant(const std::string& name) -> std::unique_ptr<SilKit::IParticipant>
    {
        return SilKit::CreateParticipant(SilKit::Config::ParticipantConfigurationFromString(""), name,
                                         _registryUri);
    }

    // Counts observed events matching kind + type + primaryIdentifier. Caller must hold _ctx.mutex.
    auto CountUnlocked(SilKit_Experimental_ServiceKind kind, SilKit_Experimental_ServiceDiscoveryEvent_Type type,
                       const std::string& primaryIdentifier) const -> size_t
    {
        return static_cast<size_t>(std::count_if(_ctx.events.begin(), _ctx.events.end(), [&](const auto& e) {
            return e.serviceKind == kind && e.eventType == type && e.primaryIdentifier == primaryIdentifier;
        }));
    }

    auto Count(SilKit_Experimental_ServiceKind kind, SilKit_Experimental_ServiceDiscoveryEvent_Type type,
               const std::string& primaryIdentifier) -> size_t
    {
        std::lock_guard<std::mutex> lock{_ctx.mutex};
        return CountUnlocked(kind, type, primaryIdentifier);
    }

    template <typename Predicate>
    auto WaitFor(Predicate predicate, std::chrono::milliseconds timeout = kWaitTimeout) -> bool
    {
        std::unique_lock<std::mutex> lock{_ctx.mutex};
        return _ctx.cv.wait_for(lock, timeout, [&] { return predicate(); });
    }

    // Returns a copy of the first observed event matching kind + type + primaryIdentifier.
    auto FindEvent(SilKit_Experimental_ServiceKind kind, SilKit_Experimental_ServiceDiscoveryEvent_Type type,
                   const std::string& primaryIdentifier) -> ObservedEvent
    {
        std::lock_guard<std::mutex> lock{_ctx.mutex};
        const auto it = std::find_if(_ctx.events.begin(), _ctx.events.end(), [&](const auto& e) {
            return e.serviceKind == kind && e.eventType == type && e.primaryIdentifier == primaryIdentifier;
        });
        return it == _ctx.events.end() ? ObservedEvent{} : *it;
    }

    std::unique_ptr<SilKit::Vendor::Vector::ISilKitRegistry> _registry;
    std::string _registryUri;
    SilKit_Participant* _observer{nullptr};
    SilKit_Experimental_ServiceDiscovery* _serviceDiscovery{nullptr};
    ObserverContext _ctx;
};

// 1 publisher + 2 subscribers on the same topic. The observer must see the publisher and both
// subscribers with the right kind, primaryIdentifier (== topic), mediaType and labels.
// (In this matched scenario the events fire whether or not they are gated on a connection, so
// this stays green across the follow-up; the semantic then shifts from "created" to "connected".)
TEST_F(ITest_CapiServiceDiscovery, observer_sees_publisher_and_both_subscribers)
{
    const std::string topic{"T"};
    const std::string mediaType{"M"};

    PubSubSpec spec{topic, mediaType};
    spec.AddLabel("K", "V", SilKit::Services::MatchingLabel::Kind::Mandatory);

    auto publisher = CreateParticipant("Pub");
    auto subscriber1 = CreateParticipant("Sub1");
    auto subscriber2 = CreateParticipant("Sub2");

    publisher->CreateDataPublisher("PubCtrl", spec, 1);
    subscriber1->CreateDataSubscriber("SubCtrl1", spec, [](IDataSubscriber*, const DataMessageEvent&) {});
    subscriber2->CreateDataSubscriber("SubCtrl2", spec, [](IDataSubscriber*, const DataMessageEvent&) {});

    ASSERT_TRUE(WaitFor([&] {
        return CountUnlocked(SilKit_Experimental_ServiceKind_DataPublisher,
                             SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated, topic)
                   >= 1
               && CountUnlocked(SilKit_Experimental_ServiceKind_DataSubscriber,
                                SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated, topic)
                      >= 2;
    })) << "observer did not see the publisher and both subscribers within the timeout";

    EXPECT_EQ(Count(SilKit_Experimental_ServiceKind_DataPublisher,
                    SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated, topic),
              1u);
    EXPECT_EQ(Count(SilKit_Experimental_ServiceKind_DataSubscriber,
                    SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated, topic),
              2u);

    // The publisher descriptor must carry the topic as primaryIdentifier, plus mediaType + labels.
    const auto publisherEvent = FindEvent(SilKit_Experimental_ServiceKind_DataPublisher,
                                          SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated, topic);
    EXPECT_EQ(publisherEvent.primaryIdentifier, topic);
    EXPECT_EQ(publisherEvent.mediaType, mediaType);
    ASSERT_EQ(publisherEvent.labels.size(), 1u);
    EXPECT_EQ(publisherEvent.labels[0].key, "K");
    EXPECT_EQ(publisherEvent.labels[0].value, "V");
    EXPECT_EQ(publisherEvent.labels[0].kind, SilKit_LabelKind_Mandatory);
}

// Ground truth (independent of the C API): the two subscribers really connect to the publisher.
// The publisher uses history=1 and publishes once, so each subscriber receives the sample upon
// connecting, regardless of ordering.
TEST_F(ITest_CapiServiceDiscovery, both_subscribers_receive_published_data)
{
    const std::string topic{"T"};
    const std::string mediaType{"M"};
    PubSubSpec spec{topic, mediaType};

    struct ReceiveLatch
    {
        std::mutex mutex;
        std::condition_variable cv;
        int count{0};
        void Hit()
        {
            {
                std::lock_guard<std::mutex> lock{mutex};
                ++count;
            }
            cv.notify_all();
        }
        auto Wait(int expected, std::chrono::milliseconds timeout) -> bool
        {
            std::unique_lock<std::mutex> lock{mutex};
            return cv.wait_for(lock, timeout, [&] { return count >= expected; });
        }
    } latch;

    auto publisher = CreateParticipant("Pub");
    auto* dataPublisher = publisher->CreateDataPublisher("PubCtrl", spec, 1);
    dataPublisher->Publish(std::vector<uint8_t>{42});

    auto subscriber1 = CreateParticipant("Sub1");
    auto subscriber2 = CreateParticipant("Sub2");
    subscriber1->CreateDataSubscriber("SubCtrl1", spec, [&](IDataSubscriber*, const DataMessageEvent&) { latch.Hit(); });
    subscriber2->CreateDataSubscriber("SubCtrl2", spec, [&](IDataSubscriber*, const DataMessageEvent&) { latch.Hit(); });

    EXPECT_TRUE(latch.Wait(2, kWaitTimeout)) << "both subscribers should receive the published sample";
}

// The gating discriminator. A subscriber whose topic has no publisher must NOT produce a
// DataSubscriber ServiceCreated event at the observer: the C API only reports a subscriber once it
// is connected to a publisher (a confirmed internal match).
//
// The barrier is deterministic (no wall-clock wait): the unmatched subscriber and a sentinel
// publisher are created, in that order, on the SAME participant. All of a participant's service
// announcements reach the observer over a single in-order connection, so the subscriber's
// announcement is delivered no later than the sentinel's. Once the observer has seen the sentinel
// publisher, it has necessarily already processed the subscriber's announcement -- so if the
// (removed) premature-report behavior were present, the event would already be recorded.
TEST_F(ITest_CapiServiceDiscovery, no_subscriber_event_without_matching_publisher)
{
    const std::string lonelyTopic{"LonelyTopic"};
    const std::string sentinelTopic{"SentinelTopic"};
    const std::string mediaType{"M"};

    auto participant = CreateParticipant("LonelyParticipant");

    // Unmatched subscriber first, then the sentinel publisher (unrelated topic, so it never matches
    // the subscriber) -- both on the same participant to get in-order delivery to the observer.
    participant->CreateDataSubscriber("SubCtrl", PubSubSpec{lonelyTopic, mediaType},
                                      [](IDataSubscriber*, const DataMessageEvent&) {});
    participant->CreateDataPublisher("SentinelPub", PubSubSpec{sentinelTopic, mediaType}, 0);

    ASSERT_TRUE(WaitFor([&] {
        return CountUnlocked(SilKit_Experimental_ServiceKind_DataPublisher,
                             SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated, sentinelTopic)
               >= 1;
    })) << "observer never saw the sentinel publisher";

    EXPECT_EQ(Count(SilKit_Experimental_ServiceKind_DataSubscriber,
                    SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated, lonelyTopic),
              0u)
        << "an unmatched subscriber must not be reported as connected";
}

// The removal-synthesis half. Start matched (observer sees the subscriber connected), then destroy
// the publisher; the observer must receive a synthesized DataSubscriber ServiceRemoved
// (disconnected) event once the subscriber's last connection drops.
TEST_F(ITest_CapiServiceDiscovery, subscriber_event_removed_when_publisher_leaves)
{
    const std::string topic{"T"};
    const std::string mediaType{"M"};
    PubSubSpec spec{topic, mediaType};

    auto publisher = CreateParticipant("Pub");
    auto subscriber = CreateParticipant("Sub");
    publisher->CreateDataPublisher("PubCtrl", spec, 1);
    subscriber->CreateDataSubscriber("SubCtrl", spec, [](IDataSubscriber*, const DataMessageEvent&) {});

    ASSERT_TRUE(WaitFor([&] {
        return CountUnlocked(SilKit_Experimental_ServiceKind_DataSubscriber,
                             SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated, topic)
               >= 1;
    })) << "observer never saw the connected subscriber";

    publisher.reset();

    ASSERT_TRUE(WaitFor([&] {
        return CountUnlocked(SilKit_Experimental_ServiceKind_DataSubscriber,
                             SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceRemoved, topic)
               >= 1;
    })) << "observer should see the subscriber disconnected when the publisher leaves";
}

} // namespace
