// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "services/orchestration/DeliveryInbox.hpp"

namespace {

using namespace std::chrono_literals;
using namespace testing;
using SilKit::Services::Orchestration::DeliveryInbox;

class Test_DeliveryInbox : public testing::Test
{
protected:
    void Schedule(std::chrono::nanoseconds due, const std::string& sender, const std::string& service,
                  const std::string& label)
    {
        inbox.Schedule(due, sender, service, [this, label] { delivered.push_back(label); });
    }

    DeliveryInbox inbox;
    std::vector<std::string> delivered;
};

TEST_F(Test_DeliveryInbox, holds_deliveries_until_their_step)
{
    Schedule(5ms, "P1", "CAN1", "a");
    Schedule(2ms, "P1", "CAN1", "b");

    inbox.ReleaseBefore(2ms);
    EXPECT_TRUE(delivered.empty());

    inbox.ReleaseBefore(3ms);
    EXPECT_THAT(delivered, ElementsAre("b"));
    EXPECT_EQ(inbox.Held(), 1u);

    inbox.ReleaseBefore(6ms);
    EXPECT_THAT(delivered, ElementsAre("b", "a"));
    EXPECT_EQ(inbox.Held(), 0u);
}

TEST_F(Test_DeliveryInbox, orders_by_due_time_sender_service_and_arrival)
{
    Schedule(1ms, "P2", "CAN1", "P2-first");
    Schedule(1ms, "P1", "ETH1", "P1-eth");
    Schedule(1ms, "P2", "CAN1", "P2-second");
    Schedule(1ms, "P1", "CAN1", "P1-can");
    Schedule(0ms, "P3", "CAN1", "earliest");

    inbox.ReleaseBefore(2ms);

    EXPECT_THAT(delivered, ElementsAre("earliest", "P1-can", "P1-eth", "P2-first", "P2-second"));
}

TEST_F(Test_DeliveryInbox, delivers_late_arrivals_right_away_and_counts_them)
{
    inbox.ReleaseBefore(3ms);

    Schedule(2ms, "P1", "CAN1", "late");
    EXPECT_THAT(delivered, ElementsAre("late"));
    EXPECT_EQ(inbox.LateDeliveries(), 1u);

    Schedule(3ms, "P1", "CAN1", "on time");
    EXPECT_THAT(delivered, ElementsAre("late"));
    EXPECT_EQ(inbox.LateDeliveries(), 1u);
}

} // anonymous namespace
