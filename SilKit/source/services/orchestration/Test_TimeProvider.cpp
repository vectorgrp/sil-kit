// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <chrono>
#include <functional>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "TimeProvider.hpp"

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
} // namespace
