// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// Integration test for the experimental service-discovery C API
// (SilKit_Experimental_ServiceDiscovery_*), driven against real participants over an
// in-process registry.
//
// Design: subscribers and RPC servers are reported immediately on ServiceCreated (no connection
// gating). When a DataSubscriberInternal confirms a pub/sub match, the parent DataSubscriber
// receives a Service_Updated event carrying the updated numberOfConnections and the peer's
// identity. ServiceRemoved fires only when the service itself is destroyed, not when a connection
// drops.

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
#include "silkit/services/rpc/all.hpp"
#include "silkit/vendor/CreateSilKitRegistry.hpp"

#include "gtest/gtest.h"

namespace {

using namespace std::chrono_literals;
using SilKit::Services::PubSub::PubSubSpec;
using SilKit::Services::PubSub::DataMessageEvent;
using SilKit::Services::PubSub::IDataSubscriber;
using SilKit::Services::Rpc::RpcSpec;
using SilKit::Services::Rpc::RpcCallHandler;
using SilKit::Services::Rpc::RpcCallResultHandler;

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
    uint32_t numberOfConnections{0};
    std::string connectedParticipantName;
    std::string connectedServiceName;
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
    event.numberOfConnections = descriptor->numberOfConnections;
    event.connectedParticipantName =
        descriptor->connectedParticipantName ? descriptor->connectedParticipantName : "";
    event.connectedServiceName = descriptor->connectedServiceName ? descriptor->connectedServiceName : "";

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

    // Returns a copy of the first observed event matching the predicate (caller must NOT hold lock).
    template <typename Predicate>
    auto FindEventWhere(Predicate pred) -> ObservedEvent
    {
        std::lock_guard<std::mutex> lock{_ctx.mutex};
        const auto it = std::find_if(_ctx.events.begin(), _ctx.events.end(), pred);
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

// A subscriber with no matching publisher is reported immediately on creation with
// numberOfConnections == 0. The sentinel ensures in-order delivery (see below).
//
// The barrier is deterministic: the unmatched subscriber and a sentinel publisher are created, in
// that order, on the SAME participant. All announcements from one participant reach the observer
// over a single in-order connection, so the subscriber's announcement is delivered no later than
// the sentinel's. Once the observer has seen the sentinel publisher, it has necessarily already
// processed the subscriber's announcement.
TEST_F(ITest_CapiServiceDiscovery, subscriber_visible_immediately_without_matching_publisher)
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

    // Subscriber was announced before the sentinel; the observer must have seen it by now.
    EXPECT_EQ(Count(SilKit_Experimental_ServiceKind_DataSubscriber,
                    SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated, lonelyTopic),
              1u)
        << "a subscriber must be reported immediately on creation, even without a matching publisher";

    // And the connection count must be zero (no publisher matched).
    const auto ev = FindEvent(SilKit_Experimental_ServiceKind_DataSubscriber,
                              SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceCreated, lonelyTopic);
    EXPECT_EQ(ev.numberOfConnections, 0u);
}

// When the publisher leaves, the observer must receive a Service_Updated event on the subscriber
// with numberOfConnections == 0. ServiceRemoved does NOT fire (the subscriber is still alive).
TEST_F(ITest_CapiServiceDiscovery, subscriber_updated_when_publisher_leaves)
{
    const std::string topic{"T"};
    const std::string mediaType{"M"};
    PubSubSpec spec{topic, mediaType};

    auto publisher = CreateParticipant("Pub");
    auto subscriber = CreateParticipant("Sub");
    publisher->CreateDataPublisher("PubCtrl", spec, 1);
    subscriber->CreateDataSubscriber("SubCtrl", spec, [](IDataSubscriber*, const DataMessageEvent&) {});

    // Wait for the connection to be established before destroying the publisher.
    ASSERT_TRUE(WaitFor([&] {
        return std::any_of(_ctx.events.begin(), _ctx.events.end(), [&](const auto& e) {
            return e.serviceKind == SilKit_Experimental_ServiceKind_DataSubscriber
                   && e.eventType == SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated
                   && e.primaryIdentifier == topic && e.numberOfConnections >= 1;
        });
    })) << "observer never saw the subscriber connected to the publisher (Service_Updated, connections>=1)";

    publisher.reset();

    ASSERT_TRUE(WaitFor([&] {
        return std::any_of(_ctx.events.begin(), _ctx.events.end(), [&](const auto& e) {
            return e.serviceKind == SilKit_Experimental_ServiceKind_DataSubscriber
                   && e.eventType == SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated
                   && e.primaryIdentifier == topic && e.numberOfConnections == 0;
        });
    })) << "observer should see Service_Updated(connections=0) when the publisher leaves";
}

// When a publisher matches a subscriber, the Service_Updated event carried on the subscriber must
// name the publisher: connectedParticipantName == publisher's participant name and
// connectedServiceName == publisher's controller name. This lets the observer build a confirmed
// connection graph from service discovery data alone.
TEST_F(ITest_CapiServiceDiscovery, pubsub_service_updated_carries_publisher_peer_identity)
{
    const std::string topic{"Sensor"};
    const std::string mediaType{"application/octet-stream"};

    auto publisher = CreateParticipant("PublisherParticipant");
    auto subscriber = CreateParticipant("SubscriberParticipant");
    publisher->CreateDataPublisher("PublisherController", PubSubSpec{topic, mediaType}, 0);
    subscriber->CreateDataSubscriber("SubscriberController", PubSubSpec{topic, mediaType},
                                     [](IDataSubscriber*, const DataMessageEvent&) {});

    ASSERT_TRUE(WaitFor([&] {
        return std::any_of(_ctx.events.begin(), _ctx.events.end(), [&](const auto& e) {
            return e.serviceKind == SilKit_Experimental_ServiceKind_DataSubscriber
                   && e.eventType == SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated
                   && e.primaryIdentifier == topic && e.numberOfConnections >= 1;
        });
    })) << "Service_Updated for matched subscriber never arrived";

    const auto ev = FindEventWhere([&](const ObservedEvent& e) {
        return e.serviceKind == SilKit_Experimental_ServiceKind_DataSubscriber
               && e.eventType == SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated
               && e.primaryIdentifier == topic && e.numberOfConnections >= 1;
    });
    EXPECT_EQ(ev.connectedParticipantName, "PublisherParticipant")
        << "subscriber's Service_Updated must identify the publisher's participant";
    EXPECT_EQ(ev.connectedServiceName, "PublisherController")
        << "subscriber's Service_Updated must identify the publisher's controller name";
}

// The RPC server's Service_Updated must carry the matching client's participant name and
// controller name, letting the observer resolve concrete RPC call edges.
TEST_F(ITest_CapiServiceDiscovery, rpc_service_updated_carries_client_peer_identity)
{
    const std::string functionName{"RemoteProc"};
    const std::string mediaType{"application/octet-stream"};
    RpcSpec spec{functionName, mediaType};

    auto server = CreateParticipant("ServerParticipant");
    auto client = CreateParticipant("ClientParticipant");
    server->CreateRpcServer("ServerController", spec, RpcCallHandler{[](auto*, auto) {}});
    client->CreateRpcClient("ClientController", spec, RpcCallResultHandler{[](auto*, auto) {}});

    ASSERT_TRUE(WaitFor([&] {
        return std::any_of(_ctx.events.begin(), _ctx.events.end(), [&](const auto& e) {
            return e.serviceKind == SilKit_Experimental_ServiceKind_RpcServer
                   && e.eventType == SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated
                   && e.primaryIdentifier == functionName && e.numberOfConnections >= 1;
        });
    })) << "Service_Updated for matched RPC server never arrived";

    const auto ev = FindEventWhere([&](const ObservedEvent& e) {
        return e.serviceKind == SilKit_Experimental_ServiceKind_RpcServer
               && e.eventType == SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated
               && e.primaryIdentifier == functionName && e.numberOfConnections >= 1;
    });
    EXPECT_EQ(ev.connectedParticipantName, "ClientParticipant")
        << "server's Service_Updated must identify the RPC client's participant";
    EXPECT_EQ(ev.connectedServiceName, "ClientController")
        << "server's Service_Updated must identify the RPC client's controller name";
}

// Attaching an observer to an already-running simulation must recover not only the connection count
// but also the peer identity of connections that existed before the observer registered. This is the
// replay-ordering case: the DataSubscriberInternal may be replayed before its parent/publisher.
TEST_F(ITest_CapiServiceDiscovery, late_observer_recovers_peer_of_preexisting_connection)
{
    const std::string topic{"LateTopic"};
    const std::string mediaType{"M"};
    PubSubSpec spec{topic, mediaType};

    auto publisher = CreateParticipant("LatePub");
    auto subscriber = CreateParticipant("LateSub");
    publisher->CreateDataPublisher("LatePubCtrl", spec, 0);
    subscriber->CreateDataSubscriber("LateSubCtrl", spec, [](IDataSubscriber*, const DataMessageEvent&) {});

    // Ensure the match is established (observed via the SetUp observer) before attaching a new one.
    ASSERT_TRUE(WaitFor([&] {
        return std::any_of(_ctx.events.begin(), _ctx.events.end(), [&](const auto& e) {
            return e.serviceKind == SilKit_Experimental_ServiceKind_DataSubscriber
                   && e.eventType == SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated
                   && e.primaryIdentifier == topic && e.numberOfConnections >= 1;
        });
    })) << "the pub/sub connection was never established";

    // Attach a brand-new observer that must replay the already-running simulation.
    SilKit_ParticipantConfiguration* config{nullptr};
    ASSERT_EQ(SilKit_ParticipantConfiguration_FromString(&config, ""), SilKit_ReturnCode_SUCCESS);
    SilKit_Participant* lateObserver{nullptr};
    ASSERT_EQ(SilKit_Participant_Create(&lateObserver, config, "LateObserver", _registryUri.c_str()),
              SilKit_ReturnCode_SUCCESS);
    SilKit_ParticipantConfiguration_Destroy(config);

    ObserverContext lateCtx;
    SilKit_Experimental_ServiceDiscovery* lateSd{nullptr};
    ASSERT_EQ(SilKit_Experimental_ServiceDiscovery_Create(&lateSd, lateObserver), SilKit_ReturnCode_SUCCESS);
    ASSERT_EQ(
        SilKit_Experimental_ServiceDiscovery_SetServiceDiscoveryHandler(lateSd, &lateCtx, &OnServiceDiscovery),
        SilKit_ReturnCode_SUCCESS);

    std::unique_lock<std::mutex> lock{lateCtx.mutex};
    const bool recovered = lateCtx.cv.wait_for(lock, kWaitTimeout, [&] {
        return std::any_of(lateCtx.events.begin(), lateCtx.events.end(), [&](const auto& e) {
            return e.serviceKind == SilKit_Experimental_ServiceKind_DataSubscriber
                   && e.eventType == SilKit_Experimental_ServiceDiscoveryEvent_Type_ServiceUpdated
                   && e.primaryIdentifier == topic && e.numberOfConnections >= 1
                   && e.connectedParticipantName == "LatePub" && e.connectedServiceName == "LatePubCtrl";
        });
    });
    lock.unlock();
    EXPECT_TRUE(recovered) << "late observer must recover the peer identity of the pre-existing connection";

    SilKit_Participant_Destroy(lateObserver);
}

} // namespace
