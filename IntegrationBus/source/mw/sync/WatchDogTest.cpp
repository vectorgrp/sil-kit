// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "WatchDog.hpp"

#include <chrono>
#include <functional>
#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ParticipantConfiguration.hpp"

#include "ib/mw/sync/string_utils.hpp"

namespace {

using namespace std::chrono_literals;

using namespace testing;

using namespace ib;
using namespace ib::mw;
using namespace ib::mw::sync;


class WatchDogTest : public testing::Test
{
protected:
    struct Callbacks
    {
        MOCK_METHOD1(WarnHandler, void(std::chrono::milliseconds));
        MOCK_METHOD1(ErrorHandler, void(std::chrono::milliseconds));
    };

protected:
    WatchDogTest()
    {

    }

protected:
    // ----------------------------------------
    // Helper Methods

protected:
    // ----------------------------------------
    // Members
    Callbacks callbacks;
};

TEST_F(WatchDogTest, throw_if_warn_timeout_is_zero)
{
    EXPECT_THROW(WatchDog(cfg::HealthCheck{ 0ms,10ms }), std::runtime_error);
}

TEST_F(WatchDogTest, throw_if_error_timeout_is_zero)
{
    EXPECT_THROW(WatchDog(cfg::HealthCheck{ 10ms,0ms }), std::runtime_error);
}

TEST_F(WatchDogTest, warn_after_timeout)
{
    
    WatchDog watchDog{ cfg::HealthCheck{10ms, std::chrono::milliseconds::max()} };
    watchDog.SetWarnHandler([this](auto timeout) { callbacks.WarnHandler(timeout); });

    watchDog.Start();

    EXPECT_CALL(callbacks, WarnHandler(_)).Times(1);
    std::this_thread::sleep_for(500ms);
}

TEST_F(WatchDogTest, error_after_timeout)
{
    WatchDog watchDog{ cfg::HealthCheck{10ms, 50ms} };
    watchDog.SetErrorHandler([this](auto timeout) { callbacks.ErrorHandler(timeout); });

    watchDog.Start();

    EXPECT_CALL(callbacks, ErrorHandler(_)).Times(1);
    std::this_thread::sleep_for(500ms);
}

TEST_F(WatchDogTest, warn_only_once)
{

    WatchDog watchDog{ cfg::HealthCheck{10ms, std::chrono::milliseconds::max()} };
    watchDog.SetWarnHandler([this](auto timeout) { callbacks.WarnHandler(timeout); });
    watchDog.SetErrorHandler([this](auto timeout) { callbacks.ErrorHandler(timeout); });

    watchDog.Start();

    EXPECT_CALL(callbacks, WarnHandler(_)).Times(1);
    EXPECT_CALL(callbacks, ErrorHandler(_)).Times(0);
    std::this_thread::sleep_for(500ms);
}

TEST_F(WatchDogTest, error_only_once)
{
    WatchDog watchDog{ cfg::HealthCheck{10ms, 50ms} };
    watchDog.SetErrorHandler([this](auto timeout) { callbacks.ErrorHandler(timeout); });

    watchDog.Start();

    EXPECT_CALL(callbacks, ErrorHandler(_)).Times(1);
    std::this_thread::sleep_for(500ms);
}

TEST_F(WatchDogTest, no_callback_if_reset_in_time)
{
    WatchDog watchDog{ cfg::HealthCheck{2000ms, 3000ms} };
    watchDog.SetWarnHandler([this](auto timeout) { callbacks.WarnHandler(timeout); });
    watchDog.SetErrorHandler([this](auto timeout) { callbacks.ErrorHandler(timeout); });

    watchDog.Start();
    EXPECT_CALL(callbacks, WarnHandler(_)).Times(0);
    EXPECT_CALL(callbacks, ErrorHandler(_)).Times(0);
    std::this_thread::sleep_for(1s);
    watchDog.Reset();
}

TEST_F(WatchDogTest, create_health_check_unconfigured)
{
    cfg::HealthCheck healthCheck;
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
TEST_F(WatchDogTest, create_health_check_configured)
{
    WatchDog watchDog{cfg::HealthCheck{2000ms, 3000ms}};
    watchDog.SetWarnHandler([this](auto timeout) {
        callbacks.WarnHandler(timeout);
    });
    watchDog.SetErrorHandler([this](auto timeout) {
        callbacks.ErrorHandler(timeout);
    });

    watchDog.Start();
    EXPECT_CALL(callbacks, WarnHandler(_)).Times(0);
    EXPECT_CALL(callbacks, ErrorHandler(_)).Times(0);
    std::this_thread::sleep_for(1s);
    watchDog.Reset();
}


} // anonymous namespace for test
