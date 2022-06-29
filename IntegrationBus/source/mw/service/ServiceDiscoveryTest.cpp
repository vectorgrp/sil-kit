// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <chrono>
#include <functional>
#include <set>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ServiceDiscovery.hpp"
#include "string_utils_internal.hpp"
#include "UuidRandom.hpp"
#include "MockParticipant.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace ib;
using namespace ib::mw;
using namespace ib::mw::service;
using namespace ib::util;

using ::ib::mw::test::DummyParticipant;

class MockParticipant : public DummyParticipant
{
public:
    MOCK_METHOD(void, SendIbMessage, (const IIbServiceEndpoint*, const ParticipantDiscoveryEvent&), (override));
    MOCK_METHOD(void, SendIbMessage, (const IIbServiceEndpoint*, const ServiceDiscoveryEvent&), (override));
};

class MockServiceDescriptor : public IIbServiceEndpoint
{
public:
    ServiceDescriptor serviceDescriptor;
    MockServiceDescriptor(EndpointAddress ea)
    {
        serviceDescriptor.SetNetworkName(to_string(ea));
        serviceDescriptor.SetParticipantName(std::to_string(ea.participant));
        serviceDescriptor.SetServiceName(to_string(ea));
        serviceDescriptor.SetServiceId(ea.endpoint);
    }
    void SetServiceDescriptor(const ServiceDescriptor& _serviceDescriptor) override
    {
        serviceDescriptor = _serviceDescriptor;
    }
    auto GetServiceDescriptor() const -> const ServiceDescriptor & override
    {
        return serviceDescriptor;
    }

};

class Callbacks
{
public:
    MOCK_METHOD(void, ServiceDiscoveryHandler,
        (ServiceDiscoveryEvent::Type, const ServiceDescriptor&));
};
class DiscoveryServiceTest : public testing::Test
{
protected:
    DiscoveryServiceTest()
    {
    }


protected:
    // ----------------------------------------
    // Helper Methods

protected:
    // ----------------------------------------
    // Members

    Callbacks callbacks;
    MockParticipant participant;
};

TEST(ServiceDescriptor, portable_hash_function)
{
    using namespace ib::util::uuid;
    const auto numStrings = 1000;
    std::vector<std::string> testStrings;
    for (auto i = 0; i < numStrings; i++)
    {
        testStrings.push_back(to_string(generate()));
    }
    std::set<uint64_t> hashes;
    for (const auto& s: testStrings)
    {
        hashes.insert(ib::util::hash::Hash(s));
    }
    ASSERT_EQ(hashes.size(), testStrings.size()) << "The test strings need unique 64-bit hashes";
}

TEST_F(DiscoveryServiceTest, service_creation_notification)
{
    ServiceDescriptor senderDescriptor{};
    senderDescriptor.SetParticipantName("ParticipantA");
    senderDescriptor.SetNetworkName("Link1");
    senderDescriptor.SetServiceName("ServiceDiscovery");
    ServiceDiscovery disco{ &participant, "ParticipantA" };
    disco.SetServiceDescriptor(senderDescriptor);

    ServiceDescriptor descr;
    descr = senderDescriptor;

    disco.RegisterServiceDiscoveryHandler(
        [&descr](auto /*eventType*/, auto&& newServiceDescr) {
            ASSERT_EQ(descr.GetSupplementalData(), newServiceDescr.GetSupplementalData());
    });
    disco.RegisterServiceDiscoveryHandler([this](auto type, auto&& serviceDescr) {
        callbacks.ServiceDiscoveryHandler(type, serviceDescr);
    });

    // reference data for validation
    ServiceDiscoveryEvent event;
    event.type = ServiceDiscoveryEvent::Type::ServiceCreated;
    event.serviceDescriptor = descr;
    // NotifyServiceCreated should publish a message
    EXPECT_CALL(participant, SendIbMessage(&disco, event)).Times(1);
    // NotifyServiceCreated should also trigger ourself
    EXPECT_CALL(callbacks, ServiceDiscoveryHandler(ServiceDiscoveryEvent::Type::ServiceCreated,
        descr)).Times(1);
    // trigger notification on the same participant
    disco.NotifyServiceCreated(descr);

    // trigger notifications on reception path from different participant
    MockServiceDescriptor otherParticipant{ {1, 2} };
    descr.SetParticipantName("ParticipantOther");
    event.serviceDescriptor = descr;
    EXPECT_CALL(callbacks, ServiceDiscoveryHandler(ServiceDiscoveryEvent::Type::ServiceCreated,
        descr)).Times(1);

    // when sending a different service descriptor, we expect a notification once
    disco.ReceiveIbMessage(&otherParticipant, event);
    disco.ReceiveIbMessage(&otherParticipant, event);//should not trigger callback, is cached
}
TEST_F(DiscoveryServiceTest, multiple_service_creation_notification)
{
    MockServiceDescriptor otherParticipant{ {1, 2} };
    ServiceDiscovery disco{ &participant, "ParticipantA" };

    disco.RegisterServiceDiscoveryHandler([this](auto type, auto&& descr) {
        callbacks.ServiceDiscoveryHandler(type, descr);
    });

    ServiceDescriptor senderDescriptor;
    senderDescriptor.SetParticipantName("ParticipantA");
    senderDescriptor.SetNetworkName("Link1");
    senderDescriptor.SetServiceName("ServiceDiscovery");

    ServiceDiscoveryEvent event;

    auto sendAnnounce = [&](auto&& serviceName) {
        ServiceDescriptor descr;
        descr = senderDescriptor;
        descr.SetServiceName(serviceName);
        // Ensure we only append new services
        event.type = ServiceDiscoveryEvent::Type::ServiceCreated;
        event.serviceDescriptor = descr;

        // Expect that each serviceDescriptor is only handled by a single notification handler
        // e.g., no duplicate notifications
        EXPECT_CALL(callbacks,
            ServiceDiscoveryHandler(ServiceDiscoveryEvent::Type::ServiceCreated, descr)
        ).Times(1);

        disco.ReceiveIbMessage(&otherParticipant, event);
        disco.ReceiveIbMessage(&otherParticipant, event);//duplicate should not trigger a notification

    };

    for (auto i = 0; i < 10; i++)
    {
        sendAnnounce("Service" + std::to_string(i));
    }
}

TEST_F(DiscoveryServiceTest, service_removal)
{
    MockServiceDescriptor otherParticipant{ {1, 2} };
    ServiceDiscovery disco{ &participant, "ParticipantA" };

    disco.RegisterServiceDiscoveryHandler([this](auto type, auto&& descr) {
        callbacks.ServiceDiscoveryHandler(type, descr);
    });

    ServiceDescriptor senderDescriptor;
    senderDescriptor.SetParticipantName("ParticipantA");
    senderDescriptor.SetNetworkName("Link1");
    senderDescriptor.SetServiceName("ServiceDiscovery");

    ServiceDiscoveryEvent event;

    ServiceDescriptor descr;
    descr = senderDescriptor;
    descr.SetServiceName("TestService");
    event.type = ServiceDiscoveryEvent::Type::ServiceCreated;
    event.serviceDescriptor = descr;

    // Test addition
    EXPECT_CALL(callbacks,
        ServiceDiscoveryHandler(ServiceDiscoveryEvent::Type::ServiceCreated, descr)
    ).Times(1);
    EXPECT_CALL(callbacks,
        ServiceDiscoveryHandler(ServiceDiscoveryEvent::Type::ServiceRemoved, descr)
    ).Times(0);
    disco.ReceiveIbMessage(&otherParticipant, event);

    // add a modified one
    event.serviceDescriptor.SetServiceName("Modified");
    auto modifiedDescr = event.serviceDescriptor;
    EXPECT_CALL(callbacks,
        ServiceDiscoveryHandler(ServiceDiscoveryEvent::Type::ServiceCreated, modifiedDescr)
    ).Times(1);
    EXPECT_CALL(callbacks,
        ServiceDiscoveryHandler(ServiceDiscoveryEvent::Type::ServiceRemoved, modifiedDescr)
    ).Times(0);
    disco.ReceiveIbMessage(&otherParticipant, event);
    // Test removal
    event.type = ServiceDiscoveryEvent::Type::ServiceRemoved;
    EXPECT_CALL(callbacks,
        ServiceDiscoveryHandler(ServiceDiscoveryEvent::Type::ServiceRemoved, modifiedDescr)
    ).Times(1);
    disco.ReceiveIbMessage(&otherParticipant, event);

    // Nothing to remove, no triggers
    event.type = ServiceDiscoveryEvent::Type::ServiceRemoved;
    EXPECT_CALL(callbacks,
        ServiceDiscoveryHandler(_, _)
    ).Times(0);
    disco.ReceiveIbMessage(&otherParticipant, event);
}
} // anonymous namespace for test
