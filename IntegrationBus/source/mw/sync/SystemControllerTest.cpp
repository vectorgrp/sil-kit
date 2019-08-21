// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SystemController.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ib/mw/sync/string_utils.hpp"

#include "MockComAdapter.hpp"
#include "SyncDatatypeUtils.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace ib;
using namespace ib::mw;
using namespace ib::mw::sync;
using namespace ib::util;

using ::ib::mw::test::DummyComAdapter;

class MockComAdapter : public DummyComAdapter
{
public:
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const ParticipantCommand& msg));
    MOCK_METHOD2(SendIbMessage, void(EndpointAddress, const SystemCommand& msg));
};

class SystemControllerTest : public testing::Test
{
protected:
    SystemControllerTest()
        : controller{&comAdapter}
    {
        controller.SetEndpointAddress(addr);
    }


protected:
    // ----------------------------------------
    // Helper Methods

protected:
    // ----------------------------------------
    // Members
    EndpointAddress addr{103, 1026};

    MockComAdapter comAdapter;
    SystemController controller;
};

TEST_F(SystemControllerTest, configure_endpoint_address)
{
    EXPECT_EQ(controller.EndpointAddress(), addr);
}

TEST_F(SystemControllerTest, send_initialize)
{
    ParticipantCommand cmd{5, ParticipantCommand::Kind::Initialize};
    EXPECT_CALL(comAdapter, SendIbMessage(addr, cmd)).Times(1);
    controller.Initialize(5);
}

TEST_F(SystemControllerTest, send_reinitialize)
{
    ParticipantCommand cmd{5, ParticipantCommand::Kind::ReInitialize};
    EXPECT_CALL(comAdapter, SendIbMessage(addr, cmd)).Times(1);
    controller.ReInitialize(5);
}

TEST_F(SystemControllerTest, send_run)
{
    SystemCommand cmd{SystemCommand::Kind::Run};
    EXPECT_CALL(comAdapter, SendIbMessage(addr, cmd)).Times(1);
    controller.Run();
}
    
TEST_F(SystemControllerTest, send_stop)
{
    SystemCommand cmd{SystemCommand::Kind::Stop};
    EXPECT_CALL(comAdapter, SendIbMessage(addr, cmd)).Times(1);
    controller.Stop();
}
    
TEST_F(SystemControllerTest, send_shutdown)
{
    SystemCommand cmd{SystemCommand::Kind::Shutdown};
    EXPECT_CALL(comAdapter, SendIbMessage(addr, cmd)).Times(1);
    controller.Shutdown();
}

TEST_F(SystemControllerTest, send_preparecoldswap)
{
    SystemCommand cmd{SystemCommand::Kind::PrepareColdswap};
    EXPECT_CALL(comAdapter, SendIbMessage(addr, cmd)).Times(1);
    controller.PrepareColdswap();
}
    
TEST_F(SystemControllerTest, send_executecoldswap)
{
    SystemCommand cmd{SystemCommand::Kind::ExecuteColdswap};
    EXPECT_CALL(comAdapter, SendIbMessage(addr, cmd)).Times(1);
    controller.ExecuteColdswap();
}

} // anonymous namespace for test
