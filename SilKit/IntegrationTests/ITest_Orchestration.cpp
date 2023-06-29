#include <memory>
#include <thread>
#include <string>
#include <chrono>
#include <iostream>

#include "ITestFixture.hpp"
#include "silkit/services/can/all.hpp"

#include "gtest/gtest.h"

namespace {

using namespace std::chrono_literals;
inline std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
    out << seconds.count() << "s";
    return out;
}

using namespace SilKit::Tests;
using namespace SilKit::Config;
using namespace SilKit::Services;
using namespace SilKit::Services::Can;

struct StopTestState
{
  // Test Results
  std::atomic<std::chrono::nanoseconds> lastNow{};
};

TEST_F(ITest_SimTestHarness, stop_in_simtask_does_not_trigger_simtask_again)
{
  std::vector<std::string> canParticipantNames;
  for (char participantSuffix = 'A'; participantSuffix <= 'K'; ++participantSuffix)
  {
    canParticipantNames.emplace_back(std::string{"CanParticipant"} + participantSuffix);
  }

  {
    auto participantNames = canParticipantNames;
    participantNames.emplace_back("SimulationStopper");
    SetupFromParticipantList(participantNames);
  }

  auto state = std::make_shared<StopTestState>();

  // Set up the Sending and receiving participants
  {
    /////////////////////////////////////////////////////////////////////////
    // SimulationStopper
    /////////////////////////////////////////////////////////////////////////
    const auto participantName = "SimulationStopper";
    auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
    auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
    auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();

    timeSyncService->SetSimulationStepHandler(
      [state, lifecycleService](auto now, const std::chrono::nanoseconds /*duration*/) {
        state->lastNow = now;

        if (now == 5ms)
        {
          lifecycleService->Stop("stopping the simulation through another participant");
        }
      },
      1ms);
  }

  for (const auto& participantName : canParticipantNames)
  {
    /////////////////////////////////////////////////////////////////////////
    // CAN Participants
    /////////////////////////////////////////////////////////////////////////
    auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
    auto&& participant = simParticipant->Participant();
    auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
    auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();
    auto&& canController = participant->CreateCanController("CAN1", "CAN_1");

    lifecycleService->SetCommunicationReadyHandler([canController] {
      canController->SetBaudRate(10'000, 1'000'000, 2'000'000);
      canController->Start();
    });

    timeSyncService->SetSimulationStepHandler(
      [state, canController, dataByte = static_cast<uint8_t>(participantName.back())](
        auto /*now*/, const std::chrono::nanoseconds /*duration*/) {
        std::array<uint8_t, 1> dataBytes{dataByte};

        CanFrame frame{};
        frame.flags = {};
        frame.canId = 0x12;
        frame.dlc = 1;
        frame.dataField = SilKit::Util::MakeSpan(dataBytes);

        for (int i = 0; i < 10; ++i)
        {
          canController->SendFrame(frame);
        }
      },
      1ms);
  }

  auto ok = _simTestHarness->Run(5s);
  ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";

  auto lastNow = state->lastNow.load();
  std::cout << "lastNow = " << lastNow << std::endl; 
  EXPECT_TRUE(state->lastNow.load() == 5ms);
}

} //end namespace
