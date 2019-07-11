// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SerdesSimLin.hpp"

#include <chrono>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace std::chrono_literals;

TEST(MwVAsioSerdes, SimLin_LinMessage)
{
    using namespace ib::sim::lin;
    ib::mw::MessageBuffer buffer;

    LinMessage in;
    LinMessage out;

    std::array<uint8_t, 8> data{ 'A', 2, 3, 6, 'c' };

    in.status = MessageStatus::RxSuccess;
    in.timestamp = 13ns;

    in.linId = 7;
    in.payload.size = 5;
    in.payload.data = data;
    in.checksumModel = ChecksumModel::Classic;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.status, out.status);
    EXPECT_EQ(in.timestamp, out.timestamp);
    EXPECT_EQ(in.linId, out.linId);
    EXPECT_EQ(in.payload, out.payload);
    EXPECT_EQ(in.checksumModel, out.checksumModel);
}

TEST(MwVAsioSerdes, SimLin_RxRequest)
{
    using namespace ib::sim::lin;
    ib::mw::MessageBuffer buffer;

    RxRequest in;
    RxRequest out;

    in.linId = 3;
    in.payloadLength = 7;
    in.checksumModel = ChecksumModel::Classic;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.linId, out.linId);
    EXPECT_EQ(in.payloadLength, out.payloadLength);
    EXPECT_EQ(in.checksumModel, out.checksumModel);
}

TEST(MwVAsioSerdes, SimLin_TxAcknowledge)
{
    using namespace ib::sim::lin;
    ib::mw::MessageBuffer buffer;

    TxAcknowledge in;
    TxAcknowledge out;

    in.timestamp = 13ns;
    in.linId = 3;
    in.status = MessageStatus::TxResponseError;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.timestamp, out.timestamp);
    EXPECT_EQ(in.linId, out.linId);
    EXPECT_EQ(in.status, out.status);
}

TEST(MwVAsioSerdes, SimLin_WakeupRequest)
{
    using namespace ib::sim::lin;
    ib::mw::MessageBuffer buffer;

    WakeupRequest in;
    WakeupRequest out;

    in.timestamp = 13ns;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.timestamp, out.timestamp);
}

TEST(MwVAsioSerdes, SimLin_ControllerConfig)
{
    using namespace ib::sim::lin;
    ib::mw::MessageBuffer buffer;

    ControllerConfig in;
    ControllerConfig out;

    in.controllerMode = ControllerMode::Slave;
    in.baudrate = 1235345;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.controllerMode, out.controllerMode);
    EXPECT_EQ(in.baudrate, out.baudrate);
}

TEST(MwVAsioSerdes, SimLin_SlaveConfiguration)
{
    using namespace ib::sim::lin;
    ib::mw::MessageBuffer buffer;

    SlaveConfiguration in;
    SlaveConfiguration out;

    SlaveResponseConfig conf1;
    conf1.linId = 5;
    conf1.responseMode = ResponseMode::TxUnconditional;
    conf1.checksumModel = ChecksumModel::Undefined;
    conf1.payloadLength = 5;

    SlaveResponseConfig conf2;
    conf2.linId = 2;
    conf2.responseMode = ResponseMode::Rx;
    conf2.checksumModel = ChecksumModel::Classic;
    conf2.payloadLength = 2;

    std::vector<SlaveResponseConfig> configs{conf1, conf2};

    in.responseConfigs = configs;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.responseConfigs.size(), out.responseConfigs.size());

    for (uint8_t i = 0; i < in.responseConfigs.size(); i++)
    {
        EXPECT_EQ(in.responseConfigs[i].linId, out.responseConfigs[i].linId);
        EXPECT_EQ(in.responseConfigs[i].responseMode, out.responseConfigs[i].responseMode);
        EXPECT_EQ(in.responseConfigs[i].checksumModel, out.responseConfigs[i].checksumModel);
        EXPECT_EQ(in.responseConfigs[i].payloadLength, out.responseConfigs[i].payloadLength);
    }
}

TEST(MwVAsioSerdes, SimLin_SlaveResponse)
{
    using namespace ib::sim::lin;
    ib::mw::MessageBuffer buffer;

    SlaveResponse in;
    SlaveResponse out;

    std::array<uint8_t, 8> data{'A', 2, 3};

    in.linId = 10;
    in.payload.size = 3;
    in.payload.data = data;
    in.checksumModel = ChecksumModel::Undefined;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.linId, out.linId);
    EXPECT_EQ(in.payload, out.payload);
    EXPECT_EQ(in.checksumModel, out.checksumModel);
}