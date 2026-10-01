// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

// LIN master and slave go through several go-to-sleep / wakeup cycles and exchange frames after every wakeup, so a
// slave that does not return from sleep is detected. ITest_Lin only covers a single cycle.

#include <chrono>
#include <string>

#include "silkit/services/lin/all.hpp"
#include "silkit/services/orchestration/all.hpp"

#include "SimTestHarness.hpp"

#include "gtest/gtest.h"

namespace {

using namespace std::chrono_literals;
using namespace SilKit::Services::Lin;

constexpr LinId dataFrameId = 16;
constexpr size_t numberOfCycles = 3;

auto MakeDataFrame(uint8_t cycle) -> LinFrame
{
    LinFrame frame;
    frame.id = dataFrameId;
    frame.checksumModel = LinChecksumModel::Classic;
    frame.dataLength = 8;
    frame.data = {cycle, cycle, cycle, cycle, cycle, cycle, cycle, cycle};
    return frame;
}

TEST(ITest_LinSleepWakeup, repeated_sleep_and_wakeup_cycles)
{
    SilKit::Tests::SimTestHarnessArgs args;
    args.syncParticipantNames = {"LinMaster", "LinSlave"};
    SilKit::Tests::SimTestHarness testHarness{args};

    // --- Master: send a data frame, then the go-to-sleep command, wait for the wakeup of the slave, repeat.
    enum class MasterPhase
    {
        SendData,
        WaitForDataSent,
        SendGoToSleep,
        WaitForWakeup,
        Done
    };

    auto* master = testHarness.GetParticipant("LinMaster");
    auto* masterController = master->Participant()->CreateLinController("LIN1", "LIN_1");
    MasterPhase masterPhase = MasterPhase::SendData;
    size_t masterWakeups = 0;
    size_t masterDataFramesSent = 0;

    master->GetOrCreateLifecycleService()->SetCommunicationReadyHandler([masterController] {
        LinControllerConfig config;
        config.controllerMode = LinControllerMode::Master;
        config.baudRate = 20'000;
        masterController->Init(config);
    });
    masterController->AddFrameStatusHandler([&](ILinController*, const LinFrameStatusEvent& event) {
        if (event.frame.id == dataFrameId && event.status == LinFrameStatus::LIN_TX_OK
            && masterPhase == MasterPhase::WaitForDataSent)
        {
            masterDataFramesSent++;
            masterPhase = MasterPhase::SendGoToSleep;
        }
    });
    masterController->AddWakeupHandler([&](ILinController* controller, const LinWakeupEvent&) {
        if (masterPhase != MasterPhase::WaitForWakeup)
        {
            return;
        }
        controller->WakeupInternal();
        masterWakeups++;
        masterPhase = masterWakeups < numberOfCycles ? MasterPhase::SendData : MasterPhase::Done;
    });
    master->GetOrCreateTimeSyncService()->SetSimulationStepHandler(
        [&](std::chrono::nanoseconds now, std::chrono::nanoseconds) {
        switch (masterPhase)
        {
        case MasterPhase::SendData:
            ASSERT_EQ(masterController->Status(), LinControllerStatus::Operational);
            masterPhase = MasterPhase::WaitForDataSent;
            masterController->SendFrame(MakeDataFrame(static_cast<uint8_t>(masterWakeups)),
                                        LinFrameResponseType::MasterResponse);
            break;
        case MasterPhase::SendGoToSleep:
            masterPhase = MasterPhase::WaitForWakeup;
            masterController->GoToSleep();
            EXPECT_EQ(masterController->Status(), LinControllerStatus::Sleep);
            break;
        case MasterPhase::Done:
            master->Stop();
            break;
        default:
            break;
        }
        if (now > 1s)
        {
            ADD_FAILURE() << "LIN sleep/wakeup cycles did not finish in time";
            master->Stop();
        }
    }, 1ms);

    // --- Slave: receive the data frame, enter sleep on the go-to-sleep command and wake the bus 5ms later.
    auto* slave = testHarness.GetParticipant("LinSlave");
    auto* slaveController = slave->Participant()->CreateLinController("LIN1", "LIN_1");
    std::vector<LinFrame> slaveDataFrames;
    size_t slaveDataFramesInSleep = 0;
    size_t slaveSleeps = 0;
    std::chrono::nanoseconds slaveNow{0};
    std::chrono::nanoseconds slaveWakeupTime = std::chrono::nanoseconds::max();

    slave->GetOrCreateLifecycleService()->SetCommunicationReadyHandler([slaveController] {
        LinControllerConfig config;
        config.controllerMode = LinControllerMode::Slave;
        config.baudRate = 20'000;
        LinFrameResponse dataResponse;
        dataResponse.frame = MakeDataFrame(0);
        dataResponse.responseMode = LinFrameResponseMode::Rx;
        LinFrameResponse goToSleepResponse;
        goToSleepResponse.frame = GoToSleepFrame();
        goToSleepResponse.responseMode = LinFrameResponseMode::Rx;
        config.frameResponses = {dataResponse, goToSleepResponse};
        slaveController->Init(config);
    });
    slaveController->AddFrameStatusHandler([&](ILinController* controller, const LinFrameStatusEvent& event) {
        if (event.frame.id != dataFrameId || event.status != LinFrameStatus::LIN_RX_OK)
        {
            return;
        }
        slaveDataFrames.push_back(event.frame);
        if (controller->Status() == LinControllerStatus::Sleep)
        {
            slaveDataFramesInSleep++;
        }
    });
    slaveController->AddGoToSleepHandler([&](ILinController* controller, const LinGoToSleepEvent&) {
        controller->GoToSleepInternal();
        slaveSleeps++;
        slaveWakeupTime = slaveNow + 5ms;
    });
    slave->GetOrCreateTimeSyncService()->SetSimulationStepHandler(
        [&](std::chrono::nanoseconds now, std::chrono::nanoseconds) {
        slaveNow = now;
        if (now >= slaveWakeupTime)
        {
            slaveWakeupTime = std::chrono::nanoseconds::max();
            EXPECT_EQ(slaveController->Status(), LinControllerStatus::Sleep);
            slaveController->Wakeup();
            EXPECT_EQ(slaveController->Status(), LinControllerStatus::Operational)
                << "The slave must be operational again after sending the wakeup pulse";
        }
    }, 1ms);

    ASSERT_TRUE(testHarness.Run(10s));

    EXPECT_EQ(masterDataFramesSent, numberOfCycles);
    EXPECT_EQ(masterWakeups, numberOfCycles);
    EXPECT_EQ(slaveSleeps, numberOfCycles);
    EXPECT_EQ(slaveDataFramesInSleep, 0u);
    ASSERT_EQ(slaveDataFrames.size(), numberOfCycles) << "The slave must receive a data frame after every wakeup";
    for (size_t cycle = 0; cycle < numberOfCycles; ++cycle)
    {
        EXPECT_EQ(slaveDataFrames[cycle].data[0], cycle);
    }
    EXPECT_EQ(masterController->Status(), LinControllerStatus::Operational);
    EXPECT_EQ(slaveController->Status(), LinControllerStatus::Operational);
}

} // namespace
