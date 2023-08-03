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

#include "LinSerdes.hpp"

#include <chrono>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace std::chrono_literals;

TEST(Test_LinSerdes, SimLin_Frame)
{
    using namespace SilKit::Services::Lin;
    SilKit::Core::MessageBuffer buffer;

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

TEST(Test_LinSerdes, SimLin_SendFrameRequest)
{
    using namespace SilKit::Services::Lin;
    SilKit::Core::MessageBuffer buffer;

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

TEST(Test_LinSerdes, SimLin_SendFrameHeaderRequest)
{
    using namespace SilKit::Services::Lin;
    SilKit::Core::MessageBuffer buffer;

    LinSendFrameHeaderRequest in;
    LinSendFrameHeaderRequest out;

    in.timestamp = 13ns;
    in.id = 49;

    Serialize(buffer , in);
    Deserialize(buffer , out);

    EXPECT_EQ(in, out);
}
TEST(Test_LinSerdes, SimLin_Transmission)
{
    using namespace SilKit::Services::Lin;
    SilKit::Core::MessageBuffer buffer;

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

TEST(Test_LinSerdes, SimLin_WakeupPulse)
{
    using namespace SilKit::Services::Lin;
    SilKit::Core::MessageBuffer buffer;

    LinWakeupPulse in;
    LinWakeupPulse out;

    in.timestamp = 13ns;

    Serialize(buffer , in);
    Deserialize(buffer , out);

    EXPECT_EQ(in, out);
}

TEST(Test_LinSerdes, SimLin_FrameResponse)
{
    using namespace SilKit::Services::Lin;
    SilKit::Core::MessageBuffer buffer;

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

TEST(Test_LinSerdes, SimLin_ControllerConfig)
{
    using namespace SilKit::Services::Lin;
    SilKit::Core::MessageBuffer buffer;

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

    WireLinControllerConfig in;
    WireLinControllerConfig out;
    in.controllerMode = LinControllerMode::Slave;
    in.baudRate = 1235345;
    in.frameResponses.push_back(response1);
    in.frameResponses.push_back(response2);
    in.simulationMode = WireLinControllerConfig::SimulationMode::Dynamic;

    Serialize(buffer , in);
    Deserialize(buffer , out);

    EXPECT_EQ(in, out);
}

TEST(Test_LinSerdes, SimLin_ControllerStatusUpdate)
{
    using namespace SilKit::Services::Lin;
    SilKit::Core::MessageBuffer buffer;

    LinControllerStatusUpdate in;
    LinControllerStatusUpdate out;

    in.timestamp = 10s;
    in.status = LinControllerStatus::Sleep;

    Serialize(buffer , in);
    Deserialize(buffer , out);

    EXPECT_EQ(in, out);
}

TEST(Test_LinSerdes, SimLin_FrameResponseUpdate)
{
    using namespace SilKit::Services::Lin;
    SilKit::Core::MessageBuffer buffer;

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
