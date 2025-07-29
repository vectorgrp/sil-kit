// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "CanSerdes.hpp"

#include <chrono>

#include "gtest/gtest.h"

using namespace std::chrono_literals;

TEST(Test_CanSerdes, SimCan_CanMessage)
{
    using namespace SilKit::Services::Can;
    SilKit::Core::MessageBuffer buffer;

    WireCanFrameEvent in{};
    WireCanFrameEvent out;

    std::string payload{"TEST"};
    in.timestamp = 13ns;
    in.frame.canId = 7;
    in.frame.flags |=
        static_cast<CanFrameFlagMask>(CanFrameFlag::Ide) | static_cast<CanFrameFlagMask>(CanFrameFlag::Fdf)
        | static_cast<CanFrameFlagMask>(CanFrameFlag::Esi) | static_cast<CanFrameFlagMask>(CanFrameFlag::Sec)
        | static_cast<CanFrameFlagMask>(CanFrameFlag::Xlf);
    in.frame.dlc = 5;
    in.frame.sdt = 123;
    in.frame.vcid = 89;
    in.frame.af = 0xCAFECAFE;
    in.frame.dataField = std::vector<uint8_t>{payload.begin(), payload.end()};
    in.userContext = (void*)((size_t)0xcafecafe);

    Serialize(buffer, in);
    Deserialize(buffer, out);

    EXPECT_EQ(in.timestamp, out.timestamp);
    EXPECT_EQ(in.frame.canId, out.frame.canId);
    EXPECT_EQ(in.frame.flags, out.frame.flags);
    EXPECT_EQ(in.frame.dlc, out.frame.dlc);
    EXPECT_EQ(in.frame.sdt, out.frame.sdt);
    EXPECT_EQ(in.frame.vcid, out.frame.vcid);
    EXPECT_EQ(in.frame.af, out.frame.af);
    EXPECT_TRUE(SilKit::Util::ItemsAreEqual(in.frame.dataField, out.frame.dataField));
    EXPECT_EQ(in.userContext, out.userContext);
}

TEST(Test_CanSerdes, SimCan_CanTransmitAcknowledge)
{
    using namespace SilKit::Services::Can;
    SilKit::Core::MessageBuffer buffer;

    CanFrameTransmitEvent in;
    CanFrameTransmitEvent out;

    in.timestamp = 13ns;
    in.status = CanTransmitStatus::Transmitted;
    in.userContext = (void*)((size_t)0xcafecafe);

    Serialize(buffer, in);
    Deserialize(buffer, out);

    EXPECT_EQ(in.timestamp, out.timestamp);
    EXPECT_EQ(in.status, out.status);
    EXPECT_EQ(in.userContext, out.userContext);
}

TEST(Test_CanSerdes, SimCan_CanControllerStatus)
{
    using namespace SilKit::Services::Can;
    SilKit::Core::MessageBuffer buffer;

    CanControllerStatus in;
    CanControllerStatus out;

    in.timestamp = 13ns;
    in.controllerState = CanControllerState::Started;
    in.errorState = CanErrorState::ErrorActive;

    Serialize(buffer, in);
    Deserialize(buffer, out);

    EXPECT_EQ(in.timestamp, out.timestamp);
    EXPECT_EQ(in.controllerState, out.controllerState);
    EXPECT_EQ(in.errorState, out.errorState);
}

TEST(Test_CanSerdes, SimCan_CanConfigureBaudrate)
{
    using namespace SilKit::Services::Can;
    SilKit::Core::MessageBuffer buffer;

    CanConfigureBaudrate in;
    CanConfigureBaudrate out;

    in.baudRate = 123;
    in.fdBaudRate = 4294967295;

    Serialize(buffer, in);
    Deserialize(buffer, out);

    EXPECT_EQ(in.baudRate, out.baudRate);
    EXPECT_EQ(in.fdBaudRate, out.fdBaudRate);
}

TEST(Test_CanSerdes, SimCan_CanSetControllerMode)
{
    using namespace SilKit::Services::Can;
    SilKit::Core::MessageBuffer buffer;

    CanSetControllerMode in;
    CanSetControllerMode out;

    in.flags.resetErrorHandling = 1;
    in.flags.cancelTransmitRequests = 0;
    in.mode = CanControllerState::Started;

    Serialize(buffer, in);
    Deserialize(buffer, out);

    EXPECT_EQ(in.flags.resetErrorHandling, out.flags.resetErrorHandling);
    EXPECT_EQ(in.flags.cancelTransmitRequests, out.flags.cancelTransmitRequests);
    EXPECT_EQ(in.mode, out.mode);
}
