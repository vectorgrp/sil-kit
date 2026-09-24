// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "core/vasio/ReceiveBlobPool.hpp"

#include <algorithm>
#include <memory>
#include <thread>
#include <vector>

#include "gtest/gtest.h"

namespace {

using SilKit::Core::ReceiveBlobPool;

using Blob = std::shared_ptr<std::vector<uint8_t>>;
using WeakBlob = std::weak_ptr<std::vector<uint8_t>>;

constexpr size_t Small{100};
constexpr size_t Large{60 * 1024};

TEST(Test_ReceiveBlobPool, reports_whether_it_is_enabled)
{
    EXPECT_TRUE(ReceiveBlobPool{true}.IsEnabled());
    EXPECT_FALSE(ReceiveBlobPool{false}.IsEnabled());
}

TEST(Test_ReceiveBlobPool, reuses_an_unreferenced_blob)
{
    ReceiveBlobPool pool{true};

    const auto* first = pool.Acquire(Small).get();
    const auto second = pool.Acquire(Small);

    EXPECT_EQ(second.get(), first);
    EXPECT_GE(second->size(), Small);
}

TEST(Test_ReceiveBlobPool, skips_a_referenced_blob)
{
    ReceiveBlobPool pool{true};

    const auto retained = pool.Acquire(Small);
    const auto second = pool.Acquire(Small);

    EXPECT_NE(second.get(), retained.get());
}

TEST(Test_ReceiveBlobPool, grows_a_blob_that_is_too_small)
{
    ReceiveBlobPool pool{true};

    const auto* first = pool.Acquire(Small).get();
    const auto second = pool.Acquire(10 * Small);

    EXPECT_EQ(second.get(), first);
    EXPECT_GE(second->size(), 10 * Small);
}

TEST(Test_ReceiveBlobPool, does_not_pool_blobs_above_the_limit)
{
    ReceiveBlobPool pool{true};

    const WeakBlob large = pool.Acquire(ReceiveBlobPool::MaxBlobSize + 1);

    EXPECT_TRUE(large.expired());
}

TEST(Test_ReceiveBlobPool, bounds_the_slack_of_a_reused_blob)
{
    ReceiveBlobPool pool{true};

    const auto* large = pool.Acquire(Large).get();
    const auto small = pool.Acquire(Small);

    EXPECT_NE(small.get(), large);
    EXPECT_LE(small->size(), Small + ReceiveBlobPool::MaxSlack);
}

TEST(Test_ReceiveBlobPool, mixed_sizes_settle_into_matching_blobs)
{
    ReceiveBlobPool pool{true};

    // The first large message grows the only blob, so the pool settles in the second round.
    pool.Acquire(Small);
    pool.Acquire(Large);

    const auto* small = pool.Acquire(Small).get();
    const auto* large = pool.Acquire(Large).get();
    EXPECT_NE(small, large);

    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(pool.Acquire(Small).get(), small);
        EXPECT_EQ(pool.Acquire(Large).get(), large);
    }
}

TEST(Test_ReceiveBlobPool, full_pool_hands_out_unpooled_blobs)
{
    ReceiveBlobPool pool{true};

    std::vector<Blob> retained;
    for (size_t i = 0; i < ReceiveBlobPool::MaxEntries; ++i)
    {
        retained.push_back(pool.Acquire(Small));
    }

    const WeakBlob extra = pool.Acquire(Small);

    EXPECT_TRUE(extra.expired());
}

TEST(Test_ReceiveBlobPool, full_pool_replaces_an_oversized_blob)
{
    ReceiveBlobPool pool{true};

    std::vector<WeakBlob> large;
    {
        std::vector<Blob> retained;
        for (size_t i = 0; i < ReceiveBlobPool::MaxEntries; ++i)
        {
            retained.push_back(pool.Acquire(Large));
            large.push_back(retained.back());
        }
    }

    const WeakBlob small = pool.Acquire(Small);

    EXPECT_FALSE(small.expired());
    EXPECT_EQ(std::count_if(large.begin(), large.end(), [](const WeakBlob& blob) { return blob.expired(); }), 1);
}

TEST(Test_ReceiveBlobPool, disabled_pool_keeps_no_blobs)
{
    ReceiveBlobPool pool{false};

    for (int i = 0; i < 3; ++i)
    {
        const WeakBlob blob = pool.Acquire(Small);
        EXPECT_TRUE(blob.expired());
    }
}

// NB: TSan does not model std::atomic_thread_fence, so under TSan this test needs a suppression for
//     ReceiveBlobPool::Acquire.
TEST(Test_ReceiveBlobPool, reuses_a_blob_released_on_another_thread)
{
    ReceiveBlobPool pool{true};

    auto blob = pool.Acquire(Small);
    std::fill(blob->begin(), blob->end(), uint8_t{0x42});
    const auto* first = blob.get();

    uint8_t observed{0};
    std::thread reader{[alias = std::move(blob), &observed]() mutable {
        observed = alias->front();
        alias.reset();
    }};
    reader.join();

    auto reused = pool.Acquire(Small);
    std::fill(reused->begin(), reused->end(), uint8_t{0x17});

    EXPECT_EQ(observed, 0x42);
    EXPECT_EQ(reused.get(), first);
}

} // namespace
