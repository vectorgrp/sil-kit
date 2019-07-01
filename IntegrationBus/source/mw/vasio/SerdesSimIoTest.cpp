// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SerdesSimIo.hpp"

#include <chrono>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace std::chrono_literals;

TEST(MwVAsioSerdes, SimEthernet_AnalogIoMessage)
{
    using namespace ib::sim::io;
    ib::mw::MessageBuffer buffer;

    AnalogIoMessage in;
    AnalogIoMessage out;

    in.timestamp = 13ns;
    in.value = 5.2;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.timestamp, out.timestamp);
    EXPECT_EQ(in.value, out.value);
}

TEST(MwVAsioSerdes, SimEthernet_DigitalIoMessage)
{
    using namespace ib::sim::io;
    ib::mw::MessageBuffer buffer;

    DigitalIoMessage in;
    DigitalIoMessage out;

    in.timestamp = 13ns;
    in.value = true;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.timestamp, out.timestamp);
    EXPECT_EQ(in.value, out.value);
}

TEST(MwVAsioSerdes, SimEthernet_PatternIoMessage)
{
    using namespace ib::sim::io;
    ib::mw::MessageBuffer buffer;

    PatternIoMessage in;
    PatternIoMessage out;

    in.timestamp = 13ns;
    in.value = std::vector<uint8_t>{1, 2, 0, 'A'};

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.timestamp, out.timestamp);
    EXPECT_EQ(in.value, out.value);
}

TEST(MwVAsioSerdes, SimEthernet_PwmIoMessage)
{
    using namespace ib::sim::io;
    ib::mw::MessageBuffer buffer;

    PwmIoMessage in;
    PwmIoMessage out;

    in.timestamp = 13ns;
    in.value.frequency = 9000000200100000;
    in.value.dutyCycle = 2.8;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in.timestamp, out.timestamp);
    EXPECT_EQ(in.value.frequency, out.value.frequency);
    EXPECT_EQ(in.value.dutyCycle, out.value.dutyCycle);
}