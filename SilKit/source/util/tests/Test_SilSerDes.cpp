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
#include "silkit/util/serdes/Deserializer.hpp"
using namespace SilKit::Util::SerDes;

namespace {

template <typename T, typename std::enable_if_t<std::is_integral<T>::value, int> = 0>
void SerializeAlignedIntegral(Serializer& serializer)
{
    const T value_min = std::numeric_limits<T>::min();
    const T value_max = std::numeric_limits<T>::max();
    serializer.Serialize(value_min, 8 * sizeof(T));
    serializer.Serialize(value_max, 8 * sizeof(T));
}
template <typename T, typename std::enable_if_t<std::is_integral<T>::value, int> = 0>
void DeserializeAlignedIntegral(Deserializer& deserializer)
{
    const T value_min = deserializer.Deserialize<T>(8 * sizeof(T));
    const T value_max = deserializer.Deserialize<T>(8 * sizeof(T));

    EXPECT_EQ(value_min, std::numeric_limits<T>::min());
    EXPECT_EQ(value_max, std::numeric_limits<T>::max());
}

TEST(Test_SilSerDes, serdes_integral)
{
    Serializer serializer;
    SerializeAlignedIntegral<int8_t>(serializer);
    SerializeAlignedIntegral<int16_t>(serializer);
    SerializeAlignedIntegral<int32_t>(serializer);
    SerializeAlignedIntegral<int64_t>(serializer);
    Deserializer deserializer;
    deserializer.Reset(serializer.ReleaseBuffer());
    DeserializeAlignedIntegral<int8_t>(deserializer);
    DeserializeAlignedIntegral<int16_t>(deserializer);
    DeserializeAlignedIntegral<int32_t>(deserializer);
    DeserializeAlignedIntegral<int64_t>(deserializer);
}
TEST(Test_SilSerDes, serdes_integral_unsigned)
{
    Serializer serializer;
    SerializeAlignedIntegral<uint8_t>(serializer);
    SerializeAlignedIntegral<uint16_t>(serializer);
    SerializeAlignedIntegral<uint32_t>(serializer);
    SerializeAlignedIntegral<uint64_t>(serializer);
    Deserializer deserializer;
    deserializer.Reset(serializer.ReleaseBuffer());
    DeserializeAlignedIntegral<uint8_t>(deserializer);
    DeserializeAlignedIntegral<uint16_t>(deserializer);
    DeserializeAlignedIntegral<uint32_t>(deserializer);
    DeserializeAlignedIntegral<uint64_t>(deserializer);
}
TEST(Test_SilSerDes, serdes_integral_smaller_bitwidth)
{
    // 64bit value initalized with negative 16bit value
    constexpr int64_t reference = (std::numeric_limits<int16_t>::min)();

    Serializer serializer;
    serializer.Serialize(reference, 16);

    Deserializer deserializer;
    deserializer.Reset(serializer.ReleaseBuffer());
    const auto result = deserializer.Deserialize<int64_t>(16);
    EXPECT_EQ(reference, result);
}
TEST(Test_SilSerDes, serdes_unaligned_int63)
{
    // all are treated as int63
    int64_t a = -112353;
    int64_t b = -21256;
    int64_t c = 39784620;
    int64_t d = -158790;
    int64_t e = 39187;
    int64_t f = -290875;
    int64_t g = 34098712004124;
    int64_t h = -140897938744434;

    Serializer serializer;
    serializer.Serialize(a, 63);
    serializer.Serialize(b, 63);
    serializer.Serialize(c, 63);
    serializer.Serialize(d, 63);
    serializer.Serialize(e, 63);
    serializer.Serialize(f, 63);
    serializer.Serialize(g, 63);
    serializer.Serialize(h, 63);

    Deserializer deserializer;
    deserializer.Reset(serializer.ReleaseBuffer());
    EXPECT_EQ(a, deserializer.Deserialize<int64_t>(63));
    EXPECT_EQ(b, deserializer.Deserialize<int64_t>(63));
    EXPECT_EQ(c, deserializer.Deserialize<int64_t>(63));
    EXPECT_EQ(d, deserializer.Deserialize<int64_t>(63));
    EXPECT_EQ(e, deserializer.Deserialize<int64_t>(63));
    EXPECT_EQ(f, deserializer.Deserialize<int64_t>(63));
    EXPECT_EQ(g, deserializer.Deserialize<int64_t>(63));
    EXPECT_EQ(h, deserializer.Deserialize<int64_t>(63));
}

TEST(Test_SilSerDes, serdes_unaligned_int)
{
    int16_t a_3 = 3;
    int16_t b_3 = -2;
    int16_t c_3 = 3;
    int16_t d_3 = -1;
    int16_t e_3 = 3;
    int16_t f_3 = -2;
    int16_t g_3 = 3;
    int16_t h_3 = -1;

    Serializer serializer;
    serializer.Serialize(a_3, 3);
    serializer.Serialize(b_3, 3);
    serializer.Serialize(c_3, 3);
    serializer.Serialize(d_3, 3);
    serializer.Serialize(e_3, 3);
    serializer.Serialize(f_3, 3);
    serializer.Serialize(g_3, 3);
    serializer.Serialize(h_3, 3);

    Deserializer deserializer;
    deserializer.Reset(serializer.ReleaseBuffer());
    EXPECT_EQ(a_3, deserializer.Deserialize<int16_t>(3));
    EXPECT_EQ(b_3, deserializer.Deserialize<int16_t>(3));
    EXPECT_EQ(c_3, deserializer.Deserialize<int16_t>(3));
    EXPECT_EQ(d_3, deserializer.Deserialize<int16_t>(3));
    EXPECT_EQ(e_3, deserializer.Deserialize<int16_t>(3));
    EXPECT_EQ(f_3, deserializer.Deserialize<int16_t>(3));
    EXPECT_EQ(g_3, deserializer.Deserialize<int16_t>(3));
    EXPECT_EQ(h_3, deserializer.Deserialize<int16_t>(3));
}

TEST(Test_SilSerDes, serdes_unaligned_uint)
{
    uint16_t a_3 = 3;
    uint16_t b_3 = 2;
    uint16_t c_3 = 3;
    uint16_t d_3 = 1;
    uint16_t e_3 = 3;
    uint16_t f_3 = 2;
    uint16_t g_3 = 3;
    uint16_t h_3 = 1;

    Serializer serializer;
    serializer.Serialize(a_3, 3);
    serializer.Serialize(b_3, 3);
    serializer.Serialize(c_3, 3);
    serializer.Serialize(d_3, 3);
    serializer.Serialize(e_3, 3);
    serializer.Serialize(f_3, 3);
    serializer.Serialize(g_3, 3);
    serializer.Serialize(h_3, 3);

    Deserializer deserializer;
    deserializer.Reset(serializer.ReleaseBuffer());
    EXPECT_EQ(a_3, deserializer.Deserialize<uint16_t>(3));
    EXPECT_EQ(b_3, deserializer.Deserialize<uint16_t>(3));
    EXPECT_EQ(c_3, deserializer.Deserialize<uint16_t>(3));
    EXPECT_EQ(d_3, deserializer.Deserialize<uint16_t>(3));
    EXPECT_EQ(e_3, deserializer.Deserialize<uint16_t>(3));
    EXPECT_EQ(f_3, deserializer.Deserialize<uint16_t>(3));
    EXPECT_EQ(g_3, deserializer.Deserialize<uint16_t>(3));
    EXPECT_EQ(h_3, deserializer.Deserialize<uint16_t>(3));
}

TEST(Test_SilSerDes, serdes_bool)
{
    Serializer serializer;
    serializer.Serialize(true);
    serializer.Serialize(false);

    Deserializer deserializer;
    deserializer.Reset(serializer.ReleaseBuffer());
    EXPECT_EQ(true, deserializer.Deserialize<bool>());
    EXPECT_EQ(false, deserializer.Deserialize<bool>());
}

TEST(Test_SilSerDes, serdes_floatingpoint)
{
    const float f_neg = -13.37f;
    const float f_pos = 13.37f;
    const double d_neg = -133.7f;
    const double d_pos = 133.7f;

    Serializer serializer;
    serializer.Serialize(f_neg);
    serializer.Serialize(f_pos);
    serializer.Serialize(d_neg);
    serializer.Serialize(d_pos);

    Deserializer deserializer;
    deserializer.Reset(serializer.ReleaseBuffer());
    EXPECT_EQ(f_neg, deserializer.Deserialize<float>());
    EXPECT_EQ(f_pos, deserializer.Deserialize<float>());
    EXPECT_EQ(d_neg, deserializer.Deserialize<double>());
    EXPECT_EQ(d_pos, deserializer.Deserialize<double>());
}

TEST(Test_SilSerDes, serdes_string)
{
    const std::string value{"Hello world! I love you so much."};
    Serializer serializer;
    serializer.Serialize(value);

    Deserializer deserializer;
    deserializer.Reset(serializer.ReleaseBuffer());
    EXPECT_EQ(value, deserializer.Deserialize<std::string>());
}

TEST(Test_SilSerDes, serdes_bytes)
{
    const std::vector<uint8_t> bytes{'a', 'b', 'c', 'd'};
    Serializer serializer;
    serializer.Serialize(bytes);

    Deserializer deserializer;
    deserializer.Reset(serializer.ReleaseBuffer());
    EXPECT_EQ(bytes, deserializer.Deserialize<std::vector<uint8_t>>());
}

TEST(Test_SilSerDes, serdes_unaligned_mixed_madness)
{
    // all int64_t are treated as int63
    int64_t a = -112353;
    std::string b{"Hello world! I love you so much."};
    int64_t c = 21256;
    bool d = true;
    int64_t e = -158790;
    double f = 39187.79;

    Serializer serializer;
    serializer.Serialize(a, 63);
    serializer.Serialize(b);
    serializer.Serialize(c, 63);
    serializer.Serialize(d);
    serializer.Serialize(e, 63);
    serializer.Serialize(f);

    Deserializer deserializer;
    deserializer.Reset(serializer.ReleaseBuffer());
    EXPECT_EQ(a, deserializer.Deserialize<int64_t>(63));
    EXPECT_EQ(b, deserializer.Deserialize<std::string>());
    EXPECT_EQ(c, deserializer.Deserialize<int64_t>(63));
    EXPECT_EQ(d, deserializer.Deserialize<bool>());
    EXPECT_EQ(e, deserializer.Deserialize<int64_t>(63));
    EXPECT_EQ(f, deserializer.Deserialize<double>());
}

TEST(Test_SilSerDes, serdes_array)
{
    Serializer serializer;
    serializer.BeginArray(3);
    serializer.Serialize(1, 32);
    serializer.Serialize(2, 32);
    serializer.Serialize(3, 32);
    serializer.EndArray();

    Deserializer deserializer;
    deserializer.Reset(serializer.ReleaseBuffer());
    EXPECT_EQ(3, deserializer.BeginArray());
    EXPECT_EQ(1, deserializer.Deserialize<int32_t>(32));
    EXPECT_EQ(2, deserializer.Deserialize<int32_t>(32));
    EXPECT_EQ(3, deserializer.Deserialize<int32_t>(32));
    deserializer.EndArray();
}

} // anonymous namespace
