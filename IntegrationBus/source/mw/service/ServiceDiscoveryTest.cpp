// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ServiceDiscovery.hpp"

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

class MockServiceId : public IIbServiceEndpoint
{
public:
    ServiceId serviceId;
    MockServiceId(EndpointAddress ea)
    {
        serviceId.legacyEpa = ea;
    }
    void SetServiceId(const ServiceId& _serviceId) override
    {
        serviceId = _serviceId;
    }
    auto GetServiceId() const -> const ServiceId & override
    {
        return serviceId;
    }

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

    MockComAdapter comAdapter;
};

TEST_F(DiscoveryServiceTest, service_creation_notification)
{
//TODO
}


} // anonymous namespace for test
