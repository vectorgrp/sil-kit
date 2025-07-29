// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "SynchronizedHandlers.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <thread>
#include <functional>
#include <set>
#include <mutex>
#include <atomic>

namespace {

using TestFunction = std::function<void()>;

struct Callbacks
{
    MOCK_METHOD(void, TestA, ());
    MOCK_METHOD(void, TestB, ());
    MOCK_METHOD(void, TestC, ());
    MOCK_METHOD(void, TestD, ());
};

TEST(Test_SynchronizedHandlers, add_and_remove_functions_during_calling)
{
    SilKit::Util::SynchronizedHandlers<TestFunction> callables;

    Callbacks callbacks;

    const auto hA = callables.Add([&callbacks] { callbacks.TestA(); });

    const auto hB = callables.Add([&callables, &callbacks, hA] {
        callbacks.TestB();
        callables.Remove(hA);
    });

    const auto hC = callables.Add([&callables, &callbacks, hB] {
        callbacks.TestC();
        callables.Remove(hB);

        callables.Add([&callbacks] { callbacks.TestD(); });
    });

    EXPECT_CALL(callbacks, TestA).Times(1);
    EXPECT_CALL(callbacks, TestB).Times(1);
    EXPECT_CALL(callbacks, TestC).Times(1);
    EXPECT_CALL(callbacks, TestD).Times(1);

    callables.InvokeAll();

    callables.Remove(hC);

    EXPECT_CALL(callbacks, TestA).Times(0);
    EXPECT_CALL(callbacks, TestB).Times(0);
    EXPECT_CALL(callbacks, TestC).Times(0);
    EXPECT_CALL(callbacks, TestD).Times(1);

    callables.InvokeAll();
}

using TestFunctionTwoArgs = std::function<void(int, float)>;

struct CallbacksTwoArgs
{
    MOCK_METHOD(void, TestA, (int i, float f));
    MOCK_METHOD(void, TestB, (int i, float f));
    MOCK_METHOD(void, TestC, (int i, float f));
    MOCK_METHOD(void, TestD, (int i, float f));
};

TEST(Test_SynchronizedHandlers, add_and_remove_functions_during_calling_two_args)
{
    SilKit::Util::SynchronizedHandlers<TestFunctionTwoArgs> callables;

    CallbacksTwoArgs callbacks;

    const auto hA = callables.Add([&callbacks](int i, float f) { callbacks.TestA(i, f); });

    const auto hB = callables.Add([&callables, &callbacks, hA](int i, float f) {
        callbacks.TestB(i, f);
        callables.Remove(hA);
    });

    const auto hC = callables.Add([&callables, &callbacks, hB](int i, float f) {
        callbacks.TestC(i, f);
        callables.Remove(hB);

        callables.Add([&callbacks](int i, float f) { callbacks.TestD(i, f); });
    });

    const int i = 1;
    const float f = 2.0f;

    EXPECT_CALL(callbacks, TestA(i, f)).Times(1);
    EXPECT_CALL(callbacks, TestB(i, f)).Times(1);
    EXPECT_CALL(callbacks, TestC(i, f)).Times(1);
    EXPECT_CALL(callbacks, TestD(i, f)).Times(1);

    callables.InvokeAll(i, f);

    callables.Remove(hC);

    EXPECT_CALL(callbacks, TestA(testing::_, testing::_)).Times(0);
    EXPECT_CALL(callbacks, TestB(testing::_, testing::_)).Times(0);
    EXPECT_CALL(callbacks, TestC(testing::_, testing::_)).Times(0);
    EXPECT_CALL(callbacks, TestD(i, f)).Times(1);

    callables.InvokeAll(i, f);
}

TEST(Test_SynchronizedHandlers, add_remove_call_concurrently)
{
    SilKit::Util::SynchronizedHandlers<TestFunction> callables;

    std::mutex mutex;
    std::set<SilKit::Util::HandlerId> handlerIds;

    std::atomic<std::uint32_t> called{0};

    static constexpr int handler_count = 10;
    static constexpr std::uint32_t call_count = 10;

    auto adder = std::thread{[&mutex, &handlerIds, &callables, &called] {
        for (int i = 0; i < handler_count; ++i)
        {
            const auto handlerId = callables.Add([&called] {
                std::this_thread::sleep_for(std::chrono::milliseconds{10});
                ++called;
            });

            {
                const auto lock = std::unique_lock<std::mutex>{mutex};
                handlerIds.emplace(handlerId);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds{10});
        }
    }};

    auto remover = std::thread{[&mutex, &handlerIds, &callables, &called] {
        std::this_thread::sleep_for(std::chrono::milliseconds{50});

        for (int i = 0; i < handler_count; ++i)
        {
            const auto is_empty = [&mutex, &handlerIds] {
                const auto lock = std::unique_lock<std::mutex>{mutex};
                return handlerIds.empty();
            };

            while (is_empty() || called.load() < call_count)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds{10});
            }

            const auto result = [&mutex, &handlerIds] {
                const auto lock = std::unique_lock<std::mutex>{mutex};
                const auto it = handlerIds.begin();
                if (it != handlerIds.end())
                {
                    const auto handlerId = *it;
                    handlerIds.erase(it);

                    return std::make_pair(handlerId, true);
                }

                return std::make_pair(SilKit::Util::HandlerId{}, false);
            }();

            if (result.second)
            {
                callables.Remove(result.first);
            }
        }
    }};

    auto caller = std::thread{[&callables, &called] {
        while (called.load() < call_count)
        {
            callables.InvokeAll();
        }
    }};

    adder.join();
    remover.join();
    caller.join();

    EXPECT_GE(called.load(), call_count);
    EXPECT_EQ(handlerIds.size(), 0u);
}

TEST(Test_SynchronizedHandlers, swap_transfers_handlers)
{
    using TestHandlers = SilKit::Util::SynchronizedHandlers<TestFunction>;
    using std::swap;

    size_t callCounter = 0;

    TestHandlers a, b;

    std::set<SilKit::Util::HandlerId> ids;

    ids.insert(b.Add([&callCounter] { ++callCounter; }));

    ids.insert(b.Add([&callCounter] { ++callCounter; }));

    ASSERT_EQ(ids.size(), 2u);

    ASSERT_EQ(callCounter, 0u);
    a.InvokeAll();
    ASSERT_EQ(callCounter, 0u);
    b.InvokeAll();
    ASSERT_EQ(callCounter, 2u);

    swap(a, b);

    ASSERT_EQ(callCounter, 2u);
    a.InvokeAll();
    ASSERT_EQ(callCounter, 4u);
    b.InvokeAll();
    ASSERT_EQ(callCounter, 4u);

    ids.insert(a.Add([&callCounter] { ++callCounter; }));

    ASSERT_EQ(ids.size(), 3u);

    ASSERT_EQ(callCounter, 4u);
    a.InvokeAll();
    ASSERT_EQ(callCounter, 7u);
    b.InvokeAll();
    ASSERT_EQ(callCounter, 7u);
}

} // namespace
