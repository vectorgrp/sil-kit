#include "ib/sim/eth/EthDatatypes.hpp"

#include "gtest/gtest.h"

namespace {


using namespace ib::sim::eth;

/*!  \brief Make sure that by construction the timestamp have a sane default value
*/
TEST(EthDatatypeTest, timestamp_default)
{
    using tstype = decltype(EthMessage::timestamp);

    EthMessage msg{};
    ASSERT_EQ(msg.timestamp, (std::chrono::nanoseconds::min)());

    EthMessage msg2;
    ASSERT_EQ(msg2.timestamp, (std::chrono::nanoseconds::min)());
}

}//end anonymous namespace
