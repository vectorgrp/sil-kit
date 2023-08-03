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

#include "DataSerdes.hpp"

#include <chrono>

#include "gtest/gtest.h"

using namespace std::chrono_literals;

namespace SilKit {
namespace Services {
namespace PubSub {
    static bool operator==(const WireDataMessageEvent& lhs, const WireDataMessageEvent& rhs)
    {
        return Util::ItemsAreEqual(lhs.data, rhs.data)
            && lhs.timestamp == rhs.timestamp;
    }
}
}
}
TEST(Test_DataSerdes, SimData_LargeDataMessage)
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

