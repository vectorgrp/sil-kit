// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <functional>
#include <numeric>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ib/util/vector_view.hpp"

namespace ib {
namespace util {
namespace tests {

using ::testing::Return;
using ::testing::A;
using ::testing::An;
using ::testing::_;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Throw;

    
TEST(IbVectorViewTest, test_view_data_and_size)
{
    using namespace std::placeholders;
    using namespace ::ib::util;

    std::vector<int> intSequence(10);
    std::iota(intSequence.begin(), intSequence.end(), 0);

    auto intSequenceView = make_vector_view(intSequence);
    EXPECT_EQ(intSequenceView.size(), intSequence.size());
    EXPECT_EQ(intSequenceView.data(), intSequence.data());

    const std::vector<int>& constIntSequence = intSequence;
    auto constIntSequenceView = make_vector_view(constIntSequence);

    EXPECT_EQ(constIntSequenceView.size(), constIntSequence.size());
    EXPECT_EQ(constIntSequenceView.data(), constIntSequence.data());
}

TEST(IbVectorViewTest, test_view_content_equals_sourcevector)
{
    using namespace std::placeholders;
    using namespace ::ib::util;

    std::vector<int> intSequence(10);
    std::iota(intSequence.begin(), intSequence.end(), 0);

    auto intSequenceView = make_vector_view(intSequence);
    for (size_t i = 0; i < intSequence.size(); i++)
    {
        EXPECT_EQ(intSequenceView[i], intSequence[i]);
    }

}

TEST(IbVectorViewTest, test_trim_view)
{
    using namespace std::placeholders;
    using namespace ::ib::util;

    std::vector<int> intSequence(10);
    std::iota(intSequence.begin(), intSequence.end(), 0);

    auto intSequenceView = make_vector_view(intSequence);

    intSequenceView.trim_front(2);
    ASSERT_EQ(intSequenceView.size(), static_cast<size_t>(8));

    intSequenceView.trim_back(2);
    ASSERT_EQ(intSequenceView.size(), static_cast<size_t>(6));

    for (size_t i = 0; i < intSequenceView.size(); i++)
    {
        EXPECT_EQ(intSequenceView[i], intSequence[i + 2]);
    }
}

TEST(IbVectorViewTest, test_iterators)
{
    using namespace std::placeholders;
    using namespace ::ib::util;

    std::vector<int> intSequence(10);
    std::iota(intSequence.begin(), intSequence.end(), 0);

    auto intSequenceView = make_vector_view(intSequence);

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


TEST(IbVectorViewTest, test_modify_view)
{
    using namespace std::placeholders;
    using namespace ::ib::util;

    std::vector<int> intSequence(10);
    std::fill(intSequence.begin(), intSequence.end(), 0);

    for (auto&& elem : intSequence)
    {
        ASSERT_EQ(elem, 0);
    }

    auto intSequenceView = make_vector_view(intSequence);
    std::iota(intSequenceView.begin(), intSequenceView.end(), 0);

    // check source vector for modifications
    int expectedValue = 0;
    for (auto&& elem : intSequence)
    {
        EXPECT_EQ(elem, expectedValue);
        expectedValue++;
    }

}
    
} // namespace tests
} // namespace util
} // namespace ib
