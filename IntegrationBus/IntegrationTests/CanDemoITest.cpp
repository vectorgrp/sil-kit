#include <memory>
#include <thread>
#include <string>
#include <chrono>
#include <iostream>

#include <unordered_map> //remove this after rebase on cmake-cleanup-branch

#include "ITestFixture.hpp"

#include "ib/sim/can/all.hpp"

#include "gtest/gtest.h"

namespace {
using namespace ib::test;
using namespace ib::cfg;
using namespace ib::sim;
using namespace ib::sim::can;

TEST_F(SimTestHarnessITest, can_demo)
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
    CanFrame msg;
    msg.canId = 123;
    std::copy(payload.begin(), payload.end(), std::back_inserter(msg.dataField));

    //Set up the Sending and receiving participants
    {
      /////////////////////////////////////////////////////////////////////////
      // CanWriter
      /////////////////////////////////////////////////////////////////////////
      const auto participantName = "CanWriter";
      auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
      auto&& participant = simParticipant->Participant();
      auto&& lifecycleService = participant->GetLifecycleService();
      auto&& timeSyncService = lifecycleService->GetTimeSyncService();
      auto&& canController = participant->CreateCanController("CanController1", "CAN_1");

      canController->AddFrameTransmitHandler([&](auto, const can::CanFrameTransmitEvent& frameTransmitEvent) {
        if (frameTransmitEvent.status == can::CanTransmitStatus::Transmitted)
        {
          receivedTransmitted = true;
          if(reinterpret_cast<void*>(participant) == frameTransmitEvent.userContext)
          {
            writerHasValidUserContext = true;
          }
        }
        if (frameTransmitEvent.status == can::CanTransmitStatus::TransmitQueueFull)
        {
          receivedTransmitQueueFull = true;
        }
      });

      canController->AddErrorStateChangeHandler([&](auto, const can::CanErrorStateChangeEvent& errorStateChangeEvent) {
        if (errorStateChangeEvent.errorState == can::CanErrorState::ErrorActive)
        {
          receivedErrorActive = true;
        }
      });

      lifecycleService->SetCommunicationReadyHandler([canController, participantName]() {
        Log() << "---   " << participantName << ": Init called, setting baud rate and starting";
        canController->SetBaudRate(10'000, 1'000'000);
        canController->Start();
      });

      timeSyncService->SetSimulationTask(
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
          auto* canController1 = participant->CreateCanController("CanController1", "CAN_1");
          Log() << "---   CanWriter sending CanFrame";
          canController1->SendFrame(msg, reinterpret_cast<void*>(participant));
          std::this_thread::sleep_for(10ms);//don't starve other threads on the CI build server
        }
      });

      canController->AddFrameHandler([&writerHasReceivedTx, &writerHasReceivedRx](auto, const can::CanFrameEvent& frameEvent) {
        //ignore early test messages
        if(frameEvent.direction ==  ib::sim::TransmitDirection::TX)
        {
          writerHasReceivedTx = true;
        }

        if(frameEvent.direction ==  ib::sim::TransmitDirection::RX)
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
      auto&& lifecycleService = participant->GetLifecycleService();
      auto&& timeSyncService = lifecycleService->GetTimeSyncService();
      auto&& canController = participant->CreateCanController("CanController1", "CAN_1");

      lifecycleService->SetCommunicationReadyHandler([canController, participantName]() {
        Log() << participantName << ": Init called, setting baud rate and starting";
        canController->SetBaudRate(10'000, 1'000'000);
        canController->Start();
      });

      timeSyncService->SetSimulationTask([canController, &msg, &receiveTime](auto now) {
        receiveTime = std::chrono::duration_cast<std::chrono::milliseconds>(now);
        //Cause a collision
        if (now == 10ms)
        {
          canController->SendFrame(msg);
        }

        });

      canController->AddFrameHandler(
        [&](auto, const can::CanFrameEvent& frameEvent)
        {
          if (frameEvent.userContext == nullptr)
          {
            //Ignore the early test messages
            return;
          }
          EXPECT_EQ(frameEvent.direction, ib::sim::TransmitDirection::RX);

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

      canController->AddErrorStateChangeHandler([&](auto, const can::CanErrorStateChangeEvent& errorStateChangeEvent) {
        if (errorStateChangeEvent.errorState == can::CanErrorState::ErrorActive)
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
      auto&& lifecycleService = participant->GetLifecycleService();
      auto&& canController = participant->CreateCanController("CanController1", "CAN_1");

      lifecycleService->SetCommunicationReadyHandler([canController, participantName]() {
        Log() << participantName << ": Init called, setting baud rate and starting";
        canController->SetBaudRate(10'000, 1'000'000);
        canController->Start();
      });

      canController->AddErrorStateChangeHandler([&](auto, const can::CanErrorStateChangeEvent& errorStateChangeEvent) {
        if (errorStateChangeEvent.errorState == can::CanErrorState::ErrorActive)
        {
          monitorReceivedErrorActive = true;
        }
      });

      canController->AddFrameHandler([&](auto, const can::CanFrameEvent& ) {
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
