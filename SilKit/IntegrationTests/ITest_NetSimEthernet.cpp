// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ITest_NetSim.hpp"
#include "silkit/services/ethernet/all.hpp"

namespace {

using namespace std::chrono_literals;

using namespace SilKit::Config;
using namespace SilKit::Services;
using namespace SilKit::Experimental::NetworkSimulation;

using namespace SilKit::Services::Ethernet;
using namespace SilKit::Experimental::NetworkSimulation::Ethernet;

struct ITest_NetSimEthernet : ITest_NetSim
{
    void SendEthernetFrames(std::chrono::nanoseconds now, IEthernetController* controller, std::atomic_uint& sendCount)
    {
        std::array<uint8_t, 1> dataBytes{78};

        EthernetFrame frame{};
        frame.raw = SilKit::Util::MakeSpan(dataBytes);

        if (now < _sendUntilMs)
        {
            for (size_t i = 0; i < _numFramesPerSimStep; ++i)
            {
                controller->SendFrame(frame);
                sendCount++;
            }
        }
    }

    void SetupEthernetController(Orchestration::ILifecycleService* lifecycleService, IEthernetController* controller,
                                 CallCountsSilKitHandlersEthernet& callCountsSilKitHandlerEthernet)
    {
        controller->AddFrameHandler(
            [&callCountsSilKitHandlerEthernet](IEthernetController*, const EthernetFrameEvent& /*msg*/) {
                callCountsSilKitHandlerEthernet.FrameHandler++;
            });
        controller->AddFrameTransmitHandler(
            [&callCountsSilKitHandlerEthernet](IEthernetController*, const EthernetFrameTransmitEvent& /*msg*/) {
                callCountsSilKitHandlerEthernet.FrameTransmitHandler++;
            });
        controller->AddStateChangeHandler(
            [&callCountsSilKitHandlerEthernet](IEthernetController*, const EthernetStateChangeEvent& /*msg*/) {
                callCountsSilKitHandlerEthernet.StateChangeHandler++;
            });

        controller->AddBitrateChangeHandler(
            [&callCountsSilKitHandlerEthernet](IEthernetController*, const EthernetBitrateChangeEvent& /*msg*/) {
                callCountsSilKitHandlerEthernet.BitrateChangeHandler++;
            });

        lifecycleService->SetCommunicationReadyHandler([controller] {
            controller->Activate();
        });
    }
};

class MySimulatedEthernetController
    : public MySimulatedController
    , public ISimulatedEthernetController
{
public:
    MySimulatedEthernetController(MySimulatedNetwork* mySimulatedNetwork, ControllerDescriptor controllerDescriptor)
        : MySimulatedController(mySimulatedNetwork, controllerDescriptor)
    {
    }

    void OnFrameRequest(const EthernetFrameRequest& ethernetFrameRequest) override;
    void OnSetControllerMode(const EthernetControllerMode& ethernetControllerMode) override;
};


auto MySimulatedNetwork::ProvideSimulatedController(ControllerDescriptor controllerDescriptor) -> ISimulatedController*
{
    callCounts.simulatedNetwork.ProvideSimulatedController++;
    _controllerDescriptors.push_back(controllerDescriptor);

    switch (_networkType)
    {
    case SimulatedNetworkType::Ethernet:
    {
        _mySimulatedControllers.emplace_back(
            std::make_unique<MySimulatedEthernetController>(this, controllerDescriptor));
        return _mySimulatedControllers.back().get();
    }
    default:
        break;
    }

    return {};
}

// ISimulatedEthernetController

void MySimulatedEthernetController::OnSetControllerMode(const EthernetControllerMode& /*controllerMode*/)
{
    callCounts.netSimEthernet.OnSetControllerMode++;

    std::array<ControllerDescriptor, 1> receiverArray{_controllerDescriptor};
    auto receiver = SilKit::Util::MakeSpan(receiverArray);

    EthernetStateChangeEvent stateChangeEvent{};
    stateChangeEvent.state = EthernetState::LinkUp;
    _mySimulatedNetwork->GetEthernetEventProducer()->Produce(std::move(stateChangeEvent), receiver);

    EthernetBitrateChangeEvent bitrateChangeEvent{};
    bitrateChangeEvent.bitrate = 223;
    _mySimulatedNetwork->GetEthernetEventProducer()->Produce(std::move(bitrateChangeEvent), receiver);

}

void MySimulatedEthernetController::OnFrameRequest(const EthernetFrameRequest& frameRequest)
{
    callCounts.netSimEthernet.OnFrameRequest++;

    // Send acknowledge back to the sending controller
    EthernetFrameTransmitEvent ack;
    ack.status = EthernetTransmitStatus::Transmitted;
    ack.userContext = frameRequest.userContext;

    std::array<ControllerDescriptor, 1> receiverArray{_controllerDescriptor};
    auto receiver = SilKit::Util::MakeSpan(receiverArray);
    _mySimulatedNetwork->GetEthernetEventProducer()->Produce(std::move(ack), receiver);

    // Distribute the frame to all controllers in the network
    EthernetFrameEvent frameEvent;
    frameEvent.direction = TransmitDirection::RX;
    frameEvent.frame = frameRequest.ethernetFrame;
    frameEvent.userContext = frameRequest.userContext;
    std::vector<uint8_t> payloadBytes{frameRequest.ethernetFrame.raw.begin(), frameRequest.ethernetFrame.raw.end()};

    frameEvent.frame.raw = SilKit::Util::ToSpan(payloadBytes);
    _mySimulatedNetwork->GetEthernetEventProducer()->Produce(frameEvent,
                                                             _mySimulatedNetwork->GetAllControllerDescriptors());
}

// Testing NetSim API with Ethernet
// Covers:
// - Trivial and simulated EthernetControllers in one simulation
// - Correct routing of simulated Ethernet messages
// - Network Simulator participant has EthernetControllers itself

TEST_F(ITest_NetSimEthernet, basic_networksimulation_ethernet)
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
            std::make_unique<MySimulatedNetwork>(SimulatedNetworkType::Ethernet, _simulatedNetworkName);
        networkSimulator->SimulateNetwork(_simulatedNetworkName, SimulatedNetworkType::Ethernet,
                                          std::move(simulatedNetwork));

        for (const auto& emptyNetworkName : _emptyNetworks)
        {
            auto emptyEthernetNetwork =
                std::make_unique<MySimulatedNetwork>(SimulatedNetworkType::FlexRay, emptyNetworkName);
            networkSimulator->SimulateNetwork(emptyNetworkName, SimulatedNetworkType::FlexRay,
                                              std::move(emptyEthernetNetwork));
        }

        // The network simulator has a simulated controller as well
        auto&& participant = simParticipant->Participant();
        auto&& ethernetController = participant->CreateEthernetController("Ethernet1", _simulatedNetworkName);
        SetupEthernetController(lifecycleService, ethernetController, callCounts.silKitHandlersEthernetSimulated);

        networkSimulator->Start();

        timeSyncService->SetSimulationStepHandler(
            [this, lifecycleService, ethernetController](auto now, const std::chrono::nanoseconds /*duration*/) {
                if (now == _stopAtMs)
                {
                    lifecycleService->Stop("stopping the simulation");
                }
                else
                {
                    SendEthernetFrames(now, ethernetController, callCounts.silKitSentMsgEthernet.SentFramesSimulated);
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
            auto&& ethernetController = participant->CreateEthernetController("Ethernet1", _simulatedNetworkName);
            SetupEthernetController(lifecycleService, ethernetController, callCounts.silKitHandlersEthernetSimulated);

            timeSyncService->SetSimulationStepHandler(
                [this, ethernetController](auto now, const std::chrono::nanoseconds /*duration*/) {
                    SendEthernetFrames(now, ethernetController, callCounts.silKitSentMsgEthernet.SentFramesSimulated);
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
            auto&& ethernetController = participant->CreateEthernetController("Ethernet1", _trivialNetworkName);
            SetupEthernetController(lifecycleService, ethernetController, callCounts.silKitHandlersEthernetTrivial);

            timeSyncService->SetSimulationStepHandler(
                [this, ethernetController](auto now, const std::chrono::nanoseconds /*duration*/) {
                    SendEthernetFrames(now, ethernetController, callCounts.silKitSentMsgEthernet.SentFramesTrivial);
                },
                _stepSize);
        }
    }

    auto ok = _simTestHarness->Run(5s);
    ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";

    const size_t numSimulatedEthernetControllers =
        _numParticipantsSimulated + 1; // +1 for the Ethernet controller on the Netsim participant
    const size_t numSentFramesSimulated = numSimulatedEthernetControllers * _numFramesPerSimStep * _numSimSteps;
    const size_t numSentFramesTrivial = _numParticipantsTrivial * _numFramesPerSimStep * _numSimSteps;

    EXPECT_EQ(callCounts.simulatedNetwork.EventProducer, _numSimulatedNetworks);
    EXPECT_EQ(callCounts.simulatedNetwork.ProvideSimulatedController, numSimulatedEthernetControllers);

    EXPECT_EQ(callCounts.silKitHandlersEthernetSimulated.FrameHandler, numSentFramesSimulated * numSimulatedEthernetControllers);
    EXPECT_EQ(callCounts.silKitHandlersEthernetTrivial.FrameHandler, numSentFramesTrivial * (_numParticipantsTrivial - 1)); // FrameHandler filters messages from sender
    EXPECT_EQ(callCounts.silKitHandlersEthernetSimulated.FrameTransmitHandler, numSentFramesSimulated);
    EXPECT_EQ(callCounts.silKitHandlersEthernetTrivial.FrameTransmitHandler, numSentFramesTrivial);
    EXPECT_EQ(callCounts.silKitHandlersEthernetSimulated.StateChangeHandler, numSimulatedEthernetControllers);
    EXPECT_EQ(callCounts.silKitHandlersEthernetTrivial.StateChangeHandler, _numParticipantsTrivial);
    EXPECT_EQ(callCounts.silKitHandlersEthernetSimulated.BitrateChangeHandler, numSimulatedEthernetControllers);
    EXPECT_EQ(callCounts.silKitHandlersEthernetTrivial.BitrateChangeHandler, 0);

    EXPECT_EQ(callCounts.netSimEthernet.OnFrameRequest, numSentFramesSimulated);
    EXPECT_EQ(callCounts.netSimEthernet.OnSetControllerMode, numSimulatedEthernetControllers);

    EXPECT_EQ(callCounts.silKitSentMsgEthernet.SentFramesSimulated, numSentFramesSimulated);
    EXPECT_EQ(callCounts.silKitSentMsgEthernet.SentFramesTrivial, numSentFramesTrivial);
}


} //end namespace
