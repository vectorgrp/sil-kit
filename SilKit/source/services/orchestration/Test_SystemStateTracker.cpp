// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "services/orchestration/SystemStateTracker.hpp"

#include <algorithm>
#include <cstddef>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "silkit/services/orchestration/string_utils.hpp"

namespace {

using namespace testing;

using SilKit::Services::Orchestration::ParticipantState;
using SilKit::Services::Orchestration::ParticipantStatus;
using SilKit::Services::Orchestration::SystemState;

using VSilKit::SystemStateTracker;

// The states a participant passes through on its way to Running. Used as a common prelude so that
// the interesting part of each test starts from a well-defined SystemState.
const std::vector<ParticipantState> startupStates{
    ParticipantState::ServicesCreated, ParticipantState::CommunicationInitializing,
    ParticipantState::CommunicationInitialized, ParticipantState::ReadyToRun, ParticipantState::Running};

//! A single participant status update, i.e. one received ParticipantStatus message.
struct Update
{
    std::string participantName;
    ParticipantState state;
};

//! Thin, timing-free wrapper around the tracker. No clock is read: enterTime and refreshTime are
//! left default-constructed, so every test below is fully deterministic.
class Tracker
{
public:
    void Require(const std::vector<std::string>& participantNames)
    {
        _tracker.UpdateRequiredParticipants(participantNames);
    }

    void Send(const std::string& participantName, ParticipantState state)
    {
        ParticipantStatus status{};
        status.participantName = participantName;
        status.state = state;

        _tracker.UpdateParticipantStatus(status);
    }

    void Send(const Update& update)
    {
        Send(update.participantName, update.state);
    }

    void Send(const std::vector<Update>& updates)
    {
        for (const auto& update : updates)
        {
            Send(update);
        }
    }

    //! Drive a participant through ServicesCreated .. Running.
    void SendStartup(const std::string& participantName)
    {
        for (const auto state : startupStates)
        {
            Send(participantName, state);
        }
    }

    void Remove(const std::string& participantName)
    {
        _tracker.RemoveParticipant(participantName);
    }

    auto State() const -> SystemState
    {
        return _tracker.GetSystemState();
    }

private:
    SystemStateTracker _tracker;
};

auto FormatUpdates(const std::vector<Update>& updates) -> std::string
{
    std::stringstream ss;
    bool isFirst{true};
    for (const auto& update : updates)
    {
        ss << (isFirst ? "" : ", ") << update.participantName << "->" << update.state;
        isFirst = false;
    }
    return ss.str();
}

/*! Enumerate every interleaving of two per-participant update sequences that preserves the order
 *  within each sequence.
 *
 *  This models what participants actually observe: SIL Kit guarantees ordering per peer, but there
 *  is no global order across peers. Two participants therefore legitimately receive the very same
 *  set of status messages in different orders.
 */
auto MakeInterleavings(const std::vector<Update>& first, const std::vector<Update>& second)
    -> std::vector<std::vector<Update>>
{
    // 'selection' marks, for each slot of the merged sequence, whether it is taken from 'first'.
    std::vector<bool> selection(first.size() + second.size(), false);
    std::fill(selection.begin(), selection.begin() + static_cast<std::ptrdiff_t>(first.size()), true);
    std::sort(selection.begin(), selection.end());

    std::vector<std::vector<Update>> interleavings;

    do
    {
        std::vector<Update> interleaving;
        interleaving.reserve(selection.size());

        size_t firstIndex{0};
        size_t secondIndex{0};

        for (const bool takeFromFirst : selection)
        {
            interleaving.emplace_back(takeFromFirst ? first.at(firstIndex++) : second.at(secondIndex++));
        }

        interleavings.emplace_back(std::move(interleaving));
    } while (std::next_permutation(selection.begin(), selection.end()));

    return interleavings;
}

// ================================================================================================
//  The SystemState must be a function of the participant states, not of their arrival order.
// ================================================================================================

/*! The resulting SystemState must not depend on the order in which the ParticipantStatus messages
 *  arrive.
 *
 *  SystemStateTracker::ComputeSystemState switches on *which* participant moved and *to what*, and
 *  keeps the previous SystemState whenever no rule matches. The aggregate is therefore a function
 *  of the arrival order, not of the participant states - so two SystemMonitors observing the same
 *  participants can end up reporting different SystemStates, permanently.
 *
 *  Without virtual time synchronization there is no back pressure between participants, so a fast
 *  participant genuinely does run several states ahead of a slow one under load.
 *
 *  This test deliberately asserts *agreement* between all interleavings rather than one particular
 *  SystemState: it pins down the defect without prescribing a specific aggregation rule.
 */
TEST(Test_SystemStateTracker, system_state_must_not_depend_on_arrival_order)
{
    // 'A' runs ahead and has already stopped by the time 'B' reports Running.
    std::vector<Update> updatesOfA;
    for (const auto state : startupStates)
    {
        updatesOfA.push_back({"A", state});
    }
    updatesOfA.push_back({"A", ParticipantState::Stopping});
    updatesOfA.push_back({"A", ParticipantState::Stopped});

    std::vector<Update> updatesOfB;
    for (const auto state : startupStates)
    {
        updatesOfB.push_back({"B", state});
    }

    const auto interleavings = MakeInterleavings(updatesOfA, updatesOfB);
    ASSERT_FALSE(interleavings.empty());

    // Every interleaving ends in the same participant states, so every interleaving must end in the
    // same SystemState. Collect one witnessing interleaving per distinct outcome.
    std::map<SystemState, std::vector<Update>> witnessByState;

    for (const auto& interleaving : interleavings)
    {
        Tracker tracker;
        tracker.Require({"A", "B"});
        tracker.Send(interleaving);

        witnessByState.emplace(tracker.State(), interleaving);
    }

    std::stringstream outcomes;
    for (const auto& kv : witnessByState)
    {
        outcomes << "\n  SystemState::" << kv.first << " e.g. via [" << FormatUpdates(kv.second) << "]";
    }

    EXPECT_EQ(witnessByState.size(), 1u)
        << interleavings.size() << " interleavings of the same " << (updatesOfA.size() + updatesOfB.size())
        << " participant status updates produced " << witnessByState.size() << " different system states:"
        << outcomes.str();
}

/*! Minimal, readable witness for the interleaving property above.
 *
 *  Both trackers end up with the exact same participant states - A is Stopped, B is Running - but
 *  report different SystemStates:
 *
 *  | arrival order                                    | trace                                                                                                       | result today   |
 *  | ------------------------------------------------ | ----------------------------------------------------------------------------------------------------------- | -------------- |
 *  | A->Running, A->Stopping, A->Stopped, B->Running   | no rule matches while B lags behind at ReadyToRun; B->Running then matches PS::Running's {Running, Stopped}   | SS::Stopped    |
 *  | B->Running, A->Running, A->Stopping, A->Stopped   | SS::Running, then SS::Stopping; PS::Stopped requires all of {Stopped, ShuttingDown, Shutdown}, so B=Running   | SS::Stopping   |
 *  |                                                  | blocks the last step and the stale SS::Stopping is retained                                                  |                |
 */
TEST(Test_SystemStateTracker, same_participant_states_yield_same_system_state)
{
    Tracker aheadFirst;
    {
        aheadFirst.Require({"A", "B"});

        // Bring both participants to ReadyToRun in lock-step.
        for (const auto state : startupStates)
        {
            if (state == ParticipantState::Running)
            {
                break;
            }
            aheadFirst.Send("A", state);
            aheadFirst.Send("B", state);
        }
        ASSERT_EQ(aheadFirst.State(), SystemState::ReadyToRun);

        aheadFirst.Send({{"A", ParticipantState::Running},
                         {"A", ParticipantState::Stopping},
                         {"A", ParticipantState::Stopped},
                         {"B", ParticipantState::Running}});
    }

    Tracker laggingFirst;
    {
        laggingFirst.Require({"A", "B"});

        for (const auto state : startupStates)
        {
            if (state == ParticipantState::Running)
            {
                break;
            }
            laggingFirst.Send("A", state);
            laggingFirst.Send("B", state);
        }
        ASSERT_EQ(laggingFirst.State(), SystemState::ReadyToRun);

        laggingFirst.Send({{"B", ParticipantState::Running},
                           {"A", ParticipantState::Running},
                           {"A", ParticipantState::Stopping},
                           {"A", ParticipantState::Stopped}});
    }

    EXPECT_EQ(aheadFirst.State(), laggingFirst.State())
        << "both trackers saw A=Stopped and B=Running, but disagree on the system state";
}

// ================================================================================================
//  Required participants with unknown or removed status
// ================================================================================================

/*! A required participant whose state is unknown must not leave the system reported as Running.
 *
 *  SystemStateTracker::GetAnyRequiredParticipantState() picks *_requiredParticipants.begin(), i.e.
 *  an arbitrary element of an unordered_set, and uses it as the "who moved" input of
 *  ComputeSystemState. The choice is unrelated to what actually happened.
 *
 *  This fails for either possible pick, so the test does not depend on hash order:
 *   - picking "A" (Running): every ChangeToIfAllIn rule fails because B has no status, so the
 *     previous SystemState is retained;
 *   - picking "B" (no status, hence Invalid): 'case PS::Invalid' breaks out without recomputing.
 */
TEST(Test_SystemStateTracker, unknown_required_participant_must_not_report_running)
{
    Tracker tracker;
    tracker.Require({"A"});
    tracker.SendStartup("A");
    ASSERT_EQ(tracker.State(), SystemState::Running);

    // 'B' joins the set of required participants but has never reported a status.
    tracker.Require({"A", "B"});

    EXPECT_NE(tracker.State(), SystemState::Running)
        << "the state of required participant 'B' is unknown, so the system cannot be running";
}

/*! The SystemState must keep progressing after a participant has been removed.
 *
 *  RemoveParticipant erases the participant from the status cache, but the const
 *  GetParticipantStatus does not re-insert a default. ChangeToIfAllIn therefore returns false on
 *  the removed participant for every subsequent update, and the SystemState is pinned to whatever
 *  it happened to be at the time of removal. The IsEmpty() escape hatch never triggers, because
 *  the remaining participant is still cached.
 *
 *  Here the tracker freezes at SS::Stopping - the value it took when A reported Stopping while B
 *  was still Running - and never reaches SS::Shutdown, even though both required participants have
 *  reported Shutdown.
 */
TEST(Test_SystemStateTracker, system_state_must_still_progress_after_participant_removal)
{
    Tracker tracker;
    tracker.Require({"A", "B"});

    tracker.SendStartup("A");
    tracker.SendStartup("B");
    ASSERT_EQ(tracker.State(), SystemState::Running);

    tracker.Send({{"A", ParticipantState::Stopping},
                  {"A", ParticipantState::Stopped},
                  {"A", ParticipantState::ShuttingDown},
                  {"A", ParticipantState::Shutdown}});

    // 'A' shut down gracefully and disconnected.
    tracker.Remove("A");

    tracker.Send({{"B", ParticipantState::Stopping},
                  {"B", ParticipantState::Stopped},
                  {"B", ParticipantState::ShuttingDown},
                  {"B", ParticipantState::Shutdown}});

    EXPECT_EQ(tracker.State(), SystemState::Shutdown)
        << "both required participants reported Shutdown, but the system state froze at the value it "
           "had when 'A' was removed";
}

} // anonymous namespace
