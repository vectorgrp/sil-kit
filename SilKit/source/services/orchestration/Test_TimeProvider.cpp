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
	auto handle = timeProvider.AddNextSimStepHandler([&invocationCount](auto, auto){
		invocationCount++;
		});
	timeProvider.SetTime(1ms, 0ms); //implicitly invoke handler
	timeProvider.RemoveNextSimStepHandler(handle);

	timeProvider.SetTime(2ms, 0ms); //implicitly invoke handler
	ASSERT_EQ(invocationCount, 1) << "Only the first SetTime should trigger the handler";

}
} // namespace
