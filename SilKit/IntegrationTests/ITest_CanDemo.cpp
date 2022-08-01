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

#include <unordered_map> //remove this after rebase on cmake-cleanup-branch

#include "ITestFixture.hpp"

#include "silkit/services/can/all.hpp"

#include "gtest/gtest.h"

namespace {
using namespace SilKit::Tests;
using namespace SilKit::Config;
using namespace SilKit::Services;
using namespace SilKit::Services::Can;

TEST_F(ITest_SimTestHarness, can_demo)
{
    //Create a simulation setup with 2 participants and the netsim
    SetupFromParticipantList({"CanReader", "CanWriter", "CanMonitor"});

    //Test Results
    bool writerHasReceivedTx = false;
    bool writerHasReceivedRx = false;
    bool writerHasValidUserContext = false;
    bool receivedTransmitQueueFull = false;
    bool receivedTransmitted = false;
    bool receivedErrorActive = false;
    bool readerReceivedErrorActive = false;
    bool monitorReceivedErrorActive = false;
    size_t monitorReceiveCount = 0;

    //Test data
    const std::string payload = "Hallo Welt";

    std::vector<uint8_t> payloadBytes;
    payloadBytes.resize(payload.size());
    std::copy(payload.begin(), payload.end(), payloadBytes.begin());

    CanFrame msg;
    msg.canId = 123;
    msg.dataField = payloadBytes;

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

      canController->AddFrameTransmitHandler([&, participant](auto, const Can::CanFrameTransmitEvent& frameTransmitEvent) {
        if (frameTransmitEvent.status == Can::CanTransmitStatus::Transmitted)
        {
          receivedTransmitted = true;
          if(reinterpret_cast<void*>(participant) == frameTransmitEvent.userContext)
          {
            writerHasValidUserContext = true;
          }
        }
        if (frameTransmitEvent.status == Can::CanTransmitStatus::TransmitQueueFull)
        {
          receivedTransmitQueueFull = true;
        }
      });

      canController->AddErrorStateChangeHandler([&](auto, const Can::CanErrorStateChangeEvent& errorStateChangeEvent) {
        if (errorStateChangeEvent.errorState == Can::CanErrorState::ErrorActive)
        {
          receivedErrorActive = true;
        }
      });

      lifecycleService->SetCommunicationReadyHandler([canController, participantName]() {
        Log() << "---   " << participantName << ": Init called, setting baud rate and starting";
        canController->SetBaudRate(10'000, 1'000'000, 2'000'000);
        canController->Start();
      });

      timeSyncService->SetSimulationStepHandler(
      [participant, &msg, canController] (auto now) {
        //Cause transmit queue overrun
        if (now == 0ms)
        {
          for (auto i = 0; i < 21; i++) //keep in sync with VCanController::cNumberOfTxObjects 
          {
            canController->SendFrame(msg);
          }
          return;
        }

        //Cause a collision
        if (now == 10ms)
        {
          canController->SendFrame(msg);
        }

        //Normal transmission
        if (now > 20ms)
        {
          auto* canController1 = participant->CreateCanController("CanController1", "CAN1");
          Log() << "---   CanWriter sending CanFrame";
          canController1->SendFrame(msg, reinterpret_cast<void*>(participant));
          std::this_thread::sleep_for(10ms);//don't starve other threads on the CI build server
        }
      }, 1ms);

      canController->AddFrameHandler([&writerHasReceivedTx, &writerHasReceivedRx](auto, const Can::CanFrameEvent& frameEvent) {
        //ignore early test messages
        if(frameEvent.direction ==  SilKit::Services::TransmitDirection::TX)
        {
          writerHasReceivedTx = true;
        }

        if(frameEvent.direction ==  SilKit::Services::TransmitDirection::RX)
        {
          writerHasReceivedRx = true;
        }
      }, ((DirectionMask)TransmitDirection::RX | (DirectionMask)TransmitDirection::TX) );
    }
    //Reader variables
    bool result = false;
    auto receiveTime = 0ms;
    auto messageCount{0};
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

      timeSyncService->SetSimulationStepHandler([canController, &msg, &receiveTime](auto now) {
        receiveTime = std::chrono::duration_cast<std::chrono::milliseconds>(now);
        //Cause a collision
        if (now == 10ms)
        {
          canController->SendFrame(msg);
        }

        }, 1ms);

      canController->AddFrameHandler(
        [&, lifecycleService](auto, const Can::CanFrameEvent& frameEvent)
        {
          if (frameEvent.userContext == nullptr)
          {
            //Ignore the early test messages
            return;
          }
          EXPECT_EQ(frameEvent.direction, SilKit::Services::TransmitDirection::RX);

          EXPECT_EQ(frameEvent.frame.canId, 123u);
          EXPECT_EQ(frameEvent.userContext, _simTestHarness->GetParticipant("CanWriter")->Participant())
            << "frameEvent.frame.userContext is mangled!";
          if (messageCount++ == 10)
          {
            lifecycleService->Stop("Test done");
            Log() << "---    CanReader: Sending Stop from";
          }
          result = true;
      });

      canController->AddErrorStateChangeHandler([&](auto, const Can::CanErrorStateChangeEvent& errorStateChangeEvent) {
        if (errorStateChangeEvent.errorState == Can::CanErrorState::ErrorActive)
        {
          readerReceivedErrorActive = true;
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

      canController->AddErrorStateChangeHandler([&](auto, const Can::CanErrorStateChangeEvent& errorStateChangeEvent) {
        if (errorStateChangeEvent.errorState == Can::CanErrorState::ErrorActive)
        {
          monitorReceivedErrorActive = true;
        }
      });

      canController->AddFrameHandler([&](auto, const Can::CanFrameEvent& ) {
        monitorReceiveCount++;
      });
    }
    auto ok =  _simTestHarness->Run(5s);
    ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";
    EXPECT_TRUE(result) << " Expecting a message";
    EXPECT_TRUE(writerHasReceivedTx) << " Expecting a receive Message with Direction == TX on CanWriter";
    EXPECT_TRUE(writerHasValidUserContext) << " Expecting a CanFrameTransmitEvent with a valid"
      << " userContext during normal transmission";
    EXPECT_FALSE(receivedTransmitQueueFull) << " Sending too fast should NOT result in TransmitQueue full in trivial simulation";
    EXPECT_TRUE(receivedTransmitted) << " Sending should result in acknowledgment";
    EXPECT_FALSE(receivedErrorActive) << " Collisions are not computed in trivial simulation";
    EXPECT_FALSE(readerReceivedErrorActive) << " Collisions are not computed in trivial simulation";
    EXPECT_FALSE(monitorReceivedErrorActive) << " Collisions are not computed in trivial simulation"; 
    EXPECT_GT(monitorReceiveCount, 10) << "All participants connected to bus must receive frames";
}

} //end namespace
