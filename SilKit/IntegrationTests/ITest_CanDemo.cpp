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

    std::vector<uint8_t> payloadBytes;

    CanFrame msg;

    //Reader variables
    bool result = false;
    std::chrono::milliseconds receiveTime = 0ms;
    size_t messageCount{0};
};

TEST_F(ITest_CanDemo, can_demo)
{
    //Create a simulation setup with 2 participants and the netsim
    SetupFromParticipantList({"CanReader", "CanWriter", "CanMonitor"});

    //Test Results
    auto state = std::make_shared<TestState>();

    //Test data
    const std::string payload = "Hallo Welt";

    state->payloadBytes.resize(payload.size());
    std::copy(payload.begin(), payload.end(), state->payloadBytes.begin());

    state->msg = CanFrame{};
    state->msg.canId = 123;
    state->msg.dataField = state->payloadBytes;

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
          if((void*)(intptr_t)0xDEADBEEF == frameTransmitEvent.userContext)
          {
            state->writerHasValidUserContext = true;
          }
        }
        if (frameTransmitEvent.status == Can::CanTransmitStatus::TransmitQueueFull)
        {
          state->receivedTransmitQueueFull = true;
        }
      });

      canController->AddErrorStateChangeHandler([state](auto, const Can::CanErrorStateChangeEvent& errorStateChangeEvent) {
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
      [canController, state] (auto now, std::chrono::nanoseconds /*duration*/) {
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
        if (now > 20ms)
        {
          Log() << "---   CanWriter sending CanFrame";
          canController->SendFrame(state->msg, (void*)(intptr_t)(0xDEADBEEF));
        }
      }, 1ms);

      canController->AddFrameHandler([state](auto, const Can::CanFrameEvent& frameEvent) {
        //ignore early test messages
        if(frameEvent.direction ==  SilKit::Services::TransmitDirection::TX)
        {
          state->writerHasReceivedTx = true;
        }

        if(frameEvent.direction ==  SilKit::Services::TransmitDirection::RX)
        {
          state->writerHasReceivedRx = true;
        }
      }, ((DirectionMask)TransmitDirection::RX | (DirectionMask)TransmitDirection::TX) );
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

      timeSyncService->SetSimulationStepHandler([canController, state](auto now, std::chrono::nanoseconds /*duration*/) {
        state->receiveTime = std::chrono::duration_cast<std::chrono::milliseconds>(now);
        //Cause a collision
        if (now == 10ms)
        {
          canController->SendFrame(state->msg);
        }

        }, 1ms);

      canController->AddFrameHandler(
        [state, lifecycleService](auto, const Can::CanFrameEvent& frameEvent)
        {
          if (frameEvent.timestamp < 20ms)
          {
              //Ignore the early test messages
              return;
          }
          EXPECT_EQ(frameEvent.direction, SilKit::Services::TransmitDirection::RX);

          EXPECT_EQ(frameEvent.frame.canId, 123u);

          EXPECT_TRUE(frameEvent.userContext == (void*)((size_t)(0)));

          if (state->messageCount++ == 10)
          {
            lifecycleService->Stop("Test done");
            Log() << "---    CanReader: Sending Stop from";
          }
          state->result = true;
      });

      canController->AddErrorStateChangeHandler([state](auto, const Can::CanErrorStateChangeEvent& errorStateChangeEvent) {
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

      canController->AddErrorStateChangeHandler([state](auto, const Can::CanErrorStateChangeEvent& errorStateChangeEvent) {
        if (errorStateChangeEvent.errorState == Can::CanErrorState::ErrorActive)
        {
          state->monitorReceivedErrorActive = true;
        }
      });

      canController->AddFrameHandler([state](auto, const Can::CanFrameEvent& ) {
        state->monitorReceiveCount++;
      });
    }

    auto ok =  _simTestHarness->Run(5s);
    ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";
    EXPECT_TRUE(state->result) << " Expecting a message";
    EXPECT_TRUE(state->writerHasReceivedTx) << " Expecting a receive Message with Direction == TX on CanWriter";
    EXPECT_TRUE(state->writerHasValidUserContext) << " Expecting a CanFrameTransmitEvent with a valid"
      << " userContext during normal transmission";
    EXPECT_FALSE(state->receivedTransmitQueueFull) << " Sending too fast should NOT result in TransmitQueue full in trivial simulation";
    EXPECT_TRUE(state->receivedTransmitted) << " Sending should result in acknowledgment";
    EXPECT_FALSE(state->receivedErrorActive) << " Collisions are not computed in trivial simulation";
    EXPECT_FALSE(state->readerReceivedErrorActive) << " Collisions are not computed in trivial simulation";
    EXPECT_FALSE(state->monitorReceivedErrorActive) << " Collisions are not computed in trivial simulation";
    EXPECT_GT(state->monitorReceiveCount, 10u) << "All participants connected to bus must receive frames";
}

} //end namespace
