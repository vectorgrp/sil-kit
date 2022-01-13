// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ServiceDiscovery.hpp"

#include "ib/mw/string_utils.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"


#include "MockComAdapter.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace ib;
using namespace ib::mw;
using namespace ib::mw::service;
using namespace ib::util;

using ::ib::mw::test::DummyComAdapter;

class MockComAdapter : public DummyComAdapter
{
public:
    MOCK_METHOD(void, SendIbMessage, (const IIbServiceEndpoint*, const ServiceAnnouncement&), (override));
    MOCK_METHOD(void, SendIbMessage, (const IIbServiceEndpoint*, const ServiceDiscoveryEvent&), (override));
};

class MockServiceDescriptor : public IIbServiceEndpoint
{
public:
    ServiceDescriptor serviceDescriptor;
    MockServiceDescriptor(EndpointAddress ea)
    {
        serviceDescriptor.linkName = to_string(ea);
        serviceDescriptor.participantName = to_string(ea);
        serviceDescriptor.serviceName = to_string(ea);
        serviceDescriptor.legacyEpa = ea;
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
    MockComAdapter comAdapter;
};

TEST_F(DiscoveryServiceTest, service_creation_notification)
{
    ServiceDescriptor senderDescriptor{};
    senderDescriptor.participantName = "ParticipantA";
    senderDescriptor.linkName = "Link1";
    senderDescriptor.serviceName = "ServiceDiscovery";
    ServiceDiscovery disco{ &comAdapter, "ParticipantA" };
    disco.SetServiceDescriptor(senderDescriptor);

    ServiceDescriptor descr;
    descr = senderDescriptor;
   
    disco.RegisterServiceDiscoveryHandler(
        [&descr](auto eventType, auto&& newServiceDescr) {
            ASSERT_EQ(descr.supplementalData, newServiceDescr.supplementalData);
    });
    disco.RegisterServiceDiscoveryHandler([this](auto type, auto&& descr) {
        callbacks.ServiceDiscoveryHandler(type, descr);
    });

    // reference data for validation
    ServiceDiscoveryEvent event;
    event.type = ServiceDiscoveryEvent::Type::ServiceCreated;
    event.service = descr;
    //Notify should publish a message
    EXPECT_CALL(comAdapter, SendIbMessage(&disco, event)).Times(1);
    // no callbacks on sending our own
    EXPECT_CALL(callbacks, ServiceDiscoveryHandler(ServiceDiscoveryEvent::Type::ServiceCreated,
        descr)).Times(0);
    // trigger notification on the same participant
    disco.NotifyServiceCreated(descr);

    // trigger notifications on reception path from different participant
    MockServiceDescriptor otherParticipant{ {1, 2} };
    descr.participantName = "ParticipantOther";
    event.service = descr;
    EXPECT_CALL(callbacks, ServiceDiscoveryHandler(ServiceDiscoveryEvent::Type::ServiceCreated,
        descr)).Times(1);

    // when sending a different service descriptor, we expect a notification once
    disco.ReceiveIbMessage(&otherParticipant, event);
    disco.ReceiveIbMessage(&otherParticipant, event);//should not trigger callback, is cached
}
TEST_F(DiscoveryServiceTest, multiple_service_creation_notification)
{
    MockServiceDescriptor otherParticipant{ {1, 2} };
    ServiceDiscovery disco{ &comAdapter, "ParticipantA" };

    disco.RegisterServiceDiscoveryHandler([this](auto type, auto&& descr) {
        callbacks.ServiceDiscoveryHandler(type, descr);
    });

    ServiceDescriptor senderDescriptor;
    senderDescriptor.participantName = "ParticipantA";
    senderDescriptor.linkName = "Link1";
    senderDescriptor.serviceName = "ServiceDiscovery";

    ServiceDiscoveryEvent event;

    auto sendAnnounce = [&](auto&& serviceName) {
        ServiceDescriptor descr;
        descr = senderDescriptor;
        descr.serviceName = serviceName;
        // Ensure we only append new services
        event.type = ServiceDiscoveryEvent::Type::ServiceCreated;
        event.service = descr;

        // Expect that each service is only handled by a single notification handler 
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
    ServiceDiscovery disco{ &comAdapter, "ParticipantA" };

    disco.RegisterServiceDiscoveryHandler([this](auto type, auto&& descr) {
        callbacks.ServiceDiscoveryHandler(type, descr);
    });

    ServiceDescriptor senderDescriptor;
    senderDescriptor.participantName = "ParticipantA";
    senderDescriptor.linkName = "Link1";
    senderDescriptor.serviceName = "ServiceDiscovery";

    ServiceDiscoveryEvent event;

    ServiceDescriptor descr;
    descr = senderDescriptor;
    descr.serviceName = "TestService";
    event.type = ServiceDiscoveryEvent::Type::ServiceCreated;
    event.service = descr;

    // Test addition
    EXPECT_CALL(callbacks,
        ServiceDiscoveryHandler(ServiceDiscoveryEvent::Type::ServiceCreated, descr)
    ).Times(1);
    EXPECT_CALL(callbacks,
        ServiceDiscoveryHandler(ServiceDiscoveryEvent::Type::ServiceRemoved, descr)
    ).Times(0);
    disco.ReceiveIbMessage(&otherParticipant, event);

    // add a modified one
    event.service.serviceName = "Modified";
    auto modifiedDescr = event.service;
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
