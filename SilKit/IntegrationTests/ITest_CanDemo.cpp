// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <memory>
#include <thread>
#include <string>
#include <chrono>
#include <iostream>
#include <atomic>

#include <unordered_map> //remove this after rebase on cmake-cleanup-branch

#include "ITestFixture.hpp"
#include "ITestThreadSafeLogger.hpp"

#include "silkit/services/can/all.hpp"

#include "gtest/gtest.h"

namespace {
using namespace SilKit::Tests;
using namespace SilKit::Config;
using namespace SilKit::Services;
using namespace SilKit::Services::Can;

struct ITest_CanDemo : ITest_SimTestHarness
{
    using ITest_SimTestHarness::ITest_SimTestHarness;
};

struct TestState
{
    bool writerHasReceivedTx = false;
    bool writerHasReceivedRx = false;
    bool writerHasValidUserContext = false;
    bool receivedTransmitQueueFull = false;
    bool receivedTransmitted = false;
    bool receivedErrorActive = false;
    bool readerReceivedErrorActive = false;
    bool monitorReceivedErrorActive = false;
    std::atomic<uint32_t> monitorReceiveCount{0};
    std::atomic<uint32_t> monitorReceiveCountXlf{0};
    std::atomic<uint32_t> monitorReceiveCountFdf{0};


    struct OwningCanFrame : CanFrame
    {
        OwningCanFrame()
        {
            this->af = {};
            this->canId = {};
            this->dataField = {};
            this->dlc = {};
            this->flags = {};
            this->sdt = {};
            this->vcid = {};
        }
        std::vector<uint8_t> payloadBytes;
    };

    OwningCanFrame msg{};
    OwningCanFrame fdFrame{};
    OwningCanFrame xlFrame{};

    static constexpr uint32_t canId = 123;
    static constexpr uint32_t fdId = 0xcafecafe;
    static constexpr uint32_t xlId = 0xcacacaca;

    //Reader variables
    bool result = false;
    std::chrono::milliseconds receiveTime = 0ms;
    size_t messageCount{0};

    TestState()
    {
        auto fillData = [](auto& frame, size_t size) {
            frame.payloadBytes.resize(size);
            for (size_t i = 0; i < frame.payloadBytes.size(); i++)
            {
                frame.payloadBytes[i] = 'A' + (i % 26);
            }
            frame.dataField = frame.payloadBytes;
        };
        //Test data, CAN
        msg.canId = canId;
        fillData(msg, 8);
        msg.dlc = 8;

        // Test data, CAN FD
        fillData(fdFrame, 64);
        fdFrame.canId = fdId;
        fdFrame.dlc = 15;
        fdFrame.flags |= SilKit_CanFrameFlag_fdf | SilKit_CanFrameFlag_ide;

        // Test data, CAN XL
        fillData(xlFrame, 2048);
        xlFrame.canId = xlId;
        xlFrame.dlc = 2048;
        xlFrame.flags |= SilKit_CanFrameFlag_fdf | SilKit_CanFrameFlag_ide | SilKit_CanFrameFlag_xlf;
        xlFrame.af = 0xdeadbeef;
        xlFrame.vcid = 234;
        xlFrame.sdt = 1;
    }
};

TEST_F(ITest_CanDemo, can_demo)
{
    //Create a simulation setup with 2 participants and the netsim
    SetupFromParticipantList({"CanReader", "CanWriter", "CanMonitor"});

    //Test Results
    auto state = std::make_shared<TestState>();


    //Set up the Sending and receiving participants
    {
        /////////////////////////////////////////////////////////////////////////
        // CanWriter
        /////////////////////////////////////////////////////////////////////////
        const auto participantName = "CanWriter";
        auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
        auto&& participant = simParticipant->Participant();
        auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
        auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();
        auto&& canController = participant->CreateCanController("CanController1", "CAN1");

        canController->AddFrameTransmitHandler([state](auto, const Can::CanFrameTransmitEvent& frameTransmitEvent) {
            if (frameTransmitEvent.status == Can::CanTransmitStatus::Transmitted)
            {
                state->receivedTransmitted = true;
                if ((void*)(intptr_t)0xDEADBEEF == frameTransmitEvent.userContext)
                {
                    state->writerHasValidUserContext = true;
                }
            }
            if (frameTransmitEvent.status == Can::CanTransmitStatus::TransmitQueueFull)
            {
                state->receivedTransmitQueueFull = true;
            }
        });

        canController->AddErrorStateChangeHandler(
            [state](auto, const Can::CanErrorStateChangeEvent& errorStateChangeEvent) {
            if (errorStateChangeEvent.errorState == Can::CanErrorState::ErrorActive)
            {
                state->receivedErrorActive = true;
            }
        });

        lifecycleService->SetCommunicationReadyHandler([canController, participantName]() {
            Log() << "---   " << participantName << ": Init called, setting baud rate and starting";
            canController->SetBaudRate(10'000, 1'000'000, 2'000'000);
            canController->Start();
        });

        timeSyncService->SetSimulationStepHandler(
            [canController, state](auto now, std::chrono::nanoseconds /*duration*/) {
            //Cause transmit queue overrun
            if (now == 0ms)
            {
                for (auto i = 0; i < 21; i++) //keep in sync with VCanController::cNumberOfTxObjects
                {
                    canController->SendFrame(state->msg);
                }
                return;
            }

            //Cause a collision
            if (now == 10ms)
            {
                canController->SendFrame(state->msg);
            }

            //Normal transmission
            if (now > 20ms && now <= 30ms)
            {
                Log() << "---   CanWriter sending CanFrame";
                canController->SendFrame(state->msg, (void*)(intptr_t)(0xDEADBEEF));
            }
            // CAN FD
            if (now > 30ms && now <= 40ms)
            {
                Log() << "---   CanWriter sending FD Frame";
                canController->SendFrame(state->fdFrame, (void*)(intptr_t)(0xFDFDFDFD));
            }
            // CAN XL
            if (now > 40ms)
            {
                Log() << "---   CanWriter sending XL Frame";
                canController->SendFrame(state->xlFrame, (void*)(intptr_t)(0xFDFDFDFD));
            }
            },
            1ms);

        canController->AddFrameHandler(
            [state](auto, const Can::CanFrameEvent& frameEvent) {
            //ignore early test messages
            if (frameEvent.direction == SilKit::Services::TransmitDirection::TX)
            {
                state->writerHasReceivedTx = true;
            }

            if (frameEvent.direction == SilKit::Services::TransmitDirection::RX)
            {
                state->writerHasReceivedRx = true;
            }
            },
            ((DirectionMask)TransmitDirection::RX | (DirectionMask)TransmitDirection::TX));
    }

    {
        /////////////////////////////////////////////////////////////////////////
        // CanReader
        /////////////////////////////////////////////////////////////////////////
        const auto participantName = "CanReader";
        auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
        auto&& participant = simParticipant->Participant();
        auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
        auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();
        auto&& canController = participant->CreateCanController("CanController1", "CAN1");

        lifecycleService->SetCommunicationReadyHandler([canController, participantName]() {
            Log() << participantName << ": Init called, setting baud rate and starting";
            canController->SetBaudRate(10'000, 1'000'000, 2'000'000);
            canController->Start();
        });

        timeSyncService->SetSimulationStepHandler(
            [canController, state](auto now, std::chrono::nanoseconds /*duration*/) {
            state->receiveTime = std::chrono::duration_cast<std::chrono::milliseconds>(now);
            //Cause a collision
            if (now == 10ms)
            {
                canController->SendFrame(state->msg);
            }
            },
            1ms);

        canController->AddFrameHandler([state, lifecycleService](auto, const Can::CanFrameEvent& frameEvent) {
            if (frameEvent.timestamp < 20ms)
            {
                //Ignore the early test messages
                return;
            }
            EXPECT_EQ(frameEvent.direction, SilKit::Services::TransmitDirection::RX);
            EXPECT_TRUE(frameEvent.userContext == (void*)((size_t)(0)));

            if (frameEvent.frame.canId == TestState::xlId)
            {
                if (state->messageCount++ == 10)
                {
                    lifecycleService->Stop("Test done");
                    Log() << "---    CanReader: Sending Stop from";
                }
            }
            state->result = true;
        });

        canController->AddErrorStateChangeHandler(
            [state](auto, const Can::CanErrorStateChangeEvent& errorStateChangeEvent) {
            if (errorStateChangeEvent.errorState == Can::CanErrorState::ErrorActive)
            {
                state->readerReceivedErrorActive = true;
            }
        });
    }

    {
        /////////////////////////////////////////////////////////////////////////
        // CanMonitor --  Ensure that the Can simulation behaves like a bus
        /////////////////////////////////////////////////////////////////////////
        const auto participantName = "CanMonitor";
        auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
        auto&& participant = simParticipant->Participant();
        auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
        auto&& canController = participant->CreateCanController("CanController1", "CAN1");

        lifecycleService->SetCommunicationReadyHandler([canController, participantName]() {
            Log() << participantName << ": Init called, setting baud rate and starting";
            canController->SetBaudRate(10'000, 1'000'000, 2'000'000);
            canController->Start();
        });

        canController->AddErrorStateChangeHandler(
            [state](auto, const Can::CanErrorStateChangeEvent& errorStateChangeEvent) {
            if (errorStateChangeEvent.errorState == Can::CanErrorState::ErrorActive)
            {
                state->monitorReceivedErrorActive = true;
            }
        });

        canController->AddFrameHandler([state](auto, const Can::CanFrameEvent& frame) {
            if (frame.frame.canId == TestState::canId)
            {
                state->monitorReceiveCount++;
            }
            else if (frame.frame.canId == TestState::fdId)
            {
                state->monitorReceiveCountFdf++;
                EXPECT_FALSE(frame.frame.flags & SilKit_CanFrameFlag_xlf);
                EXPECT_TRUE(frame.frame.flags & SilKit_CanFrameFlag_fdf);
            }
            else if (frame.frame.canId == TestState::xlId)
            {
                state->monitorReceiveCountXlf++;
                EXPECT_EQ(frame.frame.sdt, 1);
                EXPECT_EQ(frame.frame.af, 0xdeadbeef);
                EXPECT_EQ(frame.frame.vcid, 234);
                EXPECT_EQ(frame.frame.dataField.size(), 2048u);
                EXPECT_TRUE(frame.frame.flags & SilKit_CanFrameFlag_xlf);
                EXPECT_TRUE(frame.frame.flags & SilKit_CanFrameFlag_fdf);
            }
        });
    }

    auto ok = _simTestHarness->Run(500s);
    ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";
    EXPECT_TRUE(state->result) << " Expecting a message";
    EXPECT_TRUE(state->writerHasReceivedTx) << " Expecting a receive Message with Direction == TX on CanWriter";
    EXPECT_TRUE(state->writerHasValidUserContext) << " Expecting a CanFrameTransmitEvent with a valid"
                                                  << " userContext during normal transmission";
    EXPECT_FALSE(state->receivedTransmitQueueFull)
        << " Sending too fast should NOT result in TransmitQueue full in trivial simulation";
    EXPECT_TRUE(state->receivedTransmitted) << " Sending should result in acknowledgment";
    EXPECT_FALSE(state->receivedErrorActive) << " Collisions are not computed in trivial simulation";
    EXPECT_FALSE(state->readerReceivedErrorActive) << " Collisions are not computed in trivial simulation";
    EXPECT_FALSE(state->monitorReceivedErrorActive) << " Collisions are not computed in trivial simulation";
    EXPECT_GT(state->monitorReceiveCount, 10u) << "All participants connected to bus must receive frames";
    EXPECT_GE(state->monitorReceiveCountFdf, 10u) << "All participants connected to bus must receive FD frames";
    EXPECT_GE(state->monitorReceiveCountXlf, 10u) << "All participants connected to bus must receive XL frames";
}

} //end namespace
