// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "DataSerdes.hpp"

#include <chrono>

#include "gtest/gtest.h"

using namespace std::chrono_literals;

namespace SilKit {
namespace Services {
namespace PubSub {
static bool operator==(const WireDataMessageEvent& lhs, const WireDataMessageEvent& rhs)
{
    return Util::ItemsAreEqual(lhs.data, rhs.data) && lhs.timestamp == rhs.timestamp;
}
} // namespace PubSub
} // namespace Services
} // namespace SilKit
TEST(Test_DataSerdes, SimData_LargeDataMessage)
{
    using namespace SilKit::Services::PubSub;
    using namespace SilKit::Core;

    const std::vector<uint8_t> referenceData(114'793, 'D');
    SilKit::Core::MessageBuffer buffer;
    SilKit::Services::PubSub::WireDataMessageEvent in, out;
    in.data = {1, 2, 3, 4};
    in.timestamp = 0xabcdefns;

    Serialize(buffer, in);
    Deserialize(buffer, out);

    EXPECT_EQ(in, out);
}
