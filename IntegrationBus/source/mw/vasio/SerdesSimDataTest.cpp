// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SerdesSimData.hpp"
#include "MessageBuffer.hpp"
#include "VAsioMsgKind.hpp"

#include <chrono>

#include "gtest/gtest.h"

using namespace std::chrono_literals;

namespace ib {
namespace sim {
namespace data {
    bool operator==(const DataMessageEvent& lhs, const DataMessageEvent& rhs)
    {
        return lhs.data  ==rhs.data
            && lhs.timestamp == rhs.timestamp;
    }
}
}
}
TEST(MwVAsioSerdes, SimData_LargeDataMessage)
{
    using namespace ib::sim::data;
    using namespace ib::mw;

    const std::vector<uint8_t> referenceData(114'793, 'D');
    ib::mw::MessageBuffer buffer;
    ib::sim::data::DataMessageEvent in, out;
    in.data = {1,2,3,4};
    in.timestamp = 0xabcdefns;
        
    buffer<<  in;
    buffer>> out;

    EXPECT_EQ(in, out);
}

