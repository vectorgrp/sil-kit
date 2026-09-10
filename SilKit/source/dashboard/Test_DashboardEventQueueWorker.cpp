// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

/*! Tests for the dashboard's event batching.
 *
 *  The worker is driven synchronously: the queue is filled, then stopped, then the worker is run on
 *  the test's own thread. LockedQueue::Stop() makes DequeueAllInto() hand over whatever is queued
 *  and return false on the next call, so the worker processes everything and returns - no threads
 *  and no timing in these tests.
 */

#include "dashboard/EventQueueWorkerThread.hpp"

#include <atomic>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "core/mock/participant/MockParticipant.hpp"

#include "dashboard/Mocks/MockRestClient.hpp"

using namespace testing;
using SilKit::Dashboard::DashboardBulkUpdate;

namespace VSilKit {
namespace {

namespace orchestration = SilKit::Services::Orchestration;

constexpr uint64_t simulationId{42};
constexpr uint64_t otherSimulationId{43};

auto MakeParticipantStatus(const std::string& participantName, orchestration::ParticipantState state)
    -> orchestration::ParticipantStatus
{
    orchestration::ParticipantStatus status{};
    status.participantName = participantName;
    status.state = state;
    status.enterReason = "because";
    return status;
}

auto MakeServiceData() -> ServiceData
{
    ServiceData serviceData{};
    serviceData.discoveryType = SilKit::Core::Discovery::ServiceDiscoveryEvent::Type::ServiceCreated;
    serviceData.serviceDescriptor.SetServiceName("aService");
    return serviceData;
}

class Test_DashboardEventQueueWorker : public Test
{
public:
    void SetUp() override
    {
        EXPECT_CALL(_dummyLogger, GetLogLevel).WillRepeatedly(Return(SilKit::Services::Logging::Level::Off));
    }

    template <typename T>
    void Enqueue(const std::string& simulationName, T&& payload)
    {
        _queue.Enqueue(SilKitEvent{simulationName, std::forward<T>(payload)});
    }

    void EnqueueSimulationStart(const std::string& simulationName)
    {
        Enqueue(simulationName, SimulationStart{"silkit://localhost:8500/" + simulationName, 1000});
    }

    //! Stops the queue so the worker drains it, then runs the worker to completion inline.
    void RunWorkerToCompletion()
    {
        _queue.Stop();
        EventQueueWorkerThread worker{&_dummyLogger, &_restClient, &_queue, &_abort};
        worker();
    }

    NiceMock<SilKit::Core::Tests::MockLogger> _dummyLogger;
    StrictMock<MockRestClient> _restClient;
    LockedQueue<SilKitEvent> _queue;
    std::atomic<bool> _abort{false};
};

// --- simulation lifecycle ---------------------------------------------------------------------

TEST_F(Test_DashboardEventQueueWorker, SimulationStart_CreatesTheSimulation)
{
    EXPECT_CALL(_restClient, OnSimulationStart("silkit://localhost:8500/sim", 1000)).WillOnce(Return(simulationId));

    EnqueueSimulationStart("sim");
    RunWorkerToCompletion();
}

/*! Queuing lets the same simulation be announced twice, which must not create it twice - a second
 *  id would split the simulation's data across two dashboard entries.
 */
TEST_F(Test_DashboardEventQueueWorker, SimulationStart_Twice_CreatesTheSimulationOnce)
{
    EXPECT_CALL(_restClient, OnSimulationStart(_, _)).WillOnce(Return(simulationId));

    EnqueueSimulationStart("sim");
    EnqueueSimulationStart("sim");
    RunWorkerToCompletion();
}

TEST_F(Test_DashboardEventQueueWorker, SimulationStart_ForDistinctNames_CreatesEachSimulation)
{
    EXPECT_CALL(_restClient, OnSimulationStart("silkit://localhost:8500/a", _)).WillOnce(Return(simulationId));
    EXPECT_CALL(_restClient, OnSimulationStart("silkit://localhost:8500/b", _)).WillOnce(Return(otherSimulationId));

    EnqueueSimulationStart("a");
    EnqueueSimulationStart("b");
    RunWorkerToCompletion();
}

//! Id 0 is the failure sentinel, so nothing more may be sent for that simulation.
TEST_F(Test_DashboardEventQueueWorker, SimulationStart_Failing_DropsTheSimulationsEvents)
{
    EXPECT_CALL(_restClient, OnSimulationStart(_, _)).WillOnce(Return(0));
    EXPECT_CALL(_restClient, OnBulkUpdate(_, _)).Times(0);

    EnqueueSimulationStart("sim");
    Enqueue("sim", orchestration::ParticipantConnectionInformation{"P1"});
    RunWorkerToCompletion();
}

//! An event for a simulation that was never started is dropped, not attributed to another id.
TEST_F(Test_DashboardEventQueueWorker, EventsForAnUnknownSimulation_AreDropped)
{
    EXPECT_CALL(_restClient, OnBulkUpdate(_, _)).Times(0);

    Enqueue("neverStarted", orchestration::ParticipantConnectionInformation{"P1"});
    RunWorkerToCompletion();
}

// --- batching ---------------------------------------------------------------------------------

TEST_F(Test_DashboardEventQueueWorker, Events_AreBatchedIntoASingleBulkUpdate)
{
    EXPECT_CALL(_restClient, OnSimulationStart(_, _)).WillOnce(Return(simulationId));

    DashboardBulkUpdate captured;
    EXPECT_CALL(_restClient, OnBulkUpdate(simulationId, _))
        .WillOnce(WithArg<1>([&captured](const DashboardBulkUpdate& update) {
        captured.participantConnectionInformations = update.participantConnectionInformations;
        captured.participantStatuses = update.participantStatuses;
        captured.systemStates = update.systemStates;
        captured.serviceDatas = update.serviceDatas;
        captured.stopped = update.stopped;
    }));

    EnqueueSimulationStart("sim");
    Enqueue("sim", orchestration::ParticipantConnectionInformation{"P1"});
    Enqueue("sim", orchestration::ParticipantConnectionInformation{"P2"});
    Enqueue("sim", MakeParticipantStatus("P1", orchestration::ParticipantState::Running));
    Enqueue("sim", orchestration::SystemState::Running);
    Enqueue("sim", MakeServiceData());
    RunWorkerToCompletion();

    EXPECT_EQ(captured.participantConnectionInformations.size(), 2u);
    EXPECT_EQ(captured.participantStatuses.size(), 1u);
    EXPECT_EQ(captured.systemStates.size(), 1u);
    EXPECT_EQ(captured.serviceDatas.size(), 1u);
    EXPECT_FALSE(captured.stopped.has_value());
}

TEST_F(Test_DashboardEventQueueWorker, EachSimulation_GetsItsOwnBulkUpdate)
{
    EXPECT_CALL(_restClient, OnSimulationStart("silkit://localhost:8500/a", _)).WillOnce(Return(simulationId));
    EXPECT_CALL(_restClient, OnSimulationStart("silkit://localhost:8500/b", _)).WillOnce(Return(otherSimulationId));

    size_t participantsForA{0};
    size_t participantsForB{0};
    EXPECT_CALL(_restClient, OnBulkUpdate(simulationId, _)).WillOnce(WithArg<1>([&](const DashboardBulkUpdate& u) {
        participantsForA = u.participantConnectionInformations.size();
    }));
    EXPECT_CALL(_restClient, OnBulkUpdate(otherSimulationId, _)).WillOnce(WithArg<1>([&](const DashboardBulkUpdate& u) {
        participantsForB = u.participantConnectionInformations.size();
    }));

    EnqueueSimulationStart("a");
    EnqueueSimulationStart("b");
    Enqueue("a", orchestration::ParticipantConnectionInformation{"P1"});
    Enqueue("b", orchestration::ParticipantConnectionInformation{"P2"});
    Enqueue("b", orchestration::ParticipantConnectionInformation{"P3"});
    RunWorkerToCompletion();

    EXPECT_EQ(participantsForA, 1u);
    EXPECT_EQ(participantsForB, 2u);
}

//! Nothing to report means no request at all, rather than an empty bulk update every batch.
TEST_F(Test_DashboardEventQueueWorker, ASimulationWithNothingToReport_SendsNoBulkUpdate)
{
    EXPECT_CALL(_restClient, OnSimulationStart(_, _)).WillOnce(Return(simulationId));
    EXPECT_CALL(_restClient, OnBulkUpdate(_, _)).Times(0);

    EnqueueSimulationStart("sim");
    RunWorkerToCompletion();
}

/*! A new simulation start flushes first, so events accumulated before it are not attributed to a
 *  batch that also contains the new simulation.
 */
TEST_F(Test_DashboardEventQueueWorker, ASimulationStart_FlushesWhatWasAlreadyAccumulated)
{
    InSequence sequence;

    EXPECT_CALL(_restClient, OnSimulationStart("silkit://localhost:8500/a", _)).WillOnce(Return(simulationId));
    EXPECT_CALL(_restClient, OnBulkUpdate(simulationId, _));
    EXPECT_CALL(_restClient, OnSimulationStart("silkit://localhost:8500/b", _)).WillOnce(Return(otherSimulationId));

    EnqueueSimulationStart("a");
    Enqueue("a", orchestration::ParticipantConnectionInformation{"P1"});
    EnqueueSimulationStart("b");
    RunWorkerToCompletion();
}

// --- simulation end ---------------------------------------------------------------------------

TEST_F(Test_DashboardEventQueueWorker, SimulationEnd_SetsStopped)
{
    EXPECT_CALL(_restClient, OnSimulationStart(_, _)).WillOnce(Return(simulationId));

    std::optional<uint64_t> stopped;
    EXPECT_CALL(_restClient, OnBulkUpdate(simulationId, _))
        .WillOnce(WithArg<1>([&stopped](const DashboardBulkUpdate& u) { stopped = u.stopped; }));

    EnqueueSimulationStart("sim");
    Enqueue("sim", SimulationEnd{7777});
    RunWorkerToCompletion();

    ASSERT_TRUE(stopped.has_value());
    EXPECT_EQ(*stopped, 7777u);
}

//! After the end the name is forgotten, so late events are dropped rather than reusing the id.
TEST_F(Test_DashboardEventQueueWorker, EventsAfterSimulationEnd_AreDropped)
{
    EXPECT_CALL(_restClient, OnSimulationStart(_, _)).WillOnce(Return(simulationId));
    EXPECT_CALL(_restClient, OnBulkUpdate(simulationId, _)).Times(1);

    EnqueueSimulationStart("sim");
    Enqueue("sim", SimulationEnd{1});
    Enqueue("sim", orchestration::ParticipantConnectionInformation{"tooLate"});
    RunWorkerToCompletion();
}

//! A simulation may start again under the same name after it ended.
TEST_F(Test_DashboardEventQueueWorker, ASimulationNameCanBeReusedAfterItEnded)
{
    EXPECT_CALL(_restClient, OnSimulationStart(_, _))
        .WillOnce(Return(simulationId))
        .WillOnce(Return(otherSimulationId));
    EXPECT_CALL(_restClient, OnBulkUpdate(simulationId, _));
    EXPECT_CALL(_restClient, OnBulkUpdate(otherSimulationId, _));

    EnqueueSimulationStart("sim");
    Enqueue("sim", SimulationEnd{1});
    EnqueueSimulationStart("sim");
    Enqueue("sim", orchestration::ParticipantConnectionInformation{"P1"});
    RunWorkerToCompletion();
}

// --- metrics ----------------------------------------------------------------------------------

//! Metrics have their own endpoint and are sent as they arrive rather than folded into the batch.
TEST_F(Test_DashboardEventQueueWorker, MetricsUpdates_AreSentImmediatelyAndNotBatched)
{
    EXPECT_CALL(_restClient, OnSimulationStart(_, _)).WillOnce(Return(simulationId));
    EXPECT_CALL(_restClient, OnMetricsUpdate(simulationId, "P1", _)).Times(1);
    EXPECT_CALL(_restClient, OnBulkUpdate(_, _)).Times(0);

    EnqueueSimulationStart("sim");
    Enqueue("sim", MetricsUpdatePair{"P1", MetricsUpdate{}});
    RunWorkerToCompletion();
}

// --- abort ------------------------------------------------------------------------------------

/*! Abort still flushes what was already accumulated.
 *
 *  The shutdown grace period in DashboardInstance exists precisely so the last accumulated update
 *  reaches the dashboard, which only works if the worker sends it on the way out. The flag is
 *  tripped from the metrics call, which is the one request issued while a batch is accumulating.
 */
TEST_F(Test_DashboardEventQueueWorker, Abort_FlushesWhatWasAccumulated)
{
    EXPECT_CALL(_restClient, OnSimulationStart(_, _)).WillOnce(Return(simulationId));
    EXPECT_CALL(_restClient, OnMetricsUpdate(simulationId, "P1", _)).WillOnce([this](auto&&...) {
        _abort.store(true);
    });

    size_t participants{0};
    EXPECT_CALL(_restClient, OnBulkUpdate(simulationId, _)).WillOnce(WithArg<1>([&](const DashboardBulkUpdate& update) {
        participants = update.participantConnectionInformations.size();
    }));

    EnqueueSimulationStart("sim");
    Enqueue("sim", orchestration::ParticipantConnectionInformation{"P1"});
    Enqueue("sim", MetricsUpdatePair{"P1", MetricsUpdate{}}); // trips the abort flag
    Enqueue("sim", orchestration::ParticipantConnectionInformation{"neverProcessed"});

    RunWorkerToCompletion();

    // P1 was accumulated before the abort and must still have been sent; the event queued after the
    // flag was tripped must not have been.
    EXPECT_EQ(participants, 1u);
}

TEST_F(Test_DashboardEventQueueWorker, Abort_BeforeAnyEvent_SendsNothing)
{
    EXPECT_CALL(_restClient, OnSimulationStart(_, _)).Times(0);
    EXPECT_CALL(_restClient, OnBulkUpdate(_, _)).Times(0);

    EnqueueSimulationStart("sim");
    _abort.store(true);

    RunWorkerToCompletion();
}

// --- robustness -------------------------------------------------------------------------------

//! A throwing REST client must not escape the worker; DashboardInstance relies on it returning.
TEST_F(Test_DashboardEventQueueWorker, AThrowingRestClient_DoesNotEscapeTheWorker)
{
    EXPECT_CALL(_restClient, OnSimulationStart(_, _)).WillOnce(Throw(std::runtime_error{"boom"}));

    EnqueueSimulationStart("sim");

    EXPECT_NO_THROW(RunWorkerToCompletion());
}

/*! One unmappable event must not end all reporting.
 *
 *  This is the regression that made the dashboard look frozen: the mapper throws on data it has no
 *  representation for, that escaped ProcessEvents(), and the worker thread ended. Nothing restarts
 *  it, so every later event was queued to a consumer that no longer existed and the dashboard kept
 *  showing whatever had arrived before - no metrics, no attributes, no status changes.
 */
TEST_F(Test_DashboardEventQueueWorker, AFailedMetricsUpdate_DoesNotStopLaterEvents)
{
    InSequence sequence;
    EXPECT_CALL(_restClient, OnSimulationStart(_, _)).WillOnce(Return(simulationId));
    EXPECT_CALL(_restClient, OnMetricsUpdate(simulationId, "P1", _))
        .WillOnce(Throw(SilKit::SilKitError{"Unexpected controller type Something"}));
    EXPECT_CALL(_restClient, OnMetricsUpdate(simulationId, "P2", _)).Times(1);
    EXPECT_CALL(_restClient, OnBulkUpdate(simulationId, _)).Times(1);

    EnqueueSimulationStart("sim");
    Enqueue("sim", MetricsUpdatePair{"P1", MetricsUpdate{}});
    Enqueue("sim", MetricsUpdatePair{"P2", MetricsUpdate{}});
    Enqueue("sim", orchestration::ParticipantConnectionInformation{"P1"});

    RunWorkerToCompletion();
}

//! A bulk update that cannot be sent must not be retried forever, blocking everything behind it.
TEST_F(Test_DashboardEventQueueWorker, AFailedBulkUpdate_IsDroppedRatherThanRetried)
{
    EXPECT_CALL(_restClient, OnSimulationStart(_, _)).WillOnce(Return(simulationId));

    std::vector<std::string> secondFlush;
    EXPECT_CALL(_restClient, OnBulkUpdate(simulationId, _))
        .Times(2)
        // The first flush queues the next batch, ends the queue and then fails.
        .WillOnce([&](uint64_t, const DashboardBulkUpdate&) {
        Enqueue("sim", orchestration::ParticipantConnectionInformation{"P2"});
        _queue.Stop();
        throw SilKit::SilKitError{"cannot map this"};
    }).WillOnce(WithArg<1>([&](const DashboardBulkUpdate& update) {
        for (const auto& connection : update.participantConnectionInformations)
        {
            secondFlush.push_back(connection.participantName);
        }
    }));

    EnqueueSimulationStart("sim");
    Enqueue("sim", orchestration::ParticipantConnectionInformation{"P1"});

    EventQueueWorkerThread worker{&_dummyLogger, &_restClient, &_queue, &_abort};
    worker();

    // P1 is gone with the update that could not be sent, rather than being retried on every
    // following flush - which would have stalled everything queued behind it.
    EXPECT_THAT(secondFlush, ElementsAre("P2"));
}

//! Every event after a failure is still processed, so the queue cannot grow without a consumer.
TEST_F(Test_DashboardEventQueueWorker, AFailedSimulationStart_DoesNotStopLaterSimulations)
{
    EXPECT_CALL(_restClient, OnSimulationStart("silkit://localhost:8500/bad", _))
        .WillOnce(Throw(std::runtime_error{"boom"}));
    EXPECT_CALL(_restClient, OnSimulationStart("silkit://localhost:8500/good", _)).WillOnce(Return(simulationId));
    EXPECT_CALL(_restClient, OnBulkUpdate(simulationId, _)).Times(1);

    EnqueueSimulationStart("bad");
    EnqueueSimulationStart("good");
    Enqueue("good", orchestration::ParticipantConnectionInformation{"P1"});

    RunWorkerToCompletion();
}

} // namespace
} // namespace VSilKit
