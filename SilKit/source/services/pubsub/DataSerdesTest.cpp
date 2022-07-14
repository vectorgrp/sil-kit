// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "DataSerdes.hpp"

#include <chrono>

#include "gtest/gtest.h"

using namespace std::chrono_literals;

namespace SilKit {
namespace Services {
namespace PubSub {
    bool operator==(const WireDataMessageEvent& lhs, const WireDataMessageEvent& rhs)
    {
        return Util::ItemsAreEqual(lhs.data, rhs.data)
            && lhs.timestamp == rhs.timestamp;
    }
}
}
}
TEST(MwVAsioSerdes, SimData_LargeDataMessage)
{
    using namespace SilKit::Services::PubSub;
    using namespace SilKit::Core;

    const std::vector<uint8_t> referenceData(114'793, 'D');
    SilKit::Core::MessageBuffer buffer;
    SilKit::Services::PubSub::WireDataMessageEvent in, out;
    in.data = {1,2,3,4};
    in.timestamp = 0xabcdefns;

    Serialize(buffer,  in);
    Deserialize(buffer, out);

    EXPECT_EQ(in, out);
}

