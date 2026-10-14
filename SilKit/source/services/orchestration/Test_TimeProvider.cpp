// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <chrono>
#include <functional>
#include <future>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "services/orchestration/TimeProvider.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace SilKit::Services::Orchestration;

TEST(Test_TimeProvider, check_time_provider_impls)
{
    TimeProvider timeProvider{};
    timeProvider.SetSynchronizeVirtualTime(true);
    //Check default implementation is NoSync
    ASSERT_EQ(timeProvider.TimeProviderName(), "NoSyncProvider");

    timeProvider.ConfigureTimeProvider(TimeProviderKind::SyncTime);
    ASSERT_EQ(timeProvider.TimeProviderName(), "SynchronizedVirtualTimeProvider");

    timeProvider.ConfigureTimeProvider(TimeProviderKind::WallClock);
    ASSERT_EQ(timeProvider.TimeProviderName(), "WallclockProvider");

    // synchronized state must be kept, even after re-configuring
    ASSERT_TRUE(timeProvider.IsSynchronizingVirtualTime());
}

TEST(Test_TimeProvider, check_handlers)
{
    TimeProvider timeProvider{};
    // handlers only work for ::SyncTime and ::WallClock
    int invocationCount = 0;
    timeProvider.ConfigureTimeProvider(TimeProviderKind::SyncTime);
    auto handle = timeProvider.AddNextSimStepHandler([&invocationCount](auto, auto) { invocationCount++; });
    timeProvider.SetTime(1ms, 0ms); //implicitly invoke handler
    timeProvider.RemoveNextSimStepHandler(handle);

    timeProvider.SetTime(2ms, 0ms); //implicitly invoke handler
    ASSERT_EQ(invocationCount, 1) << "Only the first SetTime should trigger the handler";
}

TEST(Test_TimeProvider, handlers_can_be_added_from_a_tick)
{
    std::promise<void> addedHandlerInvoked;
    auto addedHandlerInvokedFuture = addedHandlerInvoked.get_future();

    TimeProvider timeProvider{};
    timeProvider.ConfigureTimeProvider(TimeProviderKind::NoSync);

    // only accessed by the timer thread
    bool isAdded{false};
    bool isInvoked{false};
    timeProvider.AddNextSimStepHandler([&](auto, auto) {
        if (isAdded)
        {
            return;
        }
        isAdded = true;
        timeProvider.AddNextSimStepHandler([&](auto, auto) {
            if (!isInvoked)
            {
                isInvoked = true;
                addedHandlerInvoked.set_value();
            }
        });
    });

    ASSERT_EQ(addedHandlerInvokedFuture.wait_for(5s), std::future_status::ready);
}
} // namespace
