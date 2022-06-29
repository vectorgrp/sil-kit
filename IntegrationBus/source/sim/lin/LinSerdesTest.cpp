// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "LinSerdes.hpp"

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
    in.checksumModel = LinChecksumModel::Classic;
    in.dataLength = 6;
    in.data = std::array<uint8_t, 8>{'V', 'E', 'C', 'T', 'O', 'R', 0, 0};

    Serialize(buffer , in);
    Deserialize(buffer , out);

    EXPECT_EQ(in, out);
}

TEST(MwVAsioSerdes, SimLin_SendFrameRequest)
{
    using namespace ib::sim::lin;
    ib::mw::MessageBuffer buffer;

    LinSendFrameRequest in;
    LinSendFrameRequest out;

    in.frame.id = 19;
    in.frame.checksumModel = LinChecksumModel::Enhanced;
    in.frame.dataLength = 8;
    in.frame.data = std::array<uint8_t, 8>{'A', 2, 3, 6, 'c', 'Z', 'K'};
    in.responseType = LinFrameResponseType::SlaveToSlave;

    Serialize(buffer , in);
    Deserialize(buffer , out);

    EXPECT_EQ(in, out);
}

TEST(MwVAsioSerdes, SimLin_SendFrameHeaderRequest)
{
    using namespace ib::sim::lin;
    ib::mw::MessageBuffer buffer;

    LinSendFrameHeaderRequest in;
    LinSendFrameHeaderRequest out;

    in.id = 49;

    Serialize(buffer , in);
    Deserialize(buffer , out);

    EXPECT_EQ(in, out);
}
TEST(MwVAsioSerdes, SimLin_Transmission)
{
    using namespace ib::sim::lin;
    ib::mw::MessageBuffer buffer;

    LinTransmission in;
    LinTransmission out;

    in.timestamp = 13ns;
    in.frame.id = 19;
    in.frame.checksumModel = LinChecksumModel::Enhanced;
    in.frame.dataLength = 8;
    in.frame.data = std::array<uint8_t, 8>{'A', 2, 3, 6, 'c', 'Z', 'K'};
    in.status = LinFrameStatus::LIN_TX_OK;

    Serialize(buffer , in);
    Deserialize(buffer , out);

    EXPECT_EQ(in, out);
}

TEST(MwVAsioSerdes, SimLin_WakeupPulse)
{
    using namespace ib::sim::lin;
    ib::mw::MessageBuffer buffer;

    LinWakeupPulse in;
    LinWakeupPulse out;

    in.timestamp = 13ns;

    Serialize(buffer , in);
    Deserialize(buffer , out);

    EXPECT_EQ(in, out);
}

TEST(MwVAsioSerdes, SimLin_FrameResponse)
{
    using namespace ib::sim::lin;
    ib::mw::MessageBuffer buffer;

    LinFrameResponse in;
    LinFrameResponse out;

    in.frame.id = 50;
    in.frame.checksumModel = LinChecksumModel::Enhanced;
    in.frame.dataLength = 2;
    in.frame.data = std::array<uint8_t, 8>{'A', 'B', 'E', 'c', 'A', 'B', 'E', 'c'};
    in.responseMode = LinFrameResponseMode::TxUnconditional;

    Serialize(buffer , in);
    Deserialize(buffer , out);

    EXPECT_EQ(in, out);
}

TEST(MwVAsioSerdes, SimLin_ControllerConfig)
{
    using namespace ib::sim::lin;
    ib::mw::MessageBuffer buffer;

    LinFrameResponse response1;
    response1.frame.id = 50;
    response1.frame.checksumModel = LinChecksumModel::Enhanced;
    response1.frame.dataLength = 2;
    response1.frame.data = std::array<uint8_t, 8>{'A', 'B', 'E', 'c', 'A', 'B', 'E', 'c'};
    response1.responseMode = LinFrameResponseMode::TxUnconditional;
    LinFrameResponse response2;
    response1.frame.id = 36;
    response1.frame.checksumModel = LinChecksumModel::Classic;
    response1.responseMode = LinFrameResponseMode::Rx;

    LinControllerConfig in;
    LinControllerConfig out;
    in.controllerMode = LinControllerMode::Slave;
    in.baudRate = 1235345;
    in.frameResponses.push_back(response1);
    in.frameResponses.push_back(response2);

    Serialize(buffer , in);
    Deserialize(buffer , out);

    EXPECT_EQ(in, out);
}

TEST(MwVAsioSerdes, SimLin_ControllerStatusUpdate)
{
    using namespace ib::sim::lin;
    ib::mw::MessageBuffer buffer;

    LinControllerStatusUpdate in;
    LinControllerStatusUpdate out;

    in.timestamp = 10s;
    in.status = LinControllerStatus::Sleep;

    Serialize(buffer , in);
    Deserialize(buffer , out);

    EXPECT_EQ(in, out);
}

TEST(MwVAsioSerdes, SimLin_FrameResponseUpdate)
{
    using namespace ib::sim::lin;
    ib::mw::MessageBuffer buffer;

    LinFrameResponse response1;
    response1.frame.id = 50;
    response1.frame.checksumModel = LinChecksumModel::Enhanced;
    response1.frame.dataLength = 2;
    response1.frame.data = std::array<uint8_t, 8>{'A', 'B', 'E', 'c', 'A', 'B', 'E', 'c'};
    response1.responseMode = LinFrameResponseMode::TxUnconditional;
    LinFrameResponse response2;
    response1.frame.id = 36;
    response1.frame.checksumModel = LinChecksumModel::Classic;
    response1.responseMode = LinFrameResponseMode::Rx;

    LinFrameResponseUpdate in;
    LinFrameResponseUpdate out;

    in.frameResponses.push_back(response1);
    in.frameResponses.push_back(response2);

    Serialize(buffer , in);
    Deserialize(buffer , out);

    EXPECT_EQ(in, out);
}