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

#include "ITest_MultiThreadedParticipants.hpp"

#include <list>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

class ITest_HopOnHopOff : public ITest_MultiThreadedParticipants
{
};

TEST_F(ITest_HopOnHopOff, test_GracefulShutdown)
{
    RunRegistry();

    std::list<TestParticipant> syncParticipants;
    syncParticipants.push_back({"SyncParticipant1", TimeMode::Sync, OperationMode::Coordinated});
    syncParticipants.push_back({"SyncParticipant2", TimeMode::Sync, OperationMode::Coordinated});

    std::list<TestParticipant> monitorParticipants;
    monitorParticipants.push_back({"MonitorParticipant1", TimeMode::Async, OperationMode::Autonomous});

    expectedReceptions = syncParticipants.size() + monitorParticipants.size();

    std::vector<std::string> required{};
    for (auto&& p : syncParticipants)
    {
        required.push_back(p.name);
    }
    RunSystemController(required);

    RunParticipants(monitorParticipants);
    RunParticipants(syncParticipants);

    // Each participant knows that all other participants have started.
    for (auto& p : syncParticipants)
    {
        p.AwaitCommunication();
    }
    for (auto& p : monitorParticipants)
    {
        p.AwaitCommunication();
    }
    systemControllerParticipant.AwaitRunning();

    // Stop via system controller participant
    StopSystemController();

    // Wait for coordinated to end
    JoinParticipantThreads(_participantThreads_Sync_Coordinated);

    // Expect shutdown state
    EXPECT_EQ(monitorParticipants.front().participantStates["SyncParticipant1"], ParticipantState::Shutdown);
    EXPECT_EQ(monitorParticipants.front().participantStates["SyncParticipant2"], ParticipantState::Shutdown);

    monitorParticipants.front().i.stopRequested = true;
    JoinParticipantThreads(_participantThreads_Async_Autonomous);

    StopRegistry();
}


TEST_F(ITest_HopOnHopOff, test_Async_HopOnHopOff_ToSynced)
{
    RunRegistry();

    std::list<TestParticipant> syncParticipants;
    syncParticipants.push_back({"SyncParticipant1", TimeMode::Sync, OperationMode::Coordinated});
    syncParticipants.push_back({"SyncParticipant2", TimeMode::Sync, OperationMode::Coordinated});

    std::list<TestParticipant> asyncParticipants;
    asyncParticipants.push_back({"AsyncParticipant1", TimeMode::Async, OperationMode::Invalid});
    asyncParticipants.push_back({"AsyncParticipant2", TimeMode::Async, OperationMode::Invalid});
    expectedReceptions = syncParticipants.size() + asyncParticipants.size();

    std::vector<std::string> required{};
    for (auto&& p : syncParticipants)
    {
        required.push_back(p.name);
    }
    RunSystemController(required);

    RunParticipants(syncParticipants);

    if (verbose)
    {
        std::cout << "Await simtime progress" << std::endl;
    }
    // Await simtime progress
    for (auto& p : syncParticipants)
    {
        auto futureStatus = p.simtimePassedPromise.get_future().wait_for(communicationTimeout);
        EXPECT_EQ(futureStatus, std::future_status::ready)
            << "Test Failure: Sync participants didn't achieve sim time barrier";
    }

    for (int i = 0; i < 3; i++)
    {
        if (verbose)
        {
            std::cout << ">> Cycle " << i + 1 << "/3" << std::endl;
            std::cout << ">> Hop on async participants" << std::endl;
        }

        // Hop on with async participant
        RunParticipants(asyncParticipants);

        if (verbose)
        {
            std::cout << ">> Await successful communication of async/sync participants" << std::endl;
        }
        // Await successful communication of async/sync participants
        for (auto& p : syncParticipants)
            p.AwaitCommunication();
        for (auto& p : asyncParticipants)
            p.AwaitCommunication();

        if (verbose)
        {
            std::cout << ">> Hop off async participants" << std::endl;
        }
        // Hop off: Stop while-loop of async participants
        for (auto& p : asyncParticipants)
            p.i._runAsync = false;
        JoinParticipantThreads(_participantThreads_Async_Invalid);

        // Reset communication and wait for reception once more for remaining sync participants
        expectedReceptions = syncParticipants.size();
        for (auto& p : syncParticipants)
            p.ResetReception();

        if (verbose)
        {
            std::cout << ">> Await successful communication of remaining sync participants" << std::endl;
        }
        for (auto& p : syncParticipants)
            p.AwaitCommunication();

        // Reset communication to repeat the cycle
        expectedReceptions = syncParticipants.size() + asyncParticipants.size();
        for (auto& p : syncParticipants)
            p.ResetReception();
        for (auto& p : asyncParticipants)
            p.ResetReception();
    }
    if (verbose)
    {
        std::cout << ">> Cycles done" << std::endl;
    }

    // Shutdown coordinated participants
    for (auto& p : syncParticipants)
        p.lifecycleService->Stop("Hop Off");
    JoinParticipantThreads(_participantThreads_Sync_Coordinated);

    StopSystemController();
    StopRegistry();
}

TEST_F(ITest_HopOnHopOff, test_Async_reconnect_first_joined)
{
    std::list<TestParticipant> asyncParticipants1;
    asyncParticipants1.push_back({"AsyncParticipant1", TimeMode::Async, OperationMode::Invalid});
    std::list<TestParticipant> asyncParticipants2;
    asyncParticipants2.push_back({"AsyncParticipant2", TimeMode::Async, OperationMode::Invalid});
    expectedReceptions = asyncParticipants1.size() + asyncParticipants2.size();

    // No lifecycle, only registry needed (no systemController)
    RunRegistry();

    for (int i = 0; i < 3; i++)
    {
        // Start with asyncParticipant1
        RunParticipants(asyncParticipants1);
        // Async2 is second
        RunParticipants(asyncParticipants2);

        // Await successful communication of async participants
        for (auto& p : asyncParticipants1)
            p.AwaitCommunication();
        for (auto& p : asyncParticipants2)
            p.AwaitCommunication();

        // Reset communication
        expectedReceptions = 2;
        for (auto& p : asyncParticipants1)
            p.ResetReception();
        for (auto& p : asyncParticipants2)
            p.ResetReception();

        // Hop off with asyncParticipants1
        asyncParticipants1.front().i._runAsync = false;
        _participantThreads_Async_Invalid[0].WaitForThreadExit();

        // Reconnect with asyncParticipant1
        RunParticipants(asyncParticipants1);

        // Await successful communication of async participants
        for (auto& p : asyncParticipants1)
            p.AwaitCommunication();
        for (auto& p : asyncParticipants2)
            p.AwaitCommunication();

        // Reset communication
        expectedReceptions = 2;
        for (auto& p : asyncParticipants1)
            p.ResetReception();
        for (auto& p : asyncParticipants2)
            p.ResetReception();

        // Disconnect with both
        for (auto& p : asyncParticipants1)
            p.i._runAsync = false;
        for (auto& p : asyncParticipants2)
            p.i._runAsync = false;

        JoinParticipantThreads(_participantThreads_Async_Invalid);
    }

    StopRegistry();
}

TEST_F(ITest_HopOnHopOff, test_Async_reconnect_second_joined)
{
    std::list<TestParticipant> asyncParticipants1;
    asyncParticipants1.push_back({"AsyncParticipant1", TimeMode::Async, OperationMode::Invalid});
    std::list<TestParticipant> asyncParticipants2;
    asyncParticipants2.push_back({"AsyncParticipant2", TimeMode::Async, OperationMode::Invalid});
    expectedReceptions = asyncParticipants1.size() + asyncParticipants2.size();

    // No lifecycle, only registry needed (no systemController)
    RunRegistry();

    for (int i = 0; i < 3; i++)
    {
        // Start with asyncParticipant1
        RunParticipants(asyncParticipants1);
        // Async2 is second
        RunParticipants(asyncParticipants2);

        // Await successful communication of async participants
        for (auto& p : asyncParticipants1)
            p.AwaitCommunication();
        for (auto& p : asyncParticipants2)
            p.AwaitCommunication();

        // Reset communication
        expectedReceptions = 2;
        for (auto& p : asyncParticipants1)
            p.ResetReception();
        for (auto& p : asyncParticipants2)
            p.ResetReception();

        // Hop off with asyncParticipants2
        asyncParticipants2.front().i._runAsync = false;
        _participantThreads_Async_Invalid[1].WaitForThreadExit();
        //participantThreads_Async_Invalid.erase(participantThreads_Async_Invalid.begin()+1);

        // Reconnect with asyncParticipant2
        RunParticipants(asyncParticipants2);

        // Await successful communication of async participants
        for (auto& p : asyncParticipants1)
            p.AwaitCommunication();
        for (auto& p : asyncParticipants2)
            p.AwaitCommunication();

        // Reset communication
        expectedReceptions = 2;
        for (auto& p : asyncParticipants1)
            p.ResetReception();
        for (auto& p : asyncParticipants2)
            p.ResetReception();

        // Disconnect with both
        for (auto& p : asyncParticipants1)
            p.i._runAsync = false;
        for (auto& p : asyncParticipants2)
            p.i._runAsync = false;

        JoinParticipantThreads(_participantThreads_Async_Invalid);
    }

    StopRegistry();
}

TEST_F(ITest_HopOnHopOff, test_Async_HopOnHopOff_ToEmpty)
{
    RunRegistry();

    std::list<TestParticipant> asyncParticipants;
    asyncParticipants.push_back({"AsyncParticipant1", TimeMode::Async, OperationMode::Invalid});
    asyncParticipants.push_back({"AsyncParticipant2", TimeMode::Async, OperationMode::Invalid});
    expectedReceptions = asyncParticipants.size();

    for (int i = 0; i < 3; i++)
    {
        // Hop on with async participant on empty sim
        RunParticipants(asyncParticipants);

        // Await successful communication of async/sync participants
        for (auto& p : asyncParticipants)
            p.AwaitCommunication();

        // Hop off async participants
        for (auto& p : asyncParticipants)
            p.i._runAsync = false;

        JoinParticipantThreads(_participantThreads_Async_Invalid);

        // Reset communication to repeat the cycle
        for (auto& p : asyncParticipants)
            p.ResetReception();
    }

    StopRegistry();
}

TEST_F(ITest_HopOnHopOff, test_HopOnHopOff_Autonomous_To_Coordinated)
{
    RunRegistry();

    // The coordinated and required participants
    std::list<TestParticipant> syncParticipantsCoordinated;
    syncParticipantsCoordinated.push_back({"SyncCoordinated1", TimeMode::Sync, OperationMode::Coordinated});
    syncParticipantsCoordinated.push_back({"SyncCoordinated2", TimeMode::Sync, OperationMode::Coordinated});

    // The autonomous, non-required participants that will hop on/off
    std::list<TestParticipant> syncParticipantsAutonomous;
    syncParticipantsAutonomous.push_back({"SyncAutonomous1", TimeMode::Sync, OperationMode::Autonomous});
    syncParticipantsAutonomous.push_back({"SyncAutonomous2", TimeMode::Sync, OperationMode::Autonomous});

    // Additional async, non-required participants for testing the mixture
    std::list<TestParticipant> asyncParticipants;
    asyncParticipants.push_back({"ASync1", TimeMode::Async, OperationMode::Invalid});

    // Coordinated are required
    std::vector<std::string> required{};
    for (auto&& p : syncParticipantsCoordinated)
    {
        required.push_back(p.name);
    }
    RunSystemController(required);

    // Run coordinated
    expectedReceptions = syncParticipantsCoordinated.size() + asyncParticipants.size();
    RunParticipants(asyncParticipants);
    RunParticipants(syncParticipantsCoordinated);

    for (int i = 0; i < 3; i++)
    {
        if (verbose)
        {
            std::cout << ">> Cycle " << i + 1 << "/3" << std::endl;
        }

        if (verbose)
        {
            std::cout << ">> Await successful communication of coordinated participants" << std::endl;
        }

        // Await successful communication of avaliable participants (coordinated, async)
        for (auto& p : syncParticipantsCoordinated)
            p.AwaitCommunication();
        for (auto& p : asyncParticipants)
            p.AwaitCommunication();

        // Reset communication
        expectedReceptions =
            syncParticipantsCoordinated.size() + syncParticipantsAutonomous.size() + asyncParticipants.size();
        for (auto& p : syncParticipantsCoordinated)
            p.ResetReception();
        for (auto& p : asyncParticipants)
            p.ResetReception();

        if (verbose)
        {
            std::cout << ">> Hop on with autonomous participants" << std::endl;
        }

        // Hop on with autonomous
        RunParticipants(syncParticipantsAutonomous);

        if (verbose)
        {
            std::cout << ">> Await successful communication of all participants" << std::endl;
        }
        for (auto& p : syncParticipantsCoordinated)
            p.AwaitCommunication();
        for (auto& p : syncParticipantsAutonomous)
            p.AwaitCommunication();
        for (auto& p : asyncParticipants)
            p.AwaitCommunication();

        if (verbose)
        {
            std::cout << ">> Hop off with autonomous participants" << std::endl;
        }

        for (auto& p : syncParticipantsAutonomous)
            p.lifecycleService->Stop("Hop Off");

        JoinParticipantThreads(_participantThreads_Sync_AutonomousA);

        // Reset communication to repeat the cycle
        expectedReceptions = syncParticipantsCoordinated.size() + asyncParticipants.size();
        for (auto& p : syncParticipantsCoordinated)
            p.ResetReception();
        for (auto& p : syncParticipantsAutonomous)
            p.ResetReception();
        for (auto& p : asyncParticipants)
            p.ResetReception();
    }

    // Shutdown async participants
    for (auto& p : asyncParticipants)
        p.i._runAsync = false;
    JoinParticipantThreads(_participantThreads_Async_Invalid);

    // Shutdown coordinated participants
    for (auto& p : syncParticipantsCoordinated)
        p.lifecycleService->Stop("Hop Off");
    JoinParticipantThreads(_participantThreads_Sync_Coordinated);

    StopSystemController();
    StopRegistry();
}

TEST_F(ITest_HopOnHopOff, test_HopOnHopOff_Autonomous_To_Autonomous)
{
    RunRegistry();

    // The autonomous, non-required participant that is there from the start
    std::list<TestParticipant> syncParticipantsAutonomousA;
    syncParticipantsAutonomousA.push_back({"SyncAutonomousA1", TimeMode::Sync, OperationMode::Autonomous});

    // The autonomous, non-required participants that will hop on/off
    std::list<TestParticipant> syncParticipantsAutonomousB;
    syncParticipantsAutonomousB.push_back({"SyncAutonomousB1", TimeMode::Sync, OperationMode::Autonomous});
    syncParticipantsAutonomousB.push_back({"SyncAutonomousB2", TimeMode::Sync, OperationMode::Autonomous});
    syncParticipantsAutonomousB.push_back({"SyncAutonomousB3", TimeMode::Sync, OperationMode::Autonomous});
    syncParticipantsAutonomousB.push_back({"SyncAutonomousB4", TimeMode::Sync, OperationMode::Autonomous});
    syncParticipantsAutonomousB.push_back({"SyncAutonomousB5", TimeMode::Sync, OperationMode::Autonomous});
    syncParticipantsAutonomousB.push_back({"SyncAutonomousB6", TimeMode::Sync, OperationMode::Autonomous});
    syncParticipantsAutonomousB.push_back({"SyncAutonomousB7", TimeMode::Sync, OperationMode::Autonomous});
    syncParticipantsAutonomousB.push_back({"SyncAutonomousB8", TimeMode::Sync, OperationMode::Autonomous});
    syncParticipantsAutonomousB.push_back({"SyncAutonomousB9", TimeMode::Sync, OperationMode::Autonomous});


    // Additional async, non-required participants for testing the mixture
    std::list<TestParticipant> asyncParticipants;
    asyncParticipants.push_back({"ASync1", TimeMode::Async, OperationMode::Invalid});

    // Run coordinated
    expectedReceptions = syncParticipantsAutonomousA.size() + asyncParticipants.size();
    RunParticipants(asyncParticipants);
    RunParticipants(syncParticipantsAutonomousA);

    for (int i = 0; i < 3; i++)
    {
        if (verbose)
        {
            std::cout << ">> Cycle " << i + 1 << "/3" << std::endl;
        }

        if (verbose)
        {
            std::cout << ">> Await successful communication of coordinated participants" << std::endl;
        }

        // Await successful communication of avaliable participants (autonomous A, async)
        for (auto& p : syncParticipantsAutonomousA)
            p.AwaitCommunication();
        for (auto& p : asyncParticipants)
            p.AwaitCommunication();

        // Reset communication
        expectedReceptions =
            syncParticipantsAutonomousA.size() + syncParticipantsAutonomousB.size() + asyncParticipants.size();
        for (auto& p : syncParticipantsAutonomousA)
            p.ResetReception();
        for (auto& p : asyncParticipants)
            p.ResetReception();

        if (verbose)
        {
            std::cout << ">> Hop on with autonomous participants" << std::endl;
        }

        // Hop on with autonomous B
        RunParticipants(syncParticipantsAutonomousB, "B");

        if (verbose)
        {
            std::cout << ">> Await successful communication of all participants" << std::endl;
        }
        for (auto& p : syncParticipantsAutonomousA)
            p.AwaitCommunication();
        for (auto& p : syncParticipantsAutonomousB)
            p.AwaitCommunication();
        for (auto& p : asyncParticipants)
            p.AwaitCommunication();

        if (verbose)
        {
            std::cout << ">> Hop off with autonomous B participants" << std::endl;
        }

        // Shutdown autonomous B participants
        for (auto& p : syncParticipantsAutonomousB)
            p.lifecycleService->Stop("Hop Off");
        JoinParticipantThreads(_participantThreads_Sync_AutonomousB);

        // Reset communication to repeat the cycle
        expectedReceptions = syncParticipantsAutonomousA.size() + asyncParticipants.size();
        for (auto& p : syncParticipantsAutonomousA)
            p.ResetReception();
        for (auto& p : syncParticipantsAutonomousB)
            p.ResetReception();
        for (auto& p : asyncParticipants)
            p.ResetReception();
    }

    // Shutdown async participants
    for (auto& p : asyncParticipants)
        p.i._runAsync = false;
    JoinParticipantThreads(_participantThreads_Async_Invalid);

    // Shutdown autonomous A participants
    for (auto& p : syncParticipantsAutonomousA)
        p.lifecycleService->Stop("End Test Off");
    JoinParticipantThreads(_participantThreads_Sync_AutonomousA);

    StopRegistry();
}

} // anonymous namespace
