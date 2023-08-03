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

#include "gtest/gtest.h"

#include "silkit/util/serdes/Serializer.hpp"
using namespace SilKit::Util::SerDes;

namespace {

template <typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
void SerializeAlignedIntegral()
{
    const T value_min = std::numeric_limits<T>::min();
    const T value_max = std::numeric_limits<T>::max();

    Serializer serializer;
    serializer.Serialize(value_min, 8 * sizeof(T));
    serializer.Serialize(value_max, 8 * sizeof(T));
    const auto buffer = serializer.ReleaseBuffer();

    // build reference
    std::vector<uint8_t> refBuffer;
    refBuffer.resize(sizeof(T) * 2);
    std::memcpy(&refBuffer[0], &value_min, sizeof(T));
    std::memcpy(&refBuffer[sizeof(T)], &value_max, sizeof(T));

    EXPECT_EQ(buffer, refBuffer);
}

TEST(Test_SilSerializer, serialize_integral)
{
    SerializeAlignedIntegral<int8_t>();
    SerializeAlignedIntegral<int16_t>();
    SerializeAlignedIntegral<int32_t>();
    SerializeAlignedIntegral<int64_t>();
}
TEST(Test_SilSerializer, serialize_integral_unsigned)
{
    SerializeAlignedIntegral<uint8_t>();
    SerializeAlignedIntegral<uint16_t>();
    SerializeAlignedIntegral<uint32_t>();
    SerializeAlignedIntegral<uint64_t>();
}

TEST(Test_SilSerializer, serialize_unaligned_4_uint2)
{
    // serialize 4 int2 values
    const uint16_t a_2 = 3;
    const uint8_t b_2 = 0;
    const uint32_t c_2 = 3;
    const uint64_t d_2 = 0;

    Serializer serializer;
    serializer.Serialize(a_2, 2);
    serializer.Serialize(b_2, 2);
    serializer.Serialize(c_2, 2);
    serializer.Serialize(d_2, 2);
    auto buffer = serializer.ReleaseBuffer();

    EXPECT_EQ(buffer.size(), 1u);

    // build reference
    const uint8_t ref = 0x33;
    EXPECT_EQ(buffer[0], ref);
}

TEST(Test_SilSerializer, serialize_unaligned_4_int2)
{
    // serialize 4 int2 values
    const int16_t a_2 = -1;
    const int8_t b_2 = 0;
    const int32_t c_2 = -1;
    const int64_t d_2 = 0;

    Serializer serializer;
    serializer.Serialize(a_2, 2);
    serializer.Serialize(b_2, 2);
    serializer.Serialize(c_2, 2);
    serializer.Serialize(d_2, 2);
    const auto buffer = serializer.ReleaseBuffer();

    EXPECT_EQ(buffer.size(), 1u);

    // build reference
    const uint8_t ref = 0x33;
    EXPECT_EQ(buffer[0], ref);
}

TEST(Test_SilSerializer, serialize_unaligned_with_overlap)
{
    // serialize an int7, followed by an int2
    // This should result in 2 bytes.
    const uint16_t a_7 = 0x14;
    const uint16_t b_2 = 3;

    Serializer serializer;
    serializer.Serialize(a_7, 7);
    serializer.Serialize(b_2, 2);
    const auto buffer = serializer.ReleaseBuffer();

    EXPECT_EQ(buffer.size(), 2u);

    // build reference
    const uint16_t ref = a_7 | (b_2 << 7);
    uint16_t raw = 0;
    std::memcpy(&raw, &buffer[0], 2);
    raw &= 0x1ff;

    EXPECT_EQ(raw, ref);
}

TEST(Test_SilSerializer, serialize_unaligned_mixed)
{
    // alternates between unaligned and aligned serialization.
    // Should end up with 4 bytes
    const uint16_t a_2 = 3;
    const uint16_t b_8 = 3;
    const uint16_t c_3 = 7;
    const uint16_t d_8 = 32;

    Serializer serializer;
    serializer.Serialize(a_2, 2);
    serializer.Serialize(b_8, 8);
    serializer.Serialize(c_3, 3);
    serializer.Serialize(d_8, 8);
    const auto buffer = serializer.ReleaseBuffer();

    ASSERT_EQ(buffer.size(), 4u);

    // build reference
    EXPECT_EQ(buffer[0] & 0x3, a_2);
    EXPECT_EQ(buffer[1] & 0xff, b_8);
    EXPECT_EQ(buffer[2] & 0x7, c_3);
    EXPECT_EQ(buffer[3] & 0xff, d_8);
}
TEST(Test_SilSerializer, serialize_bool)
{
    Serializer serializer;
    serializer.Serialize(true);
    serializer.Serialize(false);
    const auto buffer = serializer.ReleaseBuffer();

    EXPECT_EQ(buffer.size(), 2u);
    EXPECT_EQ(buffer[0], 1u);
    EXPECT_EQ(buffer[1], 0u);
}

template <typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
void SerializeFloatingPoint()
{
    const T value_neg = static_cast<T>(-13.37);
    const T value_pos = static_cast<T>(13.37);

    Serializer serializer;
    serializer.Serialize(value_neg);
    serializer.Serialize(value_pos);
    const auto buffer = serializer.ReleaseBuffer();

    // build reference
    std::vector<uint8_t> refBuffer;
    refBuffer.resize(sizeof(T) * 2);
    std::memcpy(&refBuffer[0], &value_neg, sizeof(T));
    std::memcpy(&refBuffer[sizeof(T)], &value_pos, sizeof(T));

    ASSERT_EQ(buffer.size(), refBuffer.size());
    EXPECT_EQ(buffer, refBuffer);
}
TEST(Test_SilSerializer, serialize_floatingpoint)
{
    SerializeFloatingPoint<float>();
    SerializeFloatingPoint<double>();
}

TEST(Test_SilSerializer, serialize_string)
{
    std::string value{"Hello world! I love you so much."};
    Serializer serializer;
    serializer.Serialize(value);
    const auto buffer = serializer.ReleaseBuffer();

    std::vector<uint8_t> refBuffer;
    const auto refSize = static_cast<uint32_t>(value.size());
    refBuffer.resize(4 + value.size());
    std::memcpy(&refBuffer[0], &refSize, 4);
    std::copy(value.begin(), value.end(), refBuffer.begin() + 4);

    EXPECT_EQ(buffer, refBuffer);
}

TEST(Test_SilSerializer, serialize_bytes)
{
    std::vector<uint8_t> bytes{'a', 'b', 'c', 'd'};
    Serializer serializer;
    serializer.Serialize(bytes);
    const auto buffer = serializer.ReleaseBuffer();

    std::vector<uint8_t> refBuffer;
    const auto refSize = static_cast<uint32_t>(bytes.size());
    refBuffer.resize(sizeof(refSize) + bytes.size());
    std::memcpy(&refBuffer[0], &refSize, 4);
    std::copy(bytes.begin(), bytes.end(), refBuffer.begin() + 4);

    EXPECT_EQ(buffer, refBuffer);
}

} // anonymous namespace
