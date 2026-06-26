// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

#include "ITestFixture.hpp"
#include "silkit/services/pubsub/all.hpp"

#include "gtest/gtest.h"

namespace {

using namespace std::chrono_literals;
using namespace SilKit::Services::PubSub;
using namespace SilKit::Tests;

inline std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    using namespace std::chrono_literals;

    if (timestamp % 1ms == 0ns)
    {
        out << (timestamp / 1ms) << "ms";
    }
    else if (timestamp % 1us == 0ns)
    {
        out << (timestamp / 1us) << "us";
    }
    else
    {
        out << timestamp.count() << "ns";
    }
    return out;
}

static auto ToReadableList(const std::vector<std::chrono::nanoseconds>& values) -> std::string
{
    std::ostringstream out;
    out << "[ ";
    for (size_t i = 0; i < values.size(); ++i)
    {
        if (i > 0)
        {
            out << ", ";
        }
        out << values[i];
    }
    out << " ]";
    return out.str();
}

static auto ToReadableTimestamp(bool valid, std::chrono::nanoseconds value) -> std::string
{
    if (!valid)
    {
        return "<unknown>";
    }

    std::ostringstream out;
    out << value;
    return out.str();
}

static auto MissingFromActual(const std::vector<std::chrono::nanoseconds>& expected,
                              const std::vector<std::chrono::nanoseconds>& actual)
    -> std::vector<std::chrono::nanoseconds>
{
    std::vector<std::chrono::nanoseconds> missing;
    missing.reserve(expected.size());

    auto itActual = actual.begin();
    for (const auto expectedValue : expected)
    {
        itActual = std::find(itActual, actual.end(), expectedValue);
        if (itActual == actual.end())
        {
            missing.emplace_back(expectedValue);
        }
        else
        {
            ++itActual;
        }
    }

    return missing;
}

struct ITest_TimeCoordinator : ITest_SimTestHarness
{
    using ITest_SimTestHarness::ITest_SimTestHarness;
};

struct TimeCoordinatorTestState
{
    std::mutex mx;
    std::vector<std::chrono::nanoseconds> publisherSimTaskNows;
    std::vector<std::chrono::nanoseconds> subscriberSimTaskNows;
    std::vector<std::chrono::nanoseconds> timeCoordinatorSimTaskNows;

    std::atomic<bool> messagePublishedAt10ms{false};
    std::atomic<bool> messageReceived{false};
    std::atomic<bool> messageAvailableAt10msPlus1us{false};

    std::atomic<bool> messageReceivedAtValid{false};
    std::atomic<std::chrono::nanoseconds> messageReceivedAt{0ns};

    std::atomic<bool> pendingReceive{false};
};

// This test models a future central time-coordinator behavior for two synchronized participants
// (Publisher and Subscriber) with a nominal step size of 1ms.
//
// Expected schedule per cycle around 10ms:
//   0ms, 1ms, ..., 10ms, 10ms + 1us, 11ms, 12ms
// i.e. one temporary slowdown step of 1us is inserted after 10ms.
//
// Communication expectation:
//   - Publisher sends one PubSub message at 10ms.
//   - Subscriber expects that message to be available at 10ms + 1us.
//
// TimeCoordinator participant:
//   - Intends to change step sizes at runtime (1us at 10ms, 999us at 10ms+1us, 1ms at 11ms),
//     but these calls are currently missing in the API and therefore represented as commented placeholders.
TEST_F(ITest_TimeCoordinator, test_fixed_schedule_slowdown_by_time_coordinator)
{
    SetupFromParticipantList({"Publisher", "Subscriber", "TimeCoordinator"});

    auto state = std::make_shared<TimeCoordinatorTestState>();

    {
        auto&& simParticipant = _simTestHarness->GetParticipant("Publisher", "", false);
        auto&& participant = simParticipant->Participant();
        auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
        auto&& timeSyncService = lifecycleService->CreateTimeSyncService();
        auto&& publisher = participant->CreateDataPublisher("Pub", PubSubSpec{"TimeCoordinatorTopic", ""});

        timeSyncService->SetSimulationStepHandler([state, lifecycleService, publisher](auto now, auto) {
            {
                std::lock_guard<std::mutex> lock{state->mx};
                state->publisherSimTaskNows.emplace_back(now);
            }

            if (now == 10ms)
            {
                const std::vector<uint8_t> payload{0xAA};
                publisher->Publish(payload);
                state->messagePublishedAt10ms = true;
            }

            if (now >= 12ms)
            {
                lifecycleService->Stop("Test complete");
            }
        }, 1ms);
    }

    {
        auto&& simParticipant = _simTestHarness->GetParticipant("Subscriber", "", false);
        auto&& participant = simParticipant->Participant();
        auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
        auto&& timeSyncService = lifecycleService->CreateTimeSyncService();

        participant->CreateDataSubscriber(
            "Sub", PubSubSpec{"TimeCoordinatorTopic", ""},
            [state](IDataSubscriber*, const DataMessageEvent&) {
                state->pendingReceive = true;
            });

        timeSyncService->SetSimulationStepHandler([state](auto now, auto) {
            {
                std::lock_guard<std::mutex> lock{state->mx};
                state->subscriberSimTaskNows.emplace_back(now);
            }

            if (state->pendingReceive.exchange(false))
            {
                state->messageReceived = true;
                if (!state->messageReceivedAtValid.load())
                {
                    state->messageReceivedAt = now;
                    state->messageReceivedAtValid = true;
                }
            }

            if (now == (10ms + 1us))
            {
                state->messageAvailableAt10msPlus1us = state->messageReceived.load();
            }
        }, 1ms);
    }

    {
        auto&& simParticipant = _simTestHarness->GetParticipant("TimeCoordinator", "", false);
        auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
        auto&& timeSyncService = lifecycleService->CreateTimeSyncService();

        timeSyncService->SetSimulationStepHandler([state](auto now, auto) {
            {
                std::lock_guard<std::mutex> lock{state->mx};
                state->timeCoordinatorSimTaskNows.emplace_back(now);
            }

            if (now == 10ms)
            {
                // timeSyncService->SetStepSize(1us);
            }
            if (now == (10ms + 1us))
            {
                // timeSyncService->SetStepSize(999us);
            }
            if (now == 11ms)
            {
                // timeSyncService->SetStepSize(1ms);
            }
        }, 1ms);
    }

    const auto ok = _simTestHarness->Run(5s);
    ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";

    std::vector<std::chrono::nanoseconds> publisherNows;
    std::vector<std::chrono::nanoseconds> subscriberNows;
    std::vector<std::chrono::nanoseconds> timeCoordinatorNows;
    {
        std::lock_guard<std::mutex> lock{state->mx};
        publisherNows = state->publisherSimTaskNows;
        subscriberNows = state->subscriberSimTaskNows;
        timeCoordinatorNows = state->timeCoordinatorSimTaskNows;
    }

    const std::vector<std::chrono::nanoseconds> expectedNows{
        0ms,
        1ms,
        2ms,
        3ms,
        4ms,
        5ms,
        6ms,
        7ms,
        8ms,
        9ms,
        10ms,
        10ms + 1us,
        11ms,
        12ms,
    };

    if (publisherNows != expectedNows)
    {
        ADD_FAILURE() << "Publisher schedule mismatch\n"
                      << "Expected: " << ToReadableList(expectedNows) << "\n"
                      << "Actual  : " << ToReadableList(publisherNows) << "\n"
                      << "Missing : " << ToReadableList(MissingFromActual(expectedNows, publisherNows));
    }

    if (subscriberNows != expectedNows)
    {
        ADD_FAILURE() << "Subscriber schedule mismatch\n"
                      << "Expected: " << ToReadableList(expectedNows) << "\n"
                      << "Actual  : " << ToReadableList(subscriberNows) << "\n"
                      << "Missing : " << ToReadableList(MissingFromActual(expectedNows, subscriberNows));
    }

    if (timeCoordinatorNows != expectedNows)
    {
        ADD_FAILURE() << "TimeCoordinator schedule mismatch\n"
                      << "Expected: " << ToReadableList(expectedNows) << "\n"
                      << "Actual  : " << ToReadableList(timeCoordinatorNows) << "\n"
                      << "Missing : " << ToReadableList(MissingFromActual(expectedNows, timeCoordinatorNows));
    }

    EXPECT_TRUE(state->messagePublishedAt10ms.load());
    EXPECT_TRUE(state->messageReceived.load());
    EXPECT_TRUE(state->messageAvailableAt10msPlus1us.load())
        << "Message was not available at 10ms+1us as expected.\n"
        << "Published@10ms      : " << std::boolalpha << state->messagePublishedAt10ms.load() << "\n"
        << "Received(any time)  : " << std::boolalpha << state->messageReceived.load() << "\n"
        << "ReceivedAt          : "
        << ToReadableTimestamp(state->messageReceivedAtValid.load(), state->messageReceivedAt.load()) << "\n"
        << "Publisher schedule  : " << ToReadableList(publisherNows) << "\n"
        << "Subscriber schedule : " << ToReadableList(subscriberNows);
}
// Same as test_fixed_schedule_slowdown_by_time_coordinator,
// but the step size change is requested by the Publisher participant instead of a dedicated TimeCoordinator participant.
TEST_F(ITest_TimeCoordinator, test_fixed_schedule_slowdown_by_participant)
{
    SetupFromParticipantList({"Publisher", "Subscriber"});

    auto state = std::make_shared<TimeCoordinatorTestState>();

    {
        auto&& simParticipant = _simTestHarness->GetParticipant("Publisher", "", false);
        auto&& participant = simParticipant->Participant();
        auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
        auto&& timeSyncService = lifecycleService->CreateTimeSyncService();
        auto&& publisher = participant->CreateDataPublisher("Pub", PubSubSpec{"TimeCoordinatorTopic", ""});

        timeSyncService->SetSimulationStepHandler([state, lifecycleService, publisher](auto now, auto) {
            {
                std::lock_guard<std::mutex> lock{state->mx};
                state->publisherSimTaskNows.emplace_back(now);
            }

            if (now == 10ms)
            {
                const std::vector<uint8_t> payload{0xAA};
                publisher->Publish(payload);
                state->messagePublishedAt10ms = true;

                // timeSyncService->SetStepSize(1us);
            }
            if (now == (10ms + 1us))
            {
                // timeSyncService->SetStepSize(999us);
            }
            if (now == 11ms)
            {
                // timeSyncService->SetStepSize(1ms);
            }

            if (now >= 12ms)
            {
                lifecycleService->Stop("Test complete");
            }
        }, 1ms);
    }

    {
        auto&& simParticipant = _simTestHarness->GetParticipant("Subscriber", "", false);
        auto&& participant = simParticipant->Participant();
        auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
        auto&& timeSyncService = lifecycleService->CreateTimeSyncService();

        participant->CreateDataSubscriber(
            "Sub", PubSubSpec{"TimeCoordinatorTopic", ""},
            [state](IDataSubscriber*, const DataMessageEvent&) {
                state->pendingReceive = true;
            });

        timeSyncService->SetSimulationStepHandler([state](auto now, auto) {
            {
                std::lock_guard<std::mutex> lock{state->mx};
                state->subscriberSimTaskNows.emplace_back(now);
            }

            if (state->pendingReceive.exchange(false))
            {
                state->messageReceived = true;
                if (!state->messageReceivedAtValid.load())
                {
                    state->messageReceivedAt = now;
                    state->messageReceivedAtValid = true;
                }
            }

            if (now == (10ms + 1us))
            {
                state->messageAvailableAt10msPlus1us = state->messageReceived.load();
            }
        }, 1ms);
    }

    const auto ok = _simTestHarness->Run(5s);
    ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";

    std::vector<std::chrono::nanoseconds> publisherNows;
    std::vector<std::chrono::nanoseconds> subscriberNows;
    {
        std::lock_guard<std::mutex> lock{state->mx};
        publisherNows = state->publisherSimTaskNows;
        subscriberNows = state->subscriberSimTaskNows;
    }

    const std::vector<std::chrono::nanoseconds> expectedNows{
        0ms,
        1ms,
        2ms,
        3ms,
        4ms,
        5ms,
        6ms,
        7ms,
        8ms,
        9ms,
        10ms,
        10ms + 1us,
        11ms,
        12ms,
    };

    if (publisherNows != expectedNows)
    {
        ADD_FAILURE() << "Publisher schedule mismatch\n"
                      << "Expected: " << ToReadableList(expectedNows) << "\n"
                      << "Actual  : " << ToReadableList(publisherNows) << "\n"
                      << "Missing : " << ToReadableList(MissingFromActual(expectedNows, publisherNows));
    }

    if (subscriberNows != expectedNows)
    {
        ADD_FAILURE() << "Subscriber schedule mismatch\n"
                      << "Expected: " << ToReadableList(expectedNows) << "\n"
                      << "Actual  : " << ToReadableList(subscriberNows) << "\n"
                      << "Missing : " << ToReadableList(MissingFromActual(expectedNows, subscriberNows));
    }

    EXPECT_TRUE(state->messagePublishedAt10ms.load());
    EXPECT_TRUE(state->messageReceived.load());
    EXPECT_TRUE(state->messageAvailableAt10msPlus1us.load())
        << "Message was not available at 10ms+1us as expected.\n"
        << "Published@10ms      : " << std::boolalpha << state->messagePublishedAt10ms.load() << "\n"
        << "Received(any time)  : " << std::boolalpha << state->messageReceived.load() << "\n"
        << "ReceivedAt          : "
        << ToReadableTimestamp(state->messageReceivedAtValid.load(), state->messageReceivedAt.load()) << "\n"
        << "Publisher schedule  : " << ToReadableList(publisherNows) << "\n"
        << "Subscriber schedule : " << ToReadableList(subscriberNows);
}

} // namespace
