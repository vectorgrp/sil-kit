// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SerdesSimCan.hpp"

#include <chrono>

#include "gtest/gtest.h"

using namespace std::chrono_literals;

TEST(MwVAsioSerdes, SimCan_CanMessage)
{
    using namespace ib::sim::can;
    ib::mw::MessageBuffer buffer;

    CanMessage in;
    CanMessage out;

    std::string payload{"TEST"};
    in.transmitId = 5;
    in.timestamp = 13ns;
    in.canId = 7;
    in.flags.ide = 1;
    in.flags.rtr = 0;
    in.flags.fdf = 1;
    in.flags.brs = 0;
    in.flags.esi = 1;
    in.dlc = 5;
    in.dataField = std::vector<uint8_t>{payload.begin(), payload.end()};

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.transmitId, out.transmitId);
    EXPECT_EQ(in.timestamp, out.timestamp);
    EXPECT_EQ(in.canId, out.canId);
    EXPECT_EQ(in.flags.ide, out.flags.ide);
    EXPECT_EQ(in.flags.rtr, out.flags.rtr);
    EXPECT_EQ(in.flags.fdf, out.flags.fdf);
    EXPECT_EQ(in.flags.brs, out.flags.brs);
    EXPECT_EQ(in.flags.esi, out.flags.esi);
    EXPECT_EQ(in.dlc, out.dlc);
    EXPECT_EQ(in.dataField, out.dataField);
}

TEST(MwVAsioSerdes, SimCan_CanTransmitAcknowledge)
{
    using namespace ib::sim::can;
    ib::mw::MessageBuffer buffer;

    CanTransmitAcknowledge in;
    CanTransmitAcknowledge out;

    in.transmitId = 5;
    in.timestamp = 13ns;
    in.status = CanTransmitStatus::Transmitted;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.transmitId, out.transmitId);
    EXPECT_EQ(in.timestamp, out.timestamp);
    EXPECT_EQ(in.status, out.status);
}

TEST(MwVAsioSerdes, SimCan_CanControllerStatus)
{
    using namespace ib::sim::can;
    ib::mw::MessageBuffer buffer;

    CanControllerStatus in;
    CanControllerStatus out;

    in.timestamp = 13ns;
    in.controllerState = CanControllerState::Started;
    in.errorState = CanErrorState::ErrorActive;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.timestamp, out.timestamp);
    EXPECT_EQ(in.controllerState, out.controllerState);
    EXPECT_EQ(in.errorState, out.errorState);
}

TEST(MwVAsioSerdes, SimCan_CanConfigureBaudrate)
{
    using namespace ib::sim::can;
    ib::mw::MessageBuffer buffer;

    CanConfigureBaudrate in;
    CanConfigureBaudrate out;

    in.baudRate = 123;
    in.fdBaudRate = 4294967295;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.baudRate, out.baudRate);
    EXPECT_EQ(in.fdBaudRate, out.fdBaudRate);
}

TEST(MwVAsioSerdes, SimCan_CanSetControllerMode)
{
    using namespace ib::sim::can;
    ib::mw::MessageBuffer buffer;

    CanSetControllerMode in;
    CanSetControllerMode out;

    in.flags.resetErrorHandling = 1;
    in.flags.cancelTransmitRequests = 0;
    in.mode = CanControllerState::Started;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.flags.resetErrorHandling, out.flags.resetErrorHandling);
    EXPECT_EQ(in.flags.cancelTransmitRequests, out.flags.cancelTransmitRequests);
    EXPECT_EQ(in.mode, out.mode);
}
