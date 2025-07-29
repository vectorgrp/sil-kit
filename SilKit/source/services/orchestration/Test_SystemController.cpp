// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "SystemController.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "silkit/services/orchestration/string_utils.hpp"

#include "MockParticipant.hpp"
#include "SyncDatatypeUtils.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace SilKit;
using namespace SilKit::Core;
using namespace SilKit::Services::Orchestration;
using namespace SilKit::Util;

using ::SilKit::Core::Tests::DummyParticipant;

class MockParticipant : public DummyParticipant
{
public:
    MOCK_METHOD2(SendMsg, void(const IServiceEndpoint*, const SystemCommand& msg));
};

class Test_SystemController : public testing::Test
{
protected:
    Test_SystemController()
        : controller{&participant}
    {
        controller.SetServiceDescriptor(descriptor);
    }


protected:
    // ----------------------------------------
    // Helper Methods

protected:
    // ----------------------------------------
    // Members
    ServiceDescriptor descriptor{"P1", "N1", "C1", 1026};

    MockParticipant participant;
    SystemController controller;
};

TEST_F(Test_SystemController, configure_serviceDescriptor)
{
    EXPECT_EQ(controller.GetServiceDescriptor(), descriptor);
}
} // anonymous namespace
