// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "MessageBuffer.hpp"

#include <vector>

#include "gtest/gtest.h"

using namespace std::chrono_literals;

TEST(MwVAsio_MessageBuffer, integral_types)
{
    ib::mw::MessageBuffer buffer;

    int32_t in{7};
    int32_t out{1};
    
    buffer << in;
    buffer >> out;

    EXPECT_EQ(in, out);
}


TEST(MwVAsio_MessageBuffer, integral_types_multiple)
{
    ib::mw::MessageBuffer buffer;

    std::vector<int32_t> in{1,2,3,4,5,6,7};
    std::vector<int32_t> out(in.size(), -1);

    for (auto i : in)
        buffer << i;

    for (auto& o : out)
        buffer >> o;
    
    EXPECT_EQ(in, out);
}

TEST(MwVAsio_MessageBuffer, floating_types)
{
    ib::mw::MessageBuffer buffer;

    double in{7};
    double out{1};

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in, out);
}

TEST(MwVAsio_MessageBuffer, floating_types_multiple)
{
    ib::mw::MessageBuffer buffer;

    std::vector<double> in{1,2,3,4,5,6,7};
    std::vector<double> out(in.size(), -1);

    for (auto i : in)
        buffer << i;

    for (auto& o : out)
        buffer >> o;

    EXPECT_EQ(in, out);
}

namespace {
enum class TestEnumT : uint8_t
{
    A,
    B,
    C,
    D
};
}

TEST(MwVAsio_MessageBuffer, enum_types)
{
    ib::mw::MessageBuffer buffer;

    TestEnumT in{TestEnumT::A};
    TestEnumT out{TestEnumT::B};

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in, out);
}

TEST(MwVAsio_MessageBuffer, std_string)
{
    ib::mw::MessageBuffer buffer;

    std::string in{"This_is_a_test_string"};
    std::string out;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in, out);
}

TEST(MwVAsio_MessageBuffer, std_vector_uint8_t)
{
    ib::mw::MessageBuffer buffer;

    std::string helperString{"This_is_a_test_string"};

    std::vector<uint8_t> in{helperString.begin(), helperString.end()};
    std::vector<uint8_t> out;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in, out);
}

TEST(MwVAsio_MessageBuffer, std_vector_string)
{
    ib::mw::MessageBuffer buffer;

    std::vector<std::string> in{"this", "is", "a", "test", "string"};
    std::vector<std::string> out;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in, out);
}

TEST(MwVAsio_MessageBuffer, std_array_uint8_t)
{
    ib::mw::MessageBuffer buffer;

    std::array<uint8_t, 8> in{ 'A', 2, 3, 4, '.'};
    std::array<uint8_t, 8> out;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in, out);
}

TEST(MwVAsio_MessageBuffer, std_array_string)
{
    ib::mw::MessageBuffer buffer;

    std::array<std::string, 6> in{"this", "is", "a", "test", "string"};
    std::array<std::string, 6> out;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in, out);
}

TEST(MwVAsio_MessageBuffer, std_chrono_nanoseconds)
{
    ib::mw::MessageBuffer buffer;

    std::chrono::nanoseconds in{1500ns};
    std::chrono::nanoseconds out;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in, out);
}

TEST(MwVAsio_MessageBuffer, std_chrono_system_time)
{
    ib::mw::MessageBuffer buffer;

    auto now = std::chrono::system_clock::now();

    decltype(now) in = std::chrono::system_clock::time_point{std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch())};
    decltype(in) out;

    buffer << in;
    buffer >> out;

    EXPECT_EQ(in, out);
}

namespace {
struct TestData {
    double doub;
    uint32_t ui32;
    int16_t i16;
    TestEnumT e;
    std::chrono::nanoseconds duration;
    std::string str;
};

bool operator==(const TestData& lhs, const TestData& rhs)
{
    return lhs.ui32 == rhs.ui32
        && lhs.i16 == rhs.i16
        && lhs.e == rhs.e
        && lhs.duration == rhs.duration
        && lhs.str == rhs.str;
}
}

TEST(MwVAsio_MessageBuffer, mixed_types)
{
    ib::mw::MessageBuffer buffer;

    TestData in{3.333, std::numeric_limits<uint32_t>::max() - 1, -13, TestEnumT::A, 17ns, "This looks nice!"};
    TestData out{0.01, 0, 0, TestEnumT::B, 0ns, std::string{}};

    buffer << in.ui32 << in.i16 << in.e << in.duration << in.str;
    buffer >> out.ui32 >> out.i16 >> out.e >> out.duration >> out.str;

    EXPECT_EQ(in, out);
}
