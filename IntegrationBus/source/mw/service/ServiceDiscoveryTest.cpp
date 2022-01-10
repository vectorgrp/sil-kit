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
        (ServiceDiscovery::Type, const ServiceDescriptor&));
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
    ServiceDescriptor senderDescriptor;
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
    ServiceAnnouncement announce;
    announce.participantName = senderDescriptor.participantName;
    announce.services.push_back(descr);
    //Notify should publish a message
    EXPECT_CALL(comAdapter, SendIbMessage(&disco, announce)).Times(1);
    // callbacks on receive
    EXPECT_CALL(callbacks, ServiceDiscoveryHandler(ServiceDiscovery::Type::ServiceCreated,
        descr)).Times(1);
    // trigger notification on the same participant
    disco.NotifyServiceCreated(descr);

    // trigger notifications on reception path from different participant
    MockServiceDescriptor otherParticipant{ {1, 2} };
    announce.participantName = otherParticipant.serviceDescriptor.participantName;
    disco.ReceiveIbMessage(&otherParticipant, announce);

    announce.participantName = otherParticipant.serviceDescriptor.participantName;
    disco.ReceiveIbMessage(&otherParticipant, announce);//should not trigger callback, is cached
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

    ServiceAnnouncement announce;
    announce.participantName = senderDescriptor.participantName;

    auto sendAnnounce = [&](auto&& serviceName) {
        ServiceDescriptor descr;
        descr = senderDescriptor;
        descr.serviceName = serviceName;
        // Ensure we only append new services
        announce.services.push_back(descr);

        // Expect that each service is only handled by a single notification handler 
        // e.g., no duplicate notifications
        EXPECT_CALL(callbacks,
            ServiceDiscoveryHandler(ServiceDiscovery::Type::ServiceCreated, descr)
        ).Times(1);

        disco.ReceiveIbMessage(&otherParticipant, announce);
        disco.ReceiveIbMessage(&otherParticipant, announce);//duplicate should not trigger a notification

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

    ServiceAnnouncement announce;
    announce.participantName = senderDescriptor.participantName;

    ServiceDescriptor descr;
    descr = senderDescriptor;
    descr.serviceName = "TestService";
    announce.services.push_back(descr);

    // Test addition
    EXPECT_CALL(callbacks,
        ServiceDiscoveryHandler(ServiceDiscovery::Type::ServiceCreated, descr)
    ).Times(1);
    disco.ReceiveIbMessage(&otherParticipant, announce);

    // Test modification
    announce.services.at(0).serviceName = "Modified";
    auto modifiedDescr = announce.services.at(0);
    EXPECT_CALL(callbacks,
        ServiceDiscoveryHandler(ServiceDiscovery::Type::ServiceCreated, modifiedDescr)
    ).Times(1);
    EXPECT_CALL(callbacks,
        ServiceDiscoveryHandler(ServiceDiscovery::Type::ServiceRemoved, descr)
    ).Times(1);
    disco.ReceiveIbMessage(&otherParticipant, announce);

    // Test removal
    announce.services.clear();
    EXPECT_CALL(callbacks,
        ServiceDiscoveryHandler(ServiceDiscovery::Type::ServiceRemoved, modifiedDescr)
    ).Times(1);
    disco.ReceiveIbMessage(&otherParticipant, announce);

    // Nothing to remove, no triggers
    EXPECT_CALL(callbacks,
        ServiceDiscoveryHandler(_, _)
    ).Times(0);
    disco.ReceiveIbMessage(&otherParticipant, announce);
}

} // anonymous namespace for test
