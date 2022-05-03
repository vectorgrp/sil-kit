// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SerdesSimLin.hpp"

#include <chrono>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace std::chrono_literals;

TEST(MwVAsioSerdes, SimLin_Frame)
{
    using namespace ib::sim::lin;
    ib::mw::MessageBuffer buffer;

    LinFrame in;
    LinFrame out;

    in.id = 7;
    in.checksumModel = ChecksumModel::Classic;
    in.dataLength = 6;
    in.data = std::array<uint8_t, 8>{'V', 'E', 'C', 'T', 'O', 'R', 0, 0};

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in, out);
}

TEST(MwVAsioSerdes, SimLin_SendFrameRequest)
{
    using namespace ib::sim::lin;
    ib::mw::MessageBuffer buffer;

    SendFrameRequest in;
    SendFrameRequest out;

    in.frame.id = 19;
    in.frame.checksumModel = ChecksumModel::Enhanced;
    in.frame.dataLength = 8;
    in.frame.data = std::array<uint8_t, 8>{'A', 2, 3, 6, 'c', 'Z', 'K'};
    in.responseType = FrameResponseType::SlaveToSlave;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in, out);
}

TEST(MwVAsioSerdes, SimLin_SendFrameHeaderRequest)
{
    using namespace ib::sim::lin;
    ib::mw::MessageBuffer buffer;

    SendFrameHeaderRequest in;
    SendFrameHeaderRequest out;

    in.id = 49;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in, out);
}
TEST(MwVAsioSerdes, SimLin_Transmission)
{
    using namespace ib::sim::lin;
    ib::mw::MessageBuffer buffer;

    Transmission in;
    Transmission out;

    in.timestamp = 13ns;
    in.frame.id = 19;
    in.frame.checksumModel = ChecksumModel::Enhanced;
    in.frame.dataLength = 8;
    in.frame.data = std::array<uint8_t, 8>{'A', 2, 3, 6, 'c', 'Z', 'K'};
    in.status = FrameStatus::LIN_TX_OK;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in, out);
}

TEST(MwVAsioSerdes, SimLin_WakeupPulse)
{
    using namespace ib::sim::lin;
    ib::mw::MessageBuffer buffer;

    WakeupPulse in;
    WakeupPulse out;

    in.timestamp = 13ns;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in, out);
}

TEST(MwVAsioSerdes, SimLin_FrameResponse)
{
    using namespace ib::sim::lin;
    ib::mw::MessageBuffer buffer;

    FrameResponse in;
    FrameResponse out;

    in.frame.id = 50;
    in.frame.checksumModel = ChecksumModel::Enhanced;
    in.frame.dataLength = 2;
    in.frame.data = std::array<uint8_t, 8>{'A', 'B', 'E', 'c', 'A', 'B', 'E', 'c'};
    in.responseMode = FrameResponseMode::TxUnconditional;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in, out);
}

TEST(MwVAsioSerdes, SimLin_ControllerConfig)
{
    using namespace ib::sim::lin;
    ib::mw::MessageBuffer buffer;

    FrameResponse response1;
    response1.frame.id = 50;
    response1.frame.checksumModel = ChecksumModel::Enhanced;
    response1.frame.dataLength = 2;
    response1.frame.data = std::array<uint8_t, 8>{'A', 'B', 'E', 'c', 'A', 'B', 'E', 'c'};
    response1.responseMode = FrameResponseMode::TxUnconditional;
    FrameResponse response2;
    response1.frame.id = 36;
    response1.frame.checksumModel = ChecksumModel::Classic;
    response1.responseMode = FrameResponseMode::Rx;


    ControllerConfig in;
    ControllerConfig out;
    in.controllerMode = ControllerMode::Slave;
    in.baudRate = 1235345;
    in.frameResponses.push_back(response1);
    in.frameResponses.push_back(response2);

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in, out);
}

TEST(MwVAsioSerdes, SimLin_ControllerStatusUpdate)
{
    using namespace ib::sim::lin;
    ib::mw::MessageBuffer buffer;

    ControllerStatusUpdate in;
    ControllerStatusUpdate out;

    in.timestamp = 10s;
    in.status = ControllerStatus::Sleep;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in, out);
}

TEST(MwVAsioSerdes, SimLin_FrameResponseUpdate)
{
    using namespace ib::sim::lin;
    ib::mw::MessageBuffer buffer;

    FrameResponse response1;
    response1.frame.id = 50;
    response1.frame.checksumModel = ChecksumModel::Enhanced;
    response1.frame.dataLength = 2;
    response1.frame.data = std::array<uint8_t, 8>{'A', 'B', 'E', 'c', 'A', 'B', 'E', 'c'};
    response1.responseMode = FrameResponseMode::TxUnconditional;
    FrameResponse response2;
    response1.frame.id = 36;
    response1.frame.checksumModel = ChecksumModel::Classic;
    response1.responseMode = FrameResponseMode::Rx;


    FrameResponseUpdate in;
    FrameResponseUpdate out;

    in.frameResponses.push_back(response1);
    in.frameResponses.push_back(response2);

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in, out);
}