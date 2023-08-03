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

#include <functional>
#include <numeric>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "silkit/util/Span.hpp"

namespace SilKit {
namespace Util {
namespace tests {

using ::testing::Return;
using ::testing::A;
using ::testing::An;
using ::testing::_;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Throw;

TEST(Test_Span, test_view_data_and_size)
{
    using namespace std::placeholders;
    using namespace ::SilKit::Util;

    std::vector<int> intSequence(10);
    std::iota(intSequence.begin(), intSequence.end(), 0);

    auto intSequenceView = ToSpan(intSequence);
    EXPECT_EQ(intSequenceView.size(), intSequence.size());
    EXPECT_EQ(intSequenceView.data(), intSequence.data());

    const std::vector<int>& constIntSequence = intSequence;
    auto constIntSequenceView = ToSpan(constIntSequence);

    EXPECT_EQ(constIntSequenceView.size(), constIntSequence.size());
    EXPECT_EQ(constIntSequenceView.data(), constIntSequence.data());
}

TEST(Test_Span, test_view_content_equals_sourcevector)
{
    using namespace std::placeholders;
    using namespace ::SilKit::Util;

    std::vector<int> intSequence(10);
    std::iota(intSequence.begin(), intSequence.end(), 0);

    auto intSequenceView = ToSpan(intSequence);
    for (size_t i = 0; i < intSequence.size(); i++)
    {
        EXPECT_EQ(intSequenceView[i], intSequence[i]);
    }
}

TEST(Test_Span, test_trim_view)
{
    using namespace std::placeholders;
    using namespace ::SilKit::Util;

    std::vector<int> intSequence(10);
    std::iota(intSequence.begin(), intSequence.end(), 0);

    auto intSequenceView = ToSpan(intSequence);

    intSequenceView.trim_front(2);
    ASSERT_EQ(intSequenceView.size(), static_cast<size_t>(8));

    intSequenceView.trim_back(2);
    ASSERT_EQ(intSequenceView.size(), static_cast<size_t>(6));

    for (size_t i = 0; i < intSequenceView.size(); i++)
    {
        EXPECT_EQ(intSequenceView[i], intSequence[i + 2]);
    }
}

TEST(Test_Span, test_iterators)
{
    using namespace std::placeholders;
    using namespace ::SilKit::Util;

    std::vector<int> intSequence(10);
    std::iota(intSequence.begin(), intSequence.end(), 0);

    auto intSequenceView = ToSpan(intSequence);

    auto vectorIter = intSequence.cbegin();
    auto viewIter = intSequenceView.cbegin();

    for (size_t i = 0; i < intSequence.size(); i++)
    {
        EXPECT_EQ(*viewIter, *vectorIter);
        viewIter++;
        vectorIter++;
    }

    vectorIter = intSequence.cbegin();
    for (auto&& viewElement : intSequenceView)
    {
        EXPECT_EQ(viewElement, *vectorIter);
        vectorIter++;
    }
}

TEST(Test_Span, test_modify_view)
{
    using namespace std::placeholders;
    using namespace ::SilKit::Util;

    std::vector<int> intSequence(10);
    std::fill(intSequence.begin(), intSequence.end(), 0);

    for (auto&& elem : intSequence)
    {
        ASSERT_EQ(elem, 0);
    }

    auto intSequenceView = ToSpan(intSequence);
    std::iota(intSequenceView.begin(), intSequenceView.end(), 0);

    // check source vector for modifications
    int expectedValue = 0;
    for (auto&& elem : intSequence)
    {
        EXPECT_EQ(elem, expectedValue);
        expectedValue++;
    }
}

TEST(Test_Span, test_ctor_vector)
{
    std::vector<uint8_t> bytes{1, 2, 3, 4, 5};

    {
        std::vector<uint8_t>& mlref = bytes;

        Span<const uint8_t> cs{mlref};
        ASSERT_EQ(cs.size(), mlref.size());
        ASSERT_EQ(ToStdVector(cs), mlref);

        Span<uint8_t> ms{mlref};
        ASSERT_EQ(ms.size(), mlref.size());
        ASSERT_EQ(ToStdVector(ms), mlref);
    }

    {
        const std::vector<uint8_t>& clref = bytes;

        Span<const uint8_t> cs{clref};
        ASSERT_EQ(cs.size(), clref.size());
        ASSERT_EQ(ToStdVector(cs), clref);

        ASSERT_FALSE((std::is_constructible<Span<uint8_t>, const std::vector<uint8_t>&>::value));
    }
}

TEST(Test_Span, test_assign_vector)
{
    std::vector<uint8_t> bytes{1, 2, 3, 4, 5};

    {
        std::vector<uint8_t>& mlref = bytes;

        Span<const uint8_t> cs;
        ASSERT_TRUE(cs.empty());

        cs = mlref;
        ASSERT_EQ(cs.size(), mlref.size());
        ASSERT_EQ(ToStdVector(cs), mlref);

        Span<uint8_t> ms;
        ASSERT_TRUE(ms.empty());

        ms = mlref;
        ASSERT_EQ(ms.size(), mlref.size());
        ASSERT_EQ(ToStdVector(ms), mlref);
    }

    {
        const std::vector<uint8_t>& clref = bytes;

        Span<const uint8_t> cs;
        ASSERT_TRUE(cs.empty());

        cs = clref;
        ASSERT_EQ(cs.size(), clref.size());
        ASSERT_EQ(ToStdVector(cs), clref);

        ASSERT_FALSE((std::is_assignable<Span<uint8_t>, const std::vector<uint8_t>&>::value));
    }
}

} // namespace tests
} // namespace Util
} // namespace SilKit
