// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ITest_NetSim.hpp"
#include "silkit/services/flexray/all.hpp"

namespace {

using namespace std::chrono_literals;

using namespace SilKit::Tests;
using namespace SilKit::Config;
using namespace SilKit::Services;
using namespace SilKit::Experimental::NetworkSimulation;

using namespace SilKit::Services::Flexray;
using namespace SilKit::Experimental::NetworkSimulation::Flexray;

struct ITest_NetSimFlexray : ITest_NetSim
{

    void SetupFlexrayController(Orchestration::ILifecycleService* lifecycleService, IFlexrayController* controller,
                            CallCountsSilKitHandlersFlexray& callCountsSilKitHandlersFlexray)
    {
        controller->AddCycleStartHandler(
            [&callCountsSilKitHandlersFlexray](IFlexrayController*, const FlexrayCycleStartEvent& /*msg*/) {
                callCountsSilKitHandlersFlexray.CycleStartHandler++;
            });
        controller->AddFrameHandler(
            [&callCountsSilKitHandlersFlexray](IFlexrayController*, const FlexrayFrameEvent& /*msg*/) {
                callCountsSilKitHandlersFlexray.FrameHandler++;
            });
        controller->AddFrameTransmitHandler(
            [&callCountsSilKitHandlersFlexray](IFlexrayController*, const FlexrayFrameTransmitEvent& /*msg*/) {
                callCountsSilKitHandlersFlexray.FrameTransmitHandler++;
            });
        controller->AddPocStatusHandler(
            [&callCountsSilKitHandlersFlexray](IFlexrayController*, const FlexrayPocStatusEvent& /*msg*/) {
                callCountsSilKitHandlersFlexray.PocStatusHandler++;
            });
        controller->AddSymbolHandler(
            [&callCountsSilKitHandlersFlexray](IFlexrayController*, const FlexraySymbolEvent& /*msg*/) {
                callCountsSilKitHandlersFlexray.SymbolHandler++;
            });
        controller->AddSymbolTransmitHandler(
            [&callCountsSilKitHandlersFlexray](IFlexrayController*, const FlexraySymbolTransmitEvent& /*msg*/) {
                callCountsSilKitHandlersFlexray.SymbolTransmitHandler++;
            });
        controller->AddWakeupHandler(
            [&callCountsSilKitHandlersFlexray](IFlexrayController*, const FlexrayWakeupEvent& /*msg*/) {
                callCountsSilKitHandlersFlexray.WakeupHandler++;
            });

        lifecycleService->SetCommunicationReadyHandler([controller] {

            FlexrayClusterParameters clusterParams;
            clusterParams.gColdstartAttempts = 8;
            clusterParams.gCycleCountMax = 63;
            clusterParams.gdActionPointOffset = 2;
            clusterParams.gdDynamicSlotIdlePhase = 1;
            clusterParams.gdMiniSlot = 5;
            clusterParams.gdMiniSlotActionPointOffset = 2;
            clusterParams.gdStaticSlot = 31;
            clusterParams.gdSymbolWindow = 0;
            clusterParams.gdSymbolWindowActionPointOffset = 1;
            clusterParams.gdTSSTransmitter = 9;
            clusterParams.gdWakeupTxActive = 60;
            clusterParams.gdWakeupTxIdle = 180;
            clusterParams.gListenNoise = 2;
            clusterParams.gMacroPerCycle = 3636;
            clusterParams.gMaxWithoutClockCorrectionFatal = 2;
            clusterParams.gMaxWithoutClockCorrectionPassive = 2;
            clusterParams.gNumberOfMiniSlots = 291;
            clusterParams.gNumberOfStaticSlots = 70;
            clusterParams.gPayloadLengthStatic = 13;
            clusterParams.gSyncFrameIDCountMax = 15;

            FlexrayNodeParameters nodeParams;
            nodeParams.pAllowHaltDueToClock = 1;
            nodeParams.pAllowPassiveToActive = 0;
            nodeParams.pChannels = FlexrayChannel::AB;
            nodeParams.pClusterDriftDamping = 2;
            nodeParams.pdAcceptedStartupRange = 212;
            nodeParams.pdListenTimeout = 400162;
            nodeParams.pKeySlotOnlyEnabled = 0;
            nodeParams.pKeySlotUsedForStartup = 1;
            nodeParams.pKeySlotUsedForSync = 0;
            nodeParams.pLatestTx = 249;
            nodeParams.pMacroInitialOffsetA = 3;
            nodeParams.pMacroInitialOffsetB = 3;
            nodeParams.pMicroInitialOffsetA = 6;
            nodeParams.pMicroInitialOffsetB = 6;
            nodeParams.pMicroPerCycle = 200000;
            nodeParams.pOffsetCorrectionOut = 127;
            nodeParams.pOffsetCorrectionStart = 3632;
            nodeParams.pRateCorrectionOut = 81;
            nodeParams.pWakeupChannel = FlexrayChannel::A;
            nodeParams.pWakeupPattern = 33;
            nodeParams.pdMicrotick = FlexrayClockPeriod::T25NS;
            nodeParams.pSamplesPerMicrotick = 2;
            nodeParams.pKeySlotId = 40;

            std::vector<FlexrayTxBufferConfig> bufferConfigs;

            FlexrayTxBufferConfig bufferCfg;
            bufferCfg.channels = FlexrayChannel::AB;
            bufferCfg.slotId = 40;
            bufferCfg.offset = 0;
            bufferCfg.repetition = 1;
            bufferCfg.hasPayloadPreambleIndicator = false;
            bufferCfg.headerCrc = 5;
            bufferCfg.transmissionMode = FlexrayTransmissionMode::SingleShot;
            bufferConfigs.push_back(bufferCfg);
            
            bufferCfg.channels = FlexrayChannel::A;
            bufferCfg.slotId = 41;
            bufferConfigs.push_back(bufferCfg);

            bufferCfg.channels = FlexrayChannel::B;
            bufferCfg.slotId = 42;
            bufferConfigs.push_back(bufferCfg);

            SilKit::Services::Flexray::FlexrayControllerConfig config;
            config.bufferConfigs = bufferConfigs;
            config.clusterParams = clusterParams;
            config.nodeParams = nodeParams;

            controller->Configure(config);
        });
    }

    void OnetimeActions(std::chrono::nanoseconds now, IFlexrayController* controller) 
    {
        if (now == 0ns)
        {
            // Trigger OnHostCommand
            controller->Wakeup();

            // Trigger OnTxBufferConfigUpdate
            FlexrayTxBufferConfig bufferCfg{};
            bufferCfg.channels = FlexrayChannel::AB;
            controller->ReconfigureTxBuffer(0, bufferCfg); // idx 0 -> slotId 40

            // Trigger OnTxBufferUpdate
            SilKit::Services::Flexray::FlexrayTxBufferUpdate txBufferUpdate{};
            txBufferUpdate.txBufferIndex = 0;
            txBufferUpdate.payloadDataValid = true;
            std::vector<uint8_t> payloadBytes{1, 2, 3};
            txBufferUpdate.payload = payloadBytes;
            controller->UpdateTxBuffer(txBufferUpdate);
        }
    }

    void SendFlexrayFrames(MySimulatedNetwork* mySimulatedNetwork, std::chrono::nanoseconds now,
                           std::atomic_uint& sendCount)
    {
        // Called by the netsim, not by controllers

        if (now < _sendUntilMs)
        {
            for (size_t i = 0; i < _numFramesPerSimStep; ++i)
            {
                FlexrayFrameEvent frameEvent{};
                std::vector<uint8_t> payloadBytes{1, 2, 3};
                FlexrayFrame frame{};
                frame.payload = payloadBytes;
                frameEvent.frame = frame;
                mySimulatedNetwork->GetFlexRayEventProducer()->Produce(
                    frameEvent, mySimulatedNetwork->GetAllControllerDescriptors());

                FlexrayFrameTransmitEvent frameTransmitEvent{};
                frameTransmitEvent.txBufferIndex = 0;
                frameTransmitEvent.frame = frame;
                mySimulatedNetwork->GetFlexRayEventProducer()->Produce(
                    frameTransmitEvent, mySimulatedNetwork->GetAllControllerDescriptors());

                sendCount++;
            }
        }
    }

};

class MySimulatedFlexrayController
    : public MySimulatedController
    , public ISimulatedFlexRayController
{
public:
    MySimulatedFlexrayController(MySimulatedNetwork* mySimulatedNetwork, ControllerDescriptor controllerDescriptor)
        : MySimulatedController(mySimulatedNetwork, controllerDescriptor)
    {
    }

    void OnHostCommand(const FlexrayHostCommand& msg) override;
    void OnControllerConfig(const SilKit::Experimental::NetworkSimulation::Flexray::FlexrayControllerConfig& msg) override;
    void OnTxBufferConfigUpdate(const FlexrayTxBufferConfigUpdate& msg) override;
    void OnTxBufferUpdate(const SilKit::Experimental::NetworkSimulation::Flexray::FlexrayTxBufferUpdate& msg) override;

};


auto MySimulatedNetwork::ProvideSimulatedController(ControllerDescriptor controllerDescriptor) -> ISimulatedController*
{
    callCounts.simulatedNetwork.ProvideSimulatedController++;
    _controllerDescriptors.push_back(controllerDescriptor);

    switch (_networkType)
    {
    case SimulatedNetworkType::FlexRay:
    {
        _mySimulatedControllers.emplace_back(std::make_unique<MySimulatedFlexrayController>(this, controllerDescriptor));
        return _mySimulatedControllers.back().get();
    }
    default:
        break;
    }

    return {};
}

// ISimulatedFlexrayController

void MySimulatedFlexrayController::OnHostCommand(const FlexrayHostCommand& msg)
{
    callCounts.netSimFlexray.OnHostCommand++;

    if (msg.command == FlexrayChiCommand::WAKEUP)
    {
        FlexrayPocStatusEvent pocStatusEvent{};
        pocStatusEvent.wakeupStatus = FlexrayWakeupStatusType::Transmitted;
        _mySimulatedNetwork->GetFlexRayEventProducer()->Produce(pocStatusEvent,
                                                                _mySimulatedNetwork->GetAllControllerDescriptors());

        FlexrayCycleStartEvent cycleStartEvent{};
        cycleStartEvent.cycleCounter = 1;
        _mySimulatedNetwork->GetFlexRayEventProducer()->Produce(cycleStartEvent,
                                                                _mySimulatedNetwork->GetAllControllerDescriptors());

        FlexraySymbolEvent symbolEvent{};
        symbolEvent.pattern = FlexraySymbolPattern::Wus; // Triggers Wakeup handler + Symbol handler
        _mySimulatedNetwork->GetFlexRayEventProducer()->Produce(symbolEvent,
                                                                _mySimulatedNetwork->GetAllControllerDescriptors());

        FlexraySymbolTransmitEvent symbolTransmitEvent{};
        symbolTransmitEvent.pattern = FlexraySymbolPattern::Wus; // SymbolTransmit handler
        std::array<ControllerDescriptor, 1> receiverArray{_controllerDescriptor};
        auto receiver = SilKit::Util::MakeSpan(receiverArray);
        _mySimulatedNetwork->GetFlexRayEventProducer()->Produce(symbolTransmitEvent, receiver);
    }
    
}

void MySimulatedFlexrayController::OnControllerConfig(
    const SilKit::Experimental::NetworkSimulation::Flexray::FlexrayControllerConfig& /*msg*/)
{
    callCounts.netSimFlexray.OnControllerConfig++;
}

void MySimulatedFlexrayController::OnTxBufferConfigUpdate(const FlexrayTxBufferConfigUpdate& /*msg*/) 
{
    callCounts.netSimFlexray.OnTxBufferConfigUpdate++;
}

void MySimulatedFlexrayController::OnTxBufferUpdate(
    const SilKit::Experimental::NetworkSimulation::Flexray::FlexrayTxBufferUpdate& /*msg*/)
{
    callCounts.netSimFlexray.OnTxBufferUpdate++;
}

// Testing NetSim API with FlexRay
// Covers:
// - Trivial and simulated FlexrayControllers in one simulation
// - Correct routing of simulated FlexRay messages
// - Network Simulator participant has FlexrayControllers itself

TEST_F(ITest_NetSimFlexray, basic_networksimulation_flexray)
{
    {
        // ----------------------------
        // NetworkSimulator
        // ----------------------------

        //auto configWithLogging = MakeParticipantConfigurationStringWithLogging(SilKit::Services::Logging::Level::Info);
        auto&& simParticipant = _simTestHarness->GetParticipant(_participantNameNetSim);
        auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
        auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();
        auto&& networkSimulator = simParticipant->GetOrCreateNetworkSimulator();

        auto simulatedNetwork =
            std::make_unique<MySimulatedNetwork>(SimulatedNetworkType::FlexRay, _simulatedNetworkName);
        auto simulatedNetworkPtr = simulatedNetwork.get();
        networkSimulator->SimulateNetwork(_simulatedNetworkName, SimulatedNetworkType::FlexRay,
                                          std::move(simulatedNetwork));

        for (const auto& emptyNetworkName : _emptyNetworks)
        {
            auto emptyFlexrayNetwork =
                std::make_unique<MySimulatedNetwork>(SimulatedNetworkType::FlexRay, emptyNetworkName);
            networkSimulator->SimulateNetwork(emptyNetworkName, SimulatedNetworkType::FlexRay,
                                              std::move(emptyFlexrayNetwork));
        }

        // The network simulator has a simulated controller as well
        auto&& participant = simParticipant->Participant();
        auto&& flexrayController = participant->CreateFlexrayController("FlexRay1", _simulatedNetworkName);
        SetupFlexrayController(lifecycleService, flexrayController, callCounts.silKitHandlersFlexray);

        networkSimulator->Start();

        timeSyncService->SetSimulationStepHandler(
            [this, simulatedNetworkPtr, lifecycleService, flexrayController](
                auto now, const std::chrono::nanoseconds /*duration*/) {
                if (now == _stopAtMs)
                {
                    lifecycleService->Stop("stopping the simulation");
                }
                else
                {
                    OnetimeActions(now, flexrayController);
                    SendFlexrayFrames(simulatedNetworkPtr, now, callCounts.silKitSentMsgFlexray.SentFrames);
                }
            },
            _stepSize);
    }

    {
        // ----------------------------
        // Simulated Participants
        // ----------------------------

        for (const auto& participantName : _participantNamesSimulated)
        {
            auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
            auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
            auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();

            auto&& participant = simParticipant->Participant();
            auto&& flexrayController = participant->CreateFlexrayController("FlexRay1", _simulatedNetworkName);
            SetupFlexrayController(lifecycleService, flexrayController, callCounts.silKitHandlersFlexray);

            timeSyncService->SetSimulationStepHandler(
                [this, flexrayController](auto now, const std::chrono::nanoseconds /*duration*/) {
                    OnetimeActions(now, flexrayController);
                },
                _stepSize);
        }
    }

    {
        // ----------------------------
        // Dummy Trivial Participants
        // ----------------------------

        for (const auto& participantName : _participantNamesTrivial)
        {
            auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
            auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();

            timeSyncService->SetSimulationStepHandler([](auto /*now*/, const std::chrono::nanoseconds /*duration*/) {},
                                                      _stepSize);
        }
    }

    auto ok = _simTestHarness->Run(5s);
    ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";

    const size_t numSimulatedFlexrayControllers =
        _numParticipantsSimulated + 1; // +1 for the FlexRay controller on the Netsim participant
    const size_t numSentFramesSimulated = _numFramesPerSimStep * _numSimSteps;
    const size_t numReceivedFramesSimulated = numSimulatedFlexrayControllers * numSentFramesSimulated;

    EXPECT_EQ(callCounts.simulatedNetwork.EventProducer, _numSimulatedNetworks);
    EXPECT_EQ(callCounts.simulatedNetwork.ProvideSimulatedController, numSimulatedFlexrayControllers);

    EXPECT_EQ(callCounts.silKitSentMsgFlexray.SentFrames, numSentFramesSimulated);
    EXPECT_EQ(callCounts.silKitHandlersFlexray.FrameHandler, numReceivedFramesSimulated);
    EXPECT_EQ(callCounts.silKitHandlersFlexray.FrameTransmitHandler, numReceivedFramesSimulated);

    // Every controller calls Wakeup(), netsim broadcasts back to all controllers: n*n 
    const size_t numSimulatedFlexrayControllersSq = numSimulatedFlexrayControllers * numSimulatedFlexrayControllers;
    EXPECT_EQ(callCounts.silKitHandlersFlexray.PocStatusHandler, numSimulatedFlexrayControllersSq);
    EXPECT_EQ(callCounts.silKitHandlersFlexray.CycleStartHandler, numSimulatedFlexrayControllersSq);
    EXPECT_EQ(callCounts.silKitHandlersFlexray.SymbolHandler, numSimulatedFlexrayControllersSq);
    EXPECT_EQ(callCounts.silKitHandlersFlexray.WakeupHandler, numSimulatedFlexrayControllersSq);

    // Here, SymbolTransmitEvent is sent back only to the triggering controller
    EXPECT_EQ(callCounts.silKitHandlersFlexray.SymbolTransmitHandler, numSimulatedFlexrayControllers);

    // Every controller does these actions once
    EXPECT_EQ(callCounts.netSimFlexray.OnControllerConfig, numSimulatedFlexrayControllers);
    EXPECT_EQ(callCounts.netSimFlexray.OnHostCommand, numSimulatedFlexrayControllers);
    EXPECT_EQ(callCounts.netSimFlexray.OnTxBufferConfigUpdate, numSimulatedFlexrayControllers);
    EXPECT_EQ(callCounts.netSimFlexray.OnTxBufferUpdate, numSimulatedFlexrayControllers);

}

} //end namespace
