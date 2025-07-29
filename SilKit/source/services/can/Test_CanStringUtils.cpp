// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/services/can/string_utils.hpp"
#include "silkit/services/can/CanDatatypes.hpp"

#include <sstream>
#include <set>

#include "gtest/gtest.h"

using namespace std::chrono_literals;

TEST(Test_CanStringUtils, CanFrameFlag_to_string_and_ostream_same_result)
{
    for (uint32_t bit = 0; bit != 32; ++bit)
    {
        const auto flag = static_cast<SilKit::Services::Can::CanFrameFlag>(BIT(bit));

        std::ostringstream ss;
        ss << flag;

        EXPECT_EQ(to_string(flag), ss.str());
    }
}

TEST(Test_CanStringUtils, CanFrameFlag_known_flags_do_not_produce_unkown_string)
{
    EXPECT_EQ(to_string(SilKit::Services::Can::CanFrameFlag::Ide).find("unknown("), std::string::npos);
    EXPECT_EQ(to_string(SilKit::Services::Can::CanFrameFlag::Rtr).find("unknown("), std::string::npos);
    EXPECT_EQ(to_string(SilKit::Services::Can::CanFrameFlag::Fdf).find("unknown("), std::string::npos);
    EXPECT_EQ(to_string(SilKit::Services::Can::CanFrameFlag::Brs).find("unknown("), std::string::npos);
    EXPECT_EQ(to_string(SilKit::Services::Can::CanFrameFlag::Esi).find("unknown("), std::string::npos);
    EXPECT_EQ(to_string(SilKit::Services::Can::CanFrameFlag::Xlf).find("unknown("), std::string::npos);
    EXPECT_EQ(to_string(SilKit::Services::Can::CanFrameFlag::Sec).find("unknown("), std::string::npos);
}

TEST(Test_CanStringUtils, CanFrameFlag_unknown_flags_produce_unkown_string)
{
    using SilKit::Services::Can::CanFrameFlag;

    const auto knownFlags =
        std::set<CanFrameFlag>{CanFrameFlag::Ide, CanFrameFlag::Rtr, CanFrameFlag::Fdf, CanFrameFlag::Brs,
                               CanFrameFlag::Esi, CanFrameFlag::Xlf, CanFrameFlag::Sec};

    for (uint32_t bit = 0; bit != 32; ++bit)
    {
        const uint32_t value = BIT(bit);
        const auto flag = static_cast<CanFrameFlag>(value);

        if (knownFlags.find(flag) != knownFlags.end())
        {
            continue;
        }

        EXPECT_EQ(to_string(flag), "unknown(" + std::to_string(value) + ")");
    }
}
