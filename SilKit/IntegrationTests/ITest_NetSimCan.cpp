// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ITest_NetSim.hpp"
#include "silkit/services/can/all.hpp"

namespace {

using namespace std::chrono_literals;

using namespace SilKit::Tests;
using namespace SilKit::Config;
using namespace SilKit::Services;
using namespace SilKit::Experimental::NetworkSimulation;

using namespace SilKit::Services::Can;
using namespace SilKit::Experimental::NetworkSimulation::Can;


struct ITest_NetSimCan : ITest_NetSim
{
    void SendCanFrames(std::chrono::nanoseconds now, ICanController* controller,
                                        std::atomic_uint& sendCount)
    {
        std::array<uint8_t, 1> dataBytes{78};

        CanFrame frame{};
        frame.flags = {};
        frame.canId = 0x12;
        frame.dlc = 1;
        frame.dataField = SilKit::Util::MakeSpan(dataBytes);

        if (now < _sendUntilMs)
        {
            for (size_t i = 0; i < _numFramesPerSimStep; ++i)
            {
                controller->SendFrame(frame);
                sendCount++;
            }
        }
    }

    void SetupCanController(Orchestration::ILifecycleService* lifecycleService, ICanController* controller,
                            CallCountsSilKitHandlersCan& callCountsSilKitHandlersCan)
    {
        controller->AddFrameHandler([&callCountsSilKitHandlersCan](ICanController*, const CanFrameEvent& /*msg*/) {
            callCountsSilKitHandlersCan.FrameHandler++;
        });
        controller->AddFrameTransmitHandler(
            [&callCountsSilKitHandlersCan](ICanController*, const CanFrameTransmitEvent& /*msg*/) {
                callCountsSilKitHandlersCan.FrameTransmitHandler++;
            });
        controller->AddStateChangeHandler(
            [&callCountsSilKitHandlersCan](ICanController*, const CanStateChangeEvent& /*msg*/) {
                callCountsSilKitHandlersCan.StateChangeHandler++;
            });
        controller->AddErrorStateChangeHandler(
            [&callCountsSilKitHandlersCan](ICanController*, const CanErrorStateChangeEvent& /*msg*/) {
                callCountsSilKitHandlersCan.ErrorStateChangeHandler++;
            });

        lifecycleService->SetCommunicationReadyHandler([controller] {
            controller->SetBaudRate(10'000, 1'000'000, 2'000'000);
            controller->Start();
        });
    }
};

class MySimulatedCanController
    : public MySimulatedController
    , public ISimulatedCanController
{
public:
    MySimulatedCanController(MySimulatedNetwork* mySimulatedNetwork, ControllerDescriptor controllerDescriptor)
        : MySimulatedController(mySimulatedNetwork, controllerDescriptor)
    {
    }

    void OnSetControllerMode(const CanControllerMode& controllerMode) override;
    void OnSetBaudrate(const CanConfigureBaudrate& configureBaudrate) override;
    void OnFrameRequest(const CanFrameRequest& frameRequest) override;
};


auto MySimulatedNetwork::ProvideSimulatedController(ControllerDescriptor controllerDescriptor) -> ISimulatedController*
{
    callCounts.simulatedNetwork.ProvideSimulatedController++;
    _controllerDescriptors.push_back(controllerDescriptor);

    switch (_networkType)
    {
    case SimulatedNetworkType::CAN:
    {
        _mySimulatedControllers.emplace_back(std::make_unique<MySimulatedCanController>(this, controllerDescriptor));
        return _mySimulatedControllers.back().get();
    }
    default:
        break;
    }

    return {};
}

// ISimulatedCanController

void MySimulatedCanController::OnSetControllerMode(const CanControllerMode& /*controllerMode*/)
{
    callCounts.netSimCan.OnSetControllerMode++;

    std::array<ControllerDescriptor, 1> receiverArray{_controllerDescriptor};
    auto receiver = SilKit::Util::MakeSpan(receiverArray);

    CanStateChangeEvent stateChangeEvent{};
    stateChangeEvent.state = CanControllerState::Started;
    _mySimulatedNetwork->GetCanEventProducer()->Produce(std::move(stateChangeEvent), receiver);

    CanErrorStateChangeEvent errorStateChangeEvent{};
    errorStateChangeEvent.errorState = CanErrorState::BusOff;
    _mySimulatedNetwork->GetCanEventProducer()->Produce(std::move(errorStateChangeEvent), receiver);

}

void MySimulatedCanController::OnSetBaudrate(const CanConfigureBaudrate& /*configureBaudrate*/)
{
    callCounts.netSimCan.OnSetBaudrate++;
}

void MySimulatedCanController::OnFrameRequest(const CanFrameRequest& frameRequest)
{
    callCounts.netSimCan.OnFrameRequest++;

    // Send acknowledge back to the sending controller
    CanFrameTransmitEvent ack;
    ack.canId = frameRequest.frame.canId;
    ack.status = CanTransmitStatus::Transmitted;
    ack.userContext = frameRequest.userContext;

    std::array<ControllerDescriptor, 1> receiverArray{_controllerDescriptor};
    auto receiver = SilKit::Util::MakeSpan(receiverArray);
    _mySimulatedNetwork->GetCanEventProducer()->Produce(std::move(ack), receiver);

    // Distribute the frame to all controllers in the network
    CanFrameEvent frameEvent;
    frameEvent.direction = TransmitDirection::RX;
    frameEvent.frame = frameRequest.frame;
    frameEvent.userContext = frameRequest.userContext;
    std::vector<uint8_t> payloadBytes{frameRequest.frame.dataField.begin(), frameRequest.frame.dataField.end()};

    frameEvent.frame.dataField = SilKit::Util::ToSpan(payloadBytes);
    _mySimulatedNetwork->GetCanEventProducer()->Produce(frameEvent, _mySimulatedNetwork->GetAllControllerDescriptors());
}

// Testing NetSim API with CAN
// Covers:
// - Trivial and simulated CanControllers in one simulation
// - Correct routing of simulated CAN messages
// - Network Simulator participant has CanControllers itself

TEST_F(ITest_NetSimCan, basic_networksimulation_can)
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

        auto simulatedNetwork = std::make_unique<MySimulatedNetwork>(SimulatedNetworkType::CAN, _simulatedNetworkName);
        networkSimulator->SimulateNetwork(_simulatedNetworkName, SimulatedNetworkType::CAN,
                                          std::move(simulatedNetwork));

        for (const auto& emptyNetworkName : _emptyNetworks)
        {
            auto emptyCanNetwork =
                std::make_unique<MySimulatedNetwork>(SimulatedNetworkType::FlexRay, emptyNetworkName);
            networkSimulator->SimulateNetwork(emptyNetworkName, SimulatedNetworkType::FlexRay,
                                              std::move(emptyCanNetwork));
        }

        // The network simulator has a simulated controller as well
        auto&& participant = simParticipant->Participant();
        auto&& canController = participant->CreateCanController("CAN1", _simulatedNetworkName);
        SetupCanController(lifecycleService, canController, callCounts.silKitHandlersCanSimulated);

        networkSimulator->Start();

        timeSyncService->SetSimulationStepHandler(
            [this, lifecycleService, canController](auto now, const std::chrono::nanoseconds /*duration*/) {
                if (now == _stopAtMs)
                {
                    lifecycleService->Stop("stopping the simulation");
                }
                else
                {
                    SendCanFrames(now, canController, callCounts.silKitSentMsgCan.SentFramesSimulated);
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
            auto&& canController = participant->CreateCanController("CAN1", _simulatedNetworkName);
            SetupCanController(lifecycleService, canController, callCounts.silKitHandlersCanSimulated);

            timeSyncService->SetSimulationStepHandler(
                [this, canController](auto now, const std::chrono::nanoseconds /*duration*/) {
                    SendCanFrames(now, canController, callCounts.silKitSentMsgCan.SentFramesSimulated);
                },
                _stepSize);
        }
    }

    {
        // ----------------------------
        // Trivial Participants
        // ----------------------------

        for (const auto& participantName : _participantNamesTrivial)
        {
            auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
            auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
            auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();

            auto&& participant = simParticipant->Participant();
            auto&& canController = participant->CreateCanController("CAN1", _trivialNetworkName);
            SetupCanController(lifecycleService, canController, callCounts.silKitHandlersCanTrivial);

            timeSyncService->SetSimulationStepHandler(
                [this, canController](auto now, const std::chrono::nanoseconds /*duration*/) {
                    SendCanFrames(now, canController, callCounts.silKitSentMsgCan.SentFramesTrivial);
                },
                _stepSize);
        }
    }

    auto ok = _simTestHarness->Run(5s);
    ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";

    const size_t numSimulatedCanControllers =
        _numParticipantsSimulated + 1; // +1 for the CAN controller on the Netsim participant
    const size_t numSentFramesSimulated = numSimulatedCanControllers * _numFramesPerSimStep * _numSimSteps;
    const size_t numSentFramesTrivial = _numParticipantsTrivial * _numFramesPerSimStep * _numSimSteps;

    EXPECT_EQ(callCounts.simulatedNetwork.EventProducer, _numSimulatedNetworks);
    EXPECT_EQ(callCounts.simulatedNetwork.ProvideSimulatedController, numSimulatedCanControllers);

    EXPECT_EQ(callCounts.silKitHandlersCanSimulated.FrameHandler, numSentFramesSimulated * numSimulatedCanControllers);
    EXPECT_EQ(callCounts.silKitHandlersCanTrivial.FrameHandler, numSentFramesTrivial * (_numParticipantsTrivial - 1)); // FrameHandler filters messages from sender
    EXPECT_EQ(callCounts.silKitHandlersCanSimulated.FrameTransmitHandler, numSentFramesSimulated);
    EXPECT_EQ(callCounts.silKitHandlersCanTrivial.FrameTransmitHandler, numSentFramesTrivial);
    EXPECT_EQ(callCounts.silKitHandlersCanSimulated.StateChangeHandler, numSimulatedCanControllers);
    EXPECT_EQ(callCounts.silKitHandlersCanTrivial.StateChangeHandler, _numParticipantsTrivial);
    EXPECT_EQ(callCounts.silKitHandlersCanSimulated.ErrorStateChangeHandler, numSimulatedCanControllers);
    EXPECT_EQ(callCounts.silKitHandlersCanTrivial.ErrorStateChangeHandler, 0);

    EXPECT_EQ(callCounts.netSimCan.OnFrameRequest, numSentFramesSimulated);
    EXPECT_EQ(callCounts.netSimCan.OnSetBaudrate, numSimulatedCanControllers);
    EXPECT_EQ(callCounts.netSimCan.OnSetControllerMode, numSimulatedCanControllers);

    EXPECT_EQ(callCounts.silKitSentMsgCan.SentFramesSimulated, numSentFramesSimulated);
    EXPECT_EQ(callCounts.silKitSentMsgCan.SentFramesTrivial, numSentFramesTrivial);
}

} //end namespace
