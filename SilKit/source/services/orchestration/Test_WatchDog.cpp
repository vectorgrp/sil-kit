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

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "silkit/services/orchestration/string_utils.hpp"

#include "ParticipantConfiguration.hpp"
#include "WatchDog.hpp"
#include "functional.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace SilKit;
using namespace SilKit::Services::Orchestration;

class LimitedMockClock : public WatchDog::IClock
{
public:
    explicit LimitedMockClock(std::chrono::nanoseconds limit, std::chrono::nanoseconds tick = 1ms)
        : _limitRep{static_cast<std::chrono::nanoseconds>(limit).count()}
        , _tick{tick}
    {
    }

    auto Now() const -> std::chrono::nanoseconds override
    {
        _now += _tick;

        // set the flag that the limit has been reached (but only _once_, otherwise an exception is thrown)
        if (_now >= GetLimit())
        {
            if (!_limitReachedFlagged.exchange(true))
            {
                _limitReached.set_value();
            }
        }

        return _now;
    }

    /// This function waits until either, the mock time has reached the current limit, or, the wall-clock timeout has
    /// been reached.
    ///
    /// \param wallClockTimeout How long this function should wait for the limit to be reached
    /// \return true if the limit was reached, false if the wall-clock timeout was reached
    bool WaitUntilLimitReachedFor(std::chrono::nanoseconds wallClockTimeout)
    {
        const auto status = _limitReached.get_future().wait_for(wallClockTimeout);
        return status == std::future_status::ready;
    }

    /// Advances the limit and resets the reached flag. Therefore WaitUntilLimitReachedFor can be called again
    /// afterwards, even if the limit had been reached before.
    ///
    /// \param advance By how much the current limit should be advanced
    void AdvanceLimitBy(std::chrono::nanoseconds advance)
    {
        _limitRep += static_cast<std::chrono::nanoseconds>(advance).count();

        if (_limitReachedFlagged)
        {
            _limitReached = {};
            _limitReachedFlagged = false;
        }
    }

private:
    auto GetLimit() const -> std::chrono::nanoseconds { return std::chrono::nanoseconds{_limitRep}; }

private:
    std::atomic<std::chrono::nanoseconds::rep> _limitRep;
    std::chrono::nanoseconds _tick;

    // stores the current mock-time
    mutable std::chrono::nanoseconds _now{};
    // true if the promise has already been set
    mutable std::atomic<bool> _limitReachedFlagged{false};
    // will be set once (!) if _now is greater or equal to _limit
    mutable std::promise<void> _limitReached{};
};

class Test_WatchDog : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD1(WarnHandler, void(std::chrono::milliseconds));
        MOCK_METHOD1(ErrorHandler, void(std::chrono::milliseconds));

        MOCK_METHOD(void, SequencePoint, ());
    };

protected:
    Test_WatchDog() = default;

protected:
    // ----------------------------------------
    // Helper Methods

protected:
    // ----------------------------------------
    // Members
    Callbacks callbacks;

    const std::chrono::milliseconds WAIT_EXPECT_READY = 10s;
    const std::chrono::milliseconds WAIT_EXPECT_TIMEOUT = 50ms;
};

// ================================================================================
// IMPORTANT: Set expectations (EXPECT_CALL) before the call to watchDog.Start()!
// ================================================================================
//
// Note: The watchdog is running in a separate thread in the background and
//       uses the wall clock for periodic checks. It is possible that
//       scheduling might lead to test failures in rare cases. To make the
//       tests entirely deterministic, this stepping must be refactored out
//       of the watchdog into it's own interface as well.

TEST_F(Test_WatchDog, throw_if_warn_timeout_is_zero)
{
    EXPECT_THROW(WatchDog(Config::HealthCheck{0ms, 10ms}), SilKitError);
}

TEST_F(Test_WatchDog, throw_if_error_timeout_is_zero)
{
    EXPECT_THROW(WatchDog(Config::HealthCheck{10ms, 0ms}), SilKitError);
}

TEST_F(Test_WatchDog, warn_after_timeout)
{
    LimitedMockClock mockClock{50ms, 2ms};

    WatchDog watchDog{Config::HealthCheck{10ms, std::chrono::milliseconds::max()}, &mockClock};
    watchDog.SetWarnHandler(Util::bind_method(&callbacks, &Callbacks::WarnHandler));

    EXPECT_CALL(callbacks, WarnHandler(_)).Times(1);

    watchDog.Start();

    ASSERT_TRUE(mockClock.WaitUntilLimitReachedFor(WAIT_EXPECT_READY));
}

TEST_F(Test_WatchDog, error_after_timeout)
{
    LimitedMockClock mockClock{100ms, 5ms};

    WatchDog watchDog{Config::HealthCheck{10ms, 50ms}, &mockClock};
    watchDog.SetErrorHandler(Util::bind_method(&callbacks, &Callbacks::ErrorHandler));

    EXPECT_CALL(callbacks, ErrorHandler(_)).Times(1);

    watchDog.Start();

    ASSERT_TRUE(mockClock.WaitUntilLimitReachedFor(WAIT_EXPECT_READY));
}

TEST_F(Test_WatchDog, warn_only_once)
{
    LimitedMockClock mockClock{100ms, 10ms};

    WatchDog watchDog{Config::HealthCheck{20ms, std::chrono::milliseconds::max()}, &mockClock};
    watchDog.SetWarnHandler(Util::bind_method(&callbacks, &Callbacks::WarnHandler));
    watchDog.SetErrorHandler(Util::bind_method(&callbacks, &Callbacks::ErrorHandler));

    EXPECT_CALL(callbacks, WarnHandler(_)).Times(1);
    EXPECT_CALL(callbacks, ErrorHandler(_)).Times(0);

    watchDog.Start();

    ASSERT_TRUE(mockClock.WaitUntilLimitReachedFor(WAIT_EXPECT_READY));
}

TEST_F(Test_WatchDog, error_only_once)
{
    LimitedMockClock mockClock{100ms, 10ms};

    WatchDog watchDog{Config::HealthCheck{10ms, 50ms}, &mockClock};
    watchDog.SetErrorHandler(Util::bind_method(&callbacks, &Callbacks::ErrorHandler));

    EXPECT_CALL(callbacks, ErrorHandler(_)).Times(1);

    watchDog.Start();

    ASSERT_TRUE(mockClock.WaitUntilLimitReachedFor(WAIT_EXPECT_READY));
}

TEST_F(Test_WatchDog, no_callback_if_reset_in_time)
{
    LimitedMockClock mockClock{1s, 100ms};

    WatchDog watchDog{Config::HealthCheck{2000ms, 3000ms}, &mockClock};
    watchDog.SetWarnHandler(Util::bind_method(&callbacks, &Callbacks::WarnHandler));
    watchDog.SetErrorHandler(Util::bind_method(&callbacks, &Callbacks::ErrorHandler));

    EXPECT_CALL(callbacks, WarnHandler(_)).Times(0);
    EXPECT_CALL(callbacks, ErrorHandler(_)).Times(0);

    watchDog.Start();

    // After being started, the watchdog will ask the clock for the current time periodically. This will advance the
    // mock time each time and eventually the limit will be reached.

    ASSERT_TRUE(mockClock.WaitUntilLimitReachedFor(WAIT_EXPECT_READY));
    mockClock.AdvanceLimitBy(1s);

    watchDog.Reset();

    // After being reset, the watchdog will not ask the clock for the current time anymore. Since the mock time only
    // advances when the watchdog asks for it, the new limit will never be reached.

    // The wall-clock timeout is deliberately chosen rather small since it should be reached.
    ASSERT_FALSE(mockClock.WaitUntilLimitReachedFor(WAIT_EXPECT_TIMEOUT));
}

TEST_F(Test_WatchDog, create_health_check_unconfigured)
{
    Config::HealthCheck healthCheck;
    WatchDog watchDog(healthCheck);
    EXPECT_EQ(watchDog.GetWarnTimeout(), watchDog._defaultTimeout);
    EXPECT_EQ(watchDog.GetErrorTimeout(), watchDog._defaultTimeout);

    healthCheck.softResponseTimeout = 5ms;
    WatchDog watchDogWithSoftTimeout{healthCheck};
    EXPECT_EQ(watchDogWithSoftTimeout.GetWarnTimeout(), 5ms);
    healthCheck.softResponseTimeout.reset();

    healthCheck.hardResponseTimeout = 5ms;
    WatchDog watchDogWithHardTimeout{healthCheck};
    EXPECT_EQ(watchDogWithHardTimeout.GetErrorTimeout(), 5ms);
}

TEST_F(Test_WatchDog, create_health_check_configured)
{
    LimitedMockClock mockClock{1000ms, 250ms};

    WatchDog watchDog{Config::HealthCheck{2000ms, 3000ms}, &mockClock};
    watchDog.SetWarnHandler(Util::bind_method(&callbacks, &Callbacks::WarnHandler));
    watchDog.SetErrorHandler(Util::bind_method(&callbacks, &Callbacks::ErrorHandler));

    EXPECT_CALL(callbacks, WarnHandler(_)).Times(0);
    EXPECT_CALL(callbacks, ErrorHandler(_)).Times(0);

    watchDog.Start();

    ASSERT_TRUE(mockClock.WaitUntilLimitReachedFor(WAIT_EXPECT_READY));

    watchDog.Reset();
}

TEST_F(Test_WatchDog, nothing_without_soft_and_hard)
{
    LimitedMockClock mockClock{200ms, 50ms};

    WatchDog watchDog{Config::HealthCheck{{}, {}}, &mockClock};
    watchDog.SetWarnHandler(Util::bind_method(&callbacks, &Callbacks::WarnHandler));
    watchDog.SetErrorHandler(Util::bind_method(&callbacks, &Callbacks::ErrorHandler));

    EXPECT_CALL(callbacks, WarnHandler(_)).Times(0);
    EXPECT_CALL(callbacks, ErrorHandler(_)).Times(0);

    watchDog.Start();

    ASSERT_TRUE(mockClock.WaitUntilLimitReachedFor(WAIT_EXPECT_READY));
}

TEST_F(Test_WatchDog, warn_with_soft_without_hard)
{
    LimitedMockClock mockClock{200ms, 50ms};

    WatchDog watchDog{Config::HealthCheck{100ms, {}}, &mockClock};
    watchDog.SetWarnHandler(Util::bind_method(&callbacks, &Callbacks::WarnHandler));
    watchDog.SetErrorHandler(Util::bind_method(&callbacks, &Callbacks::ErrorHandler));

    EXPECT_CALL(callbacks, WarnHandler(_)).Times(1);
    EXPECT_CALL(callbacks, ErrorHandler(_)).Times(0);

    watchDog.Start();

    ASSERT_TRUE(mockClock.WaitUntilLimitReachedFor(WAIT_EXPECT_READY));
}

TEST_F(Test_WatchDog, error_with_hard_without_soft)
{
    LimitedMockClock mockClock{200ms, 50ms};

    WatchDog watchDog{Config::HealthCheck{{}, 100ms}, &mockClock};
    watchDog.SetWarnHandler(Util::bind_method(&callbacks, &Callbacks::WarnHandler));
    watchDog.SetErrorHandler(Util::bind_method(&callbacks, &Callbacks::ErrorHandler));

    EXPECT_CALL(callbacks, WarnHandler(_)).Times(0);
    EXPECT_CALL(callbacks, ErrorHandler(_)).Times(1);

    watchDog.Start();

    ASSERT_TRUE(mockClock.WaitUntilLimitReachedFor(WAIT_EXPECT_READY));
}

TEST_F(Test_WatchDog, warn_and_error_with_soft_and_hard)
{
    LimitedMockClock mockClock{150ms, 10ms};

    WatchDog watchDog{Config::HealthCheck{100ms, 200ms}, &mockClock};
    watchDog.SetWarnHandler(Util::bind_method(&callbacks, &Callbacks::WarnHandler));
    watchDog.SetErrorHandler(Util::bind_method(&callbacks, &Callbacks::ErrorHandler));

    // NB: It is undefined behavior to interleave setting expectations and calling mock methods.

    // The expectations on WarnHandler and ErrorHandler are partially ordered. The explicit call to SequencePoint
    // separates two sets of expectations.
    testing::Sequence warnSequence, errorSequence;

    EXPECT_CALL(callbacks, WarnHandler(_)).Times(1).InSequence(warnSequence);
    EXPECT_CALL(callbacks, ErrorHandler(_)).Times(0).InSequence(errorSequence);
    EXPECT_CALL(callbacks, SequencePoint()).Times(1).InSequence(warnSequence, errorSequence);
    EXPECT_CALL(callbacks, WarnHandler(_)).Times(0).InSequence(warnSequence);
    EXPECT_CALL(callbacks, ErrorHandler(_)).Times(1).InSequence(errorSequence);

    watchDog.Start();

    ASSERT_TRUE(mockClock.WaitUntilLimitReachedFor(WAIT_EXPECT_READY));
    callbacks.SequencePoint();

    mockClock.AdvanceLimitBy(100ms);

    ASSERT_TRUE(mockClock.WaitUntilLimitReachedFor(WAIT_EXPECT_READY));
}

} // anonymous namespace
