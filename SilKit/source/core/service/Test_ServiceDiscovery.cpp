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
#include <functional>
#include <set>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ServiceDiscovery.hpp"
#include "string_utils_internal.hpp"
#include "Uuid.hpp"
#include "MockParticipant.hpp"
#include "MockServiceEndpoint.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace SilKit;
using namespace SilKit::Core;
using namespace SilKit::Core::Tests;
using namespace SilKit::Core::Discovery;
using namespace SilKit::Util;

using ::SilKit::Core::Tests::DummyParticipant;

class MockParticipant : public DummyParticipant
{
public:
    MOCK_METHOD(void, SendMsg, (const IServiceEndpoint*, const ParticipantDiscoveryEvent&), (override));
    MOCK_METHOD(void, SendMsg, (const IServiceEndpoint*, const ServiceDiscoveryEvent&), (override));
};

class Callbacks
{
public:
    MOCK_METHOD(void, ServiceDiscoveryHandler,
        (ServiceDiscoveryEvent::Type, const ServiceDescriptor&));
};
class Test_ServiceDiscovery : public testing::Test
{
protected:
    Test_ServiceDiscovery()
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

TEST_F(Test_ServiceDiscovery, portable_hash_function)
{
    const auto numStrings = 1000;
    std::vector<std::string> testStrings;
    for (auto i = 0; i < numStrings; i++)
    {
        testStrings.push_back(to_string(Uuid::GenerateRandom()));
    }
    std::set<uint64_t> hashes;
    for (const auto& s: testStrings)
    {
        hashes.insert(SilKit::Util::Hash::Hash(s));
    }
    ASSERT_EQ(hashes.size(), testStrings.size()) << "The test strings need unique 64-bit hashes";
}

TEST_F(Test_ServiceDiscovery, service_creation_notification)
{
    ServiceDescriptor senderDescriptor{};
    senderDescriptor.SetParticipantNameAndComputeId("ParticipantA");
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
    EXPECT_CALL(participant, SendMsg(&disco, event)).Times(1);
    // NotifyServiceCreated should also trigger ourself
    EXPECT_CALL(callbacks, ServiceDiscoveryHandler(ServiceDiscoveryEvent::Type::ServiceCreated,
        descr)).Times(1);
    // trigger notification on the same participant
    disco.NotifyServiceCreated(descr);

    // trigger notifications on reception path from different participant
    MockServiceEndpoint otherParticipant{ "P1", "N1", "C1", 2 };
    descr.SetParticipantNameAndComputeId("ParticipantOther");
    event.serviceDescriptor = descr;
    EXPECT_CALL(callbacks, ServiceDiscoveryHandler(ServiceDiscoveryEvent::Type::ServiceCreated,
        descr)).Times(1);

    // when sending a different service descriptor, we expect a notification once
    disco.ReceiveMsg(&otherParticipant, event);
    disco.ReceiveMsg(&otherParticipant, event);//should not trigger callback, is cached
}
TEST_F(Test_ServiceDiscovery, multiple_service_creation_notification)
{
    MockServiceEndpoint otherParticipant{ "P1", "N1", "C1", 2 };
    ServiceDiscovery disco{ &participant, "ParticipantA" };

    disco.RegisterServiceDiscoveryHandler([this](auto type, auto&& descr) {
        callbacks.ServiceDiscoveryHandler(type, descr);
    });

    ServiceDescriptor senderDescriptor;
    senderDescriptor.SetParticipantNameAndComputeId("ParticipantA");
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

        disco.ReceiveMsg(&otherParticipant, event);
        disco.ReceiveMsg(&otherParticipant, event);//duplicate should not trigger a notification

    };

    for (auto i = 0; i < 10; i++)
    {
        sendAnnounce("Service" + std::to_string(i));
    }
}

TEST_F(Test_ServiceDiscovery, service_removal)
{
    MockServiceEndpoint otherParticipant{ "P1", "N1", "C1", 2 };
    ServiceDiscovery disco{ &participant, "ParticipantA" };

    disco.RegisterServiceDiscoveryHandler([this](auto type, auto&& descr) {
        callbacks.ServiceDiscoveryHandler(type, descr);
    });

    ServiceDescriptor senderDescriptor;
    senderDescriptor.SetParticipantNameAndComputeId("ParticipantA");
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
    disco.ReceiveMsg(&otherParticipant, event);

    // add a modified one
    event.serviceDescriptor.SetServiceName("Modified");
    auto modifiedDescr = event.serviceDescriptor;
    EXPECT_CALL(callbacks,
        ServiceDiscoveryHandler(ServiceDiscoveryEvent::Type::ServiceCreated, modifiedDescr)
    ).Times(1);
    EXPECT_CALL(callbacks,
        ServiceDiscoveryHandler(ServiceDiscoveryEvent::Type::ServiceRemoved, modifiedDescr)
    ).Times(0);
    disco.ReceiveMsg(&otherParticipant, event);
    // Test removal
    event.type = ServiceDiscoveryEvent::Type::ServiceRemoved;
    EXPECT_CALL(callbacks,
        ServiceDiscoveryHandler(ServiceDiscoveryEvent::Type::ServiceRemoved, modifiedDescr)
    ).Times(1);
    disco.ReceiveMsg(&otherParticipant, event);

    // Nothing to remove, no triggers
    event.type = ServiceDiscoveryEvent::Type::ServiceRemoved;
    EXPECT_CALL(callbacks,
        ServiceDiscoveryHandler(_, _)
    ).Times(0);
    disco.ReceiveMsg(&otherParticipant, event);
}
} // anonymous namespace for test
