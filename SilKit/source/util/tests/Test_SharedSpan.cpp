// SPDX-FileCopyrightText: 2025 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "util/SharedSpan.hpp"

#include <memory>
#include <vector>

#include "gtest/gtest.h"

namespace {

using namespace SilKit::Util;

TEST(Test_SharedSpan, default_constructed_is_empty_and_unowned)
{
    SharedSpan<uint8_t> shared;

    EXPECT_TRUE(shared.empty());
    EXPECT_EQ(shared.size(), 0u);
    EXPECT_FALSE(shared.HasOwner());
    EXPECT_EQ(shared.AsSpan().data(), nullptr);
}

TEST(Test_SharedSpan, owns_a_copy_of_a_vector)
{
    std::vector<uint8_t> data{1, 2, 3, 4};
    SharedSpan<uint8_t> shared{data};

    ASSERT_TRUE(shared.HasOwner());
    ASSERT_EQ(shared.size(), 4u);
    // the copy is independent of the source
    EXPECT_NE(shared.AsSpan().data(), data.data());

    data.clear();
    EXPECT_EQ(shared.AsSpan()[0], 1);
    EXPECT_EQ(shared.AsSpan()[3], 4);
}

TEST(Test_SharedSpan, owns_a_copy_of_a_span)
{
    const std::vector<uint8_t> data{9, 8, 7};
    SharedSpan<uint8_t> shared{ToSpan(data)};

    ASSERT_EQ(shared.size(), 3u);
    EXPECT_NE(shared.AsSpan().data(), data.data());
    EXPECT_TRUE(ItemsAreEqual(shared.AsSpan(), ToSpan(data)));
}

TEST(Test_SharedSpan, initializer_list_owns_a_copy)
{
    SharedSpan<uint8_t> shared{1, 2, 3};

    ASSERT_EQ(shared.size(), 3u);
    EXPECT_TRUE(shared.HasOwner());
    EXPECT_EQ(shared.AsSpan()[2], 3);
}

TEST(Test_SharedSpan, pads_up_to_a_minimum_size)
{
    const std::vector<uint8_t> data{1, 2};
    SharedSpan<uint8_t> shared{ToSpan(data), 5, 0xAB};

    ASSERT_EQ(shared.size(), 5u);
    EXPECT_EQ(shared.AsSpan()[0], 1);
    EXPECT_EQ(shared.AsSpan()[1], 2);
    EXPECT_EQ(shared.AsSpan()[2], 0xAB);
    EXPECT_EQ(shared.AsSpan()[4], 0xAB);
}

TEST(Test_SharedSpan, minimum_size_does_not_truncate)
{
    const std::vector<uint8_t> data{1, 2, 3, 4};
    SharedSpan<uint8_t> shared{ToSpan(data), 2};

    EXPECT_EQ(shared.size(), 4u);
}

TEST(Test_SharedSpan, aliasing_keeps_the_owner_alive)
{
    auto blob = std::make_shared<std::vector<uint8_t>>(std::vector<uint8_t>{10, 20, 30, 40});
    const auto* rawAddress = blob->data();

    SharedSpan<uint8_t> shared{std::shared_ptr<const std::vector<uint8_t>>{blob},
                               Span<const uint8_t>{blob->data() + 1, 2}};

    // drop the original handle; the SharedSpan must keep the storage alive
    blob.reset();

    ASSERT_TRUE(shared.HasOwner());
    ASSERT_EQ(shared.size(), 2u);
    // truly aliasing, not a copy
    EXPECT_EQ(shared.AsSpan().data(), rawAddress + 1);
    EXPECT_EQ(shared.AsSpan()[0], 20);
    EXPECT_EQ(shared.AsSpan()[1], 30);
}

TEST(Test_SharedSpan, borrowed_does_not_own)
{
    const std::vector<uint8_t> data{5, 6, 7};
    auto shared = SharedSpan<uint8_t>::Borrowed(ToSpan(data));

    EXPECT_FALSE(shared.HasOwner());
    EXPECT_EQ(shared.AsSpan().data(), data.data());
    EXPECT_EQ(shared.size(), 3u);
}

TEST(Test_SharedSpan, subspan_retains_the_owner_and_aliases)
{
    SharedSpan<uint8_t> shared{std::vector<uint8_t>{1, 2, 3, 4, 5}};
    const auto* base = shared.AsSpan().data();

    auto sub = shared.Subspan(1, 3);

    ASSERT_TRUE(sub.HasOwner());
    EXPECT_EQ(sub.size(), 3u);
    EXPECT_EQ(sub.AsSpan().data(), base + 1);
    EXPECT_EQ(sub.AsSpan()[0], 2);
    EXPECT_EQ(sub.AsSpan()[2], 4);
}

TEST(Test_SharedSpan, subspan_range_is_checked)
{
    SharedSpan<uint8_t> shared{std::vector<uint8_t>{1, 2, 3}};

    EXPECT_THROW(shared.Subspan(0, 4), SilKit::OutOfRangeError);
    EXPECT_THROW(shared.Subspan(4, 0), SilKit::OutOfRangeError);
    EXPECT_THROW(shared.Subspan(2, 2), SilKit::OutOfRangeError);
    EXPECT_NO_THROW(shared.Subspan(3, 0));
}

TEST(Test_SharedSpan, cloned_detaches_from_a_larger_owner)
{
    SharedSpan<uint8_t> shared{std::vector<uint8_t>{1, 2, 3, 4, 5}};
    auto sub = shared.Subspan(1, 2);

    auto cloned = sub.Cloned();

    ASSERT_EQ(cloned.size(), 2u);
    EXPECT_NE(cloned.AsSpan().data(), sub.AsSpan().data());
    EXPECT_EQ(cloned.AsSpan()[0], 2);
    EXPECT_EQ(cloned.AsSpan()[1], 3);
}

TEST(Test_SharedSpan, items_are_equal_compares_contents)
{
    SharedSpan<uint8_t> a{std::vector<uint8_t>{1, 2, 3}};
    SharedSpan<uint8_t> b{std::vector<uint8_t>{1, 2, 3}};
    SharedSpan<uint8_t> c{std::vector<uint8_t>{1, 2, 4}};

    EXPECT_TRUE(ItemsAreEqual(a, b));
    EXPECT_FALSE(ItemsAreEqual(a, c));
}

TEST(Test_SharedSpan, make_shared_span_range_is_checked)
{
    auto blob = std::make_shared<const std::vector<uint8_t>>(std::vector<uint8_t>{1, 2, 3, 4});

    auto view = MakeSharedSpan(blob, 1, 2);
    ASSERT_EQ(view.size(), 2u);
    EXPECT_EQ(view.AsSpan().data(), blob->data() + 1);

    EXPECT_THROW(MakeSharedSpan(blob, 3, 2), SilKit::OutOfRangeError);
    EXPECT_THROW(MakeSharedSpan(blob, 5, 0), SilKit::OutOfRangeError);
}

TEST(Test_SharedSpan, make_shared_span_handles_a_null_blob)
{
    EXPECT_TRUE(MakeSharedSpan(nullptr, 0, 0).empty());
    EXPECT_THROW(MakeSharedSpan(nullptr, 0, 1), SilKit::OutOfRangeError);
}

TEST(Test_SharedSpan, copies_share_one_owner)
{
    SharedSpan<uint8_t> original{std::vector<uint8_t>{1, 2, 3}};
    const auto* base = original.AsSpan().data();

    SharedSpan<uint8_t> copy = original;
    original = SharedSpan<uint8_t>{};

    ASSERT_EQ(copy.size(), 3u);
    EXPECT_EQ(copy.AsSpan().data(), base);
    EXPECT_EQ(copy.AsSpan()[1], 2);
}

} // namespace
