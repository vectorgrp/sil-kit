// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "ITest_NetSim.hpp"
#include "silkit/services/lin/all.hpp"
#include "silkit/experimental/services/lin/LinControllerExtensions.hpp"

namespace {

using namespace std::chrono_literals;

using namespace SilKit::Tests;
using namespace SilKit::Config;
using namespace SilKit::Services;
using namespace SilKit::Experimental::NetworkSimulation;

using namespace SilKit::Services::Lin;
using namespace SilKit::Experimental::NetworkSimulation::Lin;


struct ITest_NetSimLin : ITest_NetSim
{
    void SendLinFrameHeaders(std::chrono::nanoseconds now, ILinController* controller,
                                        std::atomic_uint& sendCount)
    {
        if (now < _sendUntilMs)
        {
            for (size_t i = 0; i < _numFramesPerSimStep; ++i)
            {
                controller->SendFrameHeader(34);
                sendCount++;
            }
        }
    }

    void SendLinFrames(std::chrono::nanoseconds now, ILinController* controller,
                                        std::atomic_uint& sendCount)
    {
        LinFrame frame{};
        frame.checksumModel = LinChecksumModel::Classic;
        frame.id = 16;
        frame.dataLength = 8;
        frame.data = std::array<uint8_t, 8>{3, 4, 3, 4, 3, 4, 3, 4};

        if (now < _sendUntilMs)
        {
            for (size_t i = 0; i < _numFramesPerSimStep; ++i)
            {
                controller->SendFrame(frame, LinFrameResponseType::MasterResponse);
                sendCount++;
            }
        }
    }

    void WakeupOnce(std::chrono::nanoseconds now, ILinController* controller)
    {
        if (now == 0ns)
        {
            controller->Wakeup();
        }
    }

    void GoToSleepOnce(std::chrono::nanoseconds now, ILinController* controller)
    {
        if (now == 0ns)
        {
            controller->GoToSleep(); // Sends a GoToSleepFrame + ControllerStatusUpdate
        }
    }

    void SetupLinController(Orchestration::ILifecycleService* lifecycleService, ILinController* controller,
                            CallCountsSilKitHandlersLin& callCountsSilKitHandlersLin, bool isMaster, bool isDynamic)
    {
        controller->AddFrameStatusHandler(
            [&callCountsSilKitHandlersLin](ILinController*,
                                          const LinFrameStatusEvent& /*msg*/) {
                callCountsSilKitHandlersLin.FrameStatusHandler++;
            });
        SilKit::Experimental::Services::Lin::AddFrameHeaderHandler(
            controller,
            [isMaster, isDynamic, & callCountsSilKitHandlersLin](
                            ILinController* controller, const SilKit::Experimental::Services::Lin::LinFrameHeaderEvent& /*msg*/) {
                callCountsSilKitHandlersLin.FrameHeaderHandler++;

                ASSERT_TRUE(isDynamic);

                LinFrame frame{};
                frame.checksumModel = LinChecksumModel::Classic;
                frame.id = 16;
                frame.dataLength = 8;
                frame.data = std::array<uint8_t, 8>{3, 4, 3, 4, 3, 4, 3, 4};
                if (isMaster)
                {
                    // Sends a LinFrameResponseUpdate
                    SilKit::Experimental::Services::Lin::SendDynamicResponse(controller, frame);
                }
            });
        controller->AddGoToSleepHandler(
            [&callCountsSilKitHandlersLin](ILinController*,
                                           const LinGoToSleepEvent& /*msg*/) {
                callCountsSilKitHandlersLin.GoToSleepHandler++;
            });
        controller->AddWakeupHandler(
            [&callCountsSilKitHandlersLin](ILinController*,
                                           const LinWakeupEvent& /*msg*/) {
                callCountsSilKitHandlersLin.WakeupHandler++;
            });

        lifecycleService->SetCommunicationReadyHandler([isMaster, isDynamic, controller] {
            
            if (isDynamic)
            {
                SilKit::Experimental::Services::Lin::LinControllerDynamicConfig config{};
                config.baudRate = 20'000;
                config.controllerMode = isMaster ? LinControllerMode::Master : LinControllerMode::Slave;
                SilKit::Experimental::Services::Lin::InitDynamic(controller, config);
            }
            else
            {
                SilKit::Services::Lin::LinControllerConfig config{};
                config.baudRate = 20'000;

                if (isMaster)
                {
                    LinFrameResponse response_16;
                    response_16.frame.id = 16;
                    response_16.frame.checksumModel = LinChecksumModel::Classic;
                    response_16.frame.dataLength = 8;
                    response_16.frame.data = std::array<uint8_t, 8>{3, 4, 3, 4, 3, 4, 3, 4};
                    response_16.responseMode = LinFrameResponseMode::TxUnconditional;

                    LinFrameResponse response_34;
                    response_34.frame.id = 34;
                    response_34.frame.checksumModel = LinChecksumModel::Enhanced;
                    response_34.frame.dataLength = 6;
                    response_34.responseMode = LinFrameResponseMode::Rx;

                    config.controllerMode = LinControllerMode::Master;
                    config.frameResponses.push_back(response_16);
                    config.frameResponses.push_back(response_34);
                }
                else
                {
                    LinFrameResponse response_16;
                    response_16.frame.id = 16;
                    response_16.frame.checksumModel = LinChecksumModel::Classic;
                    response_16.frame.dataLength = 8;
                    response_16.responseMode = LinFrameResponseMode::Rx;

                    LinFrameResponse response_34;
                    response_34.frame.id = 34;
                    response_34.frame.checksumModel = LinChecksumModel::Enhanced;
                    response_34.frame.dataLength = 8;
                    response_34.frame.data = std::array<uint8_t, 8>{1, 2, 3, 4, 3, 4, 3, 4};
                    response_34.responseMode = LinFrameResponseMode::TxUnconditional;

                    config.controllerMode = LinControllerMode::Slave;
                    config.frameResponses.push_back(response_16);
                    config.frameResponses.push_back(response_34);
                }

                controller->Init(config);
            }
            
        });

    }

};

class MySimulatedLinController
    : public MySimulatedController
    , public ISimulatedLinController
{
public:
    MySimulatedLinController(MySimulatedNetwork* mySimulatedNetwork, ControllerDescriptor controllerDescriptor, bool isDynamic = false)
        : MySimulatedController(mySimulatedNetwork, controllerDescriptor), _isDynamic{isDynamic}
    {
    }
    void OnFrameRequest(const LinFrameRequest& linFrameRequest) override;
    void OnFrameHeaderRequest(const LinFrameHeaderRequest& linFrameHeaderRequest) override;
    void OnWakeupPulse(const LinWakeupPulse& linWakeupPulse) override;
    void OnControllerConfig(const SilKit::Experimental::NetworkSimulation::Lin::LinControllerConfig& linControllerConfig) override;
    void OnFrameResponseUpdate(const LinFrameResponseUpdate& linFrameResponseUpdate) override;
    void OnControllerStatusUpdate(const LinControllerStatusUpdate& linControllerStatusUpdate) override;

    bool _isDynamic;
};

auto MySimulatedNetwork::ProvideSimulatedController(ControllerDescriptor controllerDescriptor) -> ISimulatedController*
{
    callCounts.simulatedNetwork.ProvideSimulatedController++;
    _controllerDescriptors.push_back(controllerDescriptor);

    switch (_networkType)
    {
    case SimulatedNetworkType::LIN:
    {
        _mySimulatedControllers.emplace_back(std::make_unique<MySimulatedLinController>(this, controllerDescriptor, _isLinDynamic));
        return _mySimulatedControllers.back().get();
    }
    default:
        break;
    }

    return {};
}

// ISimulatedLinController

void MySimulatedLinController::OnFrameRequest(const LinFrameRequest& linFrameRequest)
{
    callCounts.netSimLin.OnFrameRequest++;

    if (linFrameRequest.frame.id == 60)
    {
        return; // Don't distribute GoToSleepFrame, done in OnControllerStatusUpdate
    }

    LinFrameStatusEvent frameStatusEvent{};
    frameStatusEvent.status = LinFrameStatus::LIN_RX_OK;

    _mySimulatedNetwork->GetLinEventProducer()->Produce(frameStatusEvent,
                                                        _mySimulatedNetwork->GetAllControllerDescriptors());
}

void MySimulatedLinController::OnFrameHeaderRequest(const LinFrameHeaderRequest& linFrameHeaderRequest)
{
    callCounts.netSimLin.OnFrameHeaderRequest++;

    LinSendFrameHeaderRequest frameHeaderRequest{};
    frameHeaderRequest.id = linFrameHeaderRequest.id;

    _mySimulatedNetwork->GetLinEventProducer()->Produce(frameHeaderRequest,
                                                        _mySimulatedNetwork->GetAllControllerDescriptors());
}

void MySimulatedLinController::OnWakeupPulse(const LinWakeupPulse& /*linWakeupPulse*/)
{
    callCounts.netSimLin.OnWakeupPulse++;

    LinWakeupEvent wakeupEvent{};
    _mySimulatedNetwork->GetLinEventProducer()->Produce(wakeupEvent,
                                                        _mySimulatedNetwork->GetAllControllerDescriptors());
}

void MySimulatedLinController::OnControllerConfig(
    const SilKit::Experimental::NetworkSimulation::Lin::LinControllerConfig& /*linControllerConfig*/)
{
    callCounts.netSimLin.OnControllerConfig++;
}

void MySimulatedLinController::OnFrameResponseUpdate(const LinFrameResponseUpdate& /*linFrameResponseUpdate*/)
{
    callCounts.netSimLin.OnFrameResponseUpdate++;

    if (_isDynamic)
    {
        // In dynamic mode, path is:
        // controller->SendFrameHeader(): SendMsg(LinSendFrameHeaderRequest)
        // netsim->OnFrameHeaderRequest(): Produce(LinSendFrameHeaderRequest)
        // Controller::FrameHeaderHandler(): SendDynamicResponse -> SendMsg(LinFrameResponseUpdate) 
        // netsim->OnFrameResponseUpdate(): Produce(LinFrameStatusEvent)
        // Controller::FrameStatusHandler()

        LinFrameStatusEvent frameStatusEvent{};
        frameStatusEvent.status = LinFrameStatus::LIN_RX_OK;

        _mySimulatedNetwork->GetLinEventProducer()->Produce(frameStatusEvent,
                                                            _mySimulatedNetwork->GetAllControllerDescriptors());
    }
}

void MySimulatedLinController::OnControllerStatusUpdate(const LinControllerStatusUpdate& linControllerStatusUpdate)
{
    callCounts.netSimLin.OnControllerStatusUpdate++;

    if (linControllerStatusUpdate.status == LinControllerStatus::SleepPending)
    {
        LinFrameStatusEvent goToSleepFrame{};
        goToSleepFrame.status = LinFrameStatus::LIN_RX_OK;
        goToSleepFrame.frame.id = 60;
        goToSleepFrame.frame.dataLength = 8;
        goToSleepFrame.frame.data = {0x0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

        _mySimulatedNetwork->GetLinEventProducer()->Produce(goToSleepFrame,
                                                            _mySimulatedNetwork->GetAllControllerDescriptors());
    }
}

// Testing NetSim API with LIN
// Covers:
// - Trivial and simulated LinControllers in one simulation
// - Correct routing of simulated LIN messages
// - Network Simulator participant has LinControllers itself

TEST_F(ITest_NetSimLin, basic_networksimulation_lin)
{
    {
        // ----------------------------
        // NetworkSimulator, LIN slave
        // ----------------------------

        //auto configWithLogging = MakeParticipantConfigurationStringWithLogging(SilKit::Services::Logging::Level::Info);
        auto&& simParticipant = _simTestHarness->GetParticipant(_participantNameNetSim);
        auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
        auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();
        auto&& networkSimulator = simParticipant->GetOrCreateNetworkSimulator();

        auto simulatedNetwork = std::make_unique<MySimulatedNetwork>(SimulatedNetworkType::LIN, _simulatedNetworkName);
        networkSimulator->SimulateNetwork(_simulatedNetworkName, SimulatedNetworkType::LIN,
                                          std::move(simulatedNetwork));

        for (const auto& emptyNetworkName : _emptyNetworks)
        {
            auto emptyLinNetwork =
                std::make_unique<MySimulatedNetwork>(SimulatedNetworkType::FlexRay, emptyNetworkName);
            networkSimulator->SimulateNetwork(emptyNetworkName, SimulatedNetworkType::FlexRay,
                                              std::move(emptyLinNetwork));
        }

        // The network simulator has a simulated controller as well
        auto&& participant = simParticipant->Participant();
        auto&& linController = participant->CreateLinController("LIN1", _simulatedNetworkName);
        SetupLinController(lifecycleService, linController, callCounts.silKitHandlersLinSimulated, false, false);

        networkSimulator->Start();

        timeSyncService->SetSimulationStepHandler(
            [this, lifecycleService](auto now, const std::chrono::nanoseconds /*duration*/) {
                if (now == _stopAtMs)
                {
                    lifecycleService->Stop("stopping the simulation");
                }
            },
            _stepSize);
    }

    {
        // ------------------------------------------------------------------
        // Simulated Participants, first one is LIN master, others are slaves
        // ------------------------------------------------------------------

        bool isLinMaster = true;
        for (const auto& participantName : _participantNamesSimulated)
        {
            auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
            auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
            auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();

            auto&& participant = simParticipant->Participant();
            auto&& linController = participant->CreateLinController("LIN1", _simulatedNetworkName);
            SetupLinController(lifecycleService, linController, callCounts.silKitHandlersLinSimulated, isLinMaster, false);

            timeSyncService->SetSimulationStepHandler(
                [this, linController, isLinMaster](auto now, const std::chrono::nanoseconds /*duration*/) {
                    if (isLinMaster)
                    {
                        SendLinFrames(now, linController, callCounts.silKitSentMsgLin.SentFramesSimulated);
                        SendLinFrameHeaders(now, linController, callCounts.silKitSentMsgLin.SentFrameHeadersSimulated);
                        GoToSleepOnce(now, linController);
                        WakeupOnce(now, linController);
                    }
                },
                _stepSize);

            isLinMaster = false;
        }
    }

    {
        // ----------------------------------------------------------------
        // Trivial Participants, first one is LIN master, others are slaves
        // ----------------------------------------------------------------

        bool isLinMaster = true;
        for (const auto& participantName : _participantNamesTrivial)
        {
            auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
            auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
            auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();

            auto&& participant = simParticipant->Participant();
            auto&& linController = participant->CreateLinController("LIN1", _trivialNetworkName);
            
            SetupLinController(lifecycleService, linController, callCounts.silKitHandlersLinTrivial, isLinMaster, false);

            timeSyncService->SetSimulationStepHandler(
                [this, linController, isLinMaster](auto now, const std::chrono::nanoseconds /*duration*/) {
                    if (isLinMaster)
                    {
                        SendLinFrames(now, linController, callCounts.silKitSentMsgLin.SentFramesTrivial);
                        SendLinFrameHeaders(now, linController, callCounts.silKitSentMsgLin.SentFrameHeadersTrivial);
                        GoToSleepOnce(now, linController);
                        WakeupOnce(now, linController);
                    }
                },
                _stepSize);

            isLinMaster = false;
        }
    }

    auto ok = _simTestHarness->Run(5s);
    ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";

    const size_t numSimulatedLinControllers = _numParticipantsSimulated + 1; // +1 for the LIN controller on the Netsim participant
    const size_t numTrivialLinControllers = _numParticipantsTrivial; 
    const size_t numSentFramesSimulated = 1 * _numFramesPerSimStep * _numSimSteps; // Only the single master will send (1 * ...)
    const size_t numSentFrameHeadersSimulated = 1 * _numFramesPerSimStep * _numSimSteps; 
    const size_t numSentFramesTrivial = 1 * _numFramesPerSimStep * _numSimSteps; 
    const size_t numSentFrameHeadersTrivial = 1 * _numFramesPerSimStep * _numSimSteps; 

    EXPECT_EQ(callCounts.simulatedNetwork.EventProducer, _numSimulatedNetworks);
    EXPECT_EQ(callCounts.simulatedNetwork.ProvideSimulatedController, numSimulatedLinControllers);

    // + 1 for GoToSleepFrame
    EXPECT_EQ(callCounts.silKitHandlersLinSimulated.FrameStatusHandler, (numSentFramesSimulated  + 1) * numSimulatedLinControllers ); 

    // + 1 for GoToSleepFrame
    // Trivial: SendFrameHeader also leads to FrameStatusHandler if ID is configured for TX -> numSentFrameHeadersTrivial included
    EXPECT_EQ(callCounts.silKitHandlersLinTrivial.FrameStatusHandler, (numSentFramesTrivial + numSentFrameHeadersTrivial) * numTrivialLinControllers + 1);
    
    // Not dynamic: No Handler triggered
    // See: LinController::ReceiveMsg(const IServiceEndpoint* from, const LinSendFrameHeaderRequest& msg)
    EXPECT_EQ(callCounts.silKitHandlersLinSimulated.FrameHeaderHandler, 0);
    // Trivial: No Handler triggered, but a LinTransmission and thus FrameStatusHandler
    EXPECT_EQ(callCounts.silKitHandlersLinTrivial.FrameHeaderHandler, 0);
    
    // - 1 because the handler is not called on the master
    EXPECT_EQ(callCounts.silKitHandlersLinSimulated.GoToSleepHandler, numSimulatedLinControllers - 1);  
    EXPECT_EQ(callCounts.silKitHandlersLinTrivial.GoToSleepHandler, numTrivialLinControllers - 1);

    EXPECT_EQ(callCounts.silKitHandlersLinSimulated.WakeupHandler, numSimulatedLinControllers);
    EXPECT_EQ(callCounts.silKitHandlersLinTrivial.WakeupHandler, numTrivialLinControllers);

    // +1 for GoToSleepFrame
    EXPECT_EQ(callCounts.netSimLin.OnFrameRequest, numSentFramesSimulated + 1 ); 

    EXPECT_EQ(callCounts.netSimLin.OnFrameHeaderRequest, numSentFrameHeadersSimulated);

    // Oneshot by the masters ILinController::Wakeup
    EXPECT_EQ(callCounts.netSimLin.OnWakeupPulse, 1);

    EXPECT_EQ(callCounts.netSimLin.OnControllerConfig, numSimulatedLinControllers);

    // linController->SendFrame will send a LinFrameResponseUpdate+LinSendFrameRequest 
    EXPECT_EQ(callCounts.netSimLin.OnFrameResponseUpdate, numSentFramesSimulated);

    // Once via ILinController::Wakeup, once via ILinController::GoToSleep
    EXPECT_EQ(callCounts.netSimLin.OnControllerStatusUpdate, 2); 

    EXPECT_EQ(callCounts.silKitSentMsgLin.SentFramesSimulated, numSentFramesSimulated);
    EXPECT_EQ(callCounts.silKitSentMsgLin.SentFrameHeadersSimulated, numSentFrameHeadersSimulated);
    EXPECT_EQ(callCounts.silKitSentMsgLin.SentFramesTrivial, numSentFramesTrivial);
    EXPECT_EQ(callCounts.silKitSentMsgLin.SentFrameHeadersTrivial, numSentFrameHeadersTrivial);
}


// Testing NetSim API with LIN Experimental Dynamic Mode

TEST_F(ITest_NetSimLin, networksimulation_lin_dynamic)
{
    const bool isDynamic = true;
    {
        // ----------------------------
        // NetworkSimulator, LIN slave
        // ----------------------------

        //auto configWithLogging = MakeParticipantConfigurationStringWithLogging(SilKit::Services::Logging::Level::Info);
        auto&& simParticipant = _simTestHarness->GetParticipant(_participantNameNetSim);
        auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
        auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();
        auto&& networkSimulator = simParticipant->GetOrCreateNetworkSimulator();

        auto simulatedNetwork =
            std::make_unique<MySimulatedNetwork>(SimulatedNetworkType::LIN, _simulatedNetworkName, isDynamic);
        networkSimulator->SimulateNetwork(_simulatedNetworkName, SimulatedNetworkType::LIN,
                                          std::move(simulatedNetwork));

        for (const auto& emptyNetworkName : _emptyNetworks)
        {
            auto emptyLinNetwork =
                std::make_unique<MySimulatedNetwork>(SimulatedNetworkType::FlexRay, emptyNetworkName);
            networkSimulator->SimulateNetwork(emptyNetworkName, SimulatedNetworkType::FlexRay,
                                              std::move(emptyLinNetwork));
        }

        // The network simulator has a simulated controller as well
        auto&& participant = simParticipant->Participant();
        auto&& linController = participant->CreateLinController("LIN1", _simulatedNetworkName);
        SetupLinController(lifecycleService, linController, callCounts.silKitHandlersLinSimulated, false, isDynamic);

        networkSimulator->Start();

        timeSyncService->SetSimulationStepHandler(
            [this, lifecycleService](auto now, const std::chrono::nanoseconds /*duration*/) {
                if (now == _stopAtMs)
                {
                    lifecycleService->Stop("stopping the simulation");
                }
            },
            _stepSize);
    }

    {
        // ------------------------------------------------------------------
        // Simulated Participants, first one is LIN master, others are slaves
        // ------------------------------------------------------------------

        bool isLinMaster = true;
        for (const auto& participantName : _participantNamesSimulated)
        {
            auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
            auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
            auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();

            auto&& participant = simParticipant->Participant();
            auto&& linController = participant->CreateLinController("LIN1", _simulatedNetworkName);
            SetupLinController(lifecycleService, linController, callCounts.silKitHandlersLinSimulated, isLinMaster,
                               isDynamic);

            timeSyncService->SetSimulationStepHandler(
                [this, linController, isLinMaster](auto now, const std::chrono::nanoseconds /*duration*/) {
                    if (isLinMaster)
                    {
                        // Dynamic mode: Only send frameHeaders
                        SendLinFrameHeaders(now, linController, callCounts.silKitSentMsgLin.SentFrameHeadersSimulated);
                        GoToSleepOnce(now, linController);
                        WakeupOnce(now, linController);
                    }
                },
                _stepSize);

            isLinMaster = false;
        }
    }

    {
        // ----------------------------------------------------------------
        // Trivial Participants, first one is LIN master, others are slaves
        // ----------------------------------------------------------------

        bool isLinMaster = true;
        for (const auto& participantName : _participantNamesTrivial)
        {
            auto&& simParticipant = _simTestHarness->GetParticipant(participantName);
            auto&& lifecycleService = simParticipant->GetOrCreateLifecycleService();
            auto&& timeSyncService = simParticipant->GetOrCreateTimeSyncService();

            auto&& participant = simParticipant->Participant();
            auto&& linController = participant->CreateLinController("LIN1", _trivialNetworkName);

            // Not dynamic!
            SetupLinController(lifecycleService, linController, callCounts.silKitHandlersLinTrivial, isLinMaster,
                               false);

            timeSyncService->SetSimulationStepHandler(
                [this, linController, isLinMaster](auto now, const std::chrono::nanoseconds /*duration*/) {
                    if (isLinMaster)
                    {
                        SendLinFrames(now, linController, callCounts.silKitSentMsgLin.SentFramesTrivial);
                        SendLinFrameHeaders(now, linController, callCounts.silKitSentMsgLin.SentFrameHeadersTrivial);
                        WakeupOnce(now, linController);
                        GoToSleepOnce(now, linController);
                    }
                },
                _stepSize);

            isLinMaster = false;
        }
    }

    auto ok = _simTestHarness->Run(5s);
    ASSERT_TRUE(ok) << "SimTestHarness should terminate without timeout";

    const size_t numSimulatedLinControllers =
        _numParticipantsSimulated + 1; // +1 for the LIN controller on the Netsim participant
    const size_t numTrivialLinControllers = _numParticipantsTrivial;
    const size_t numSentFramesSimulated = 0; // No direct call of SendFrame in dynamic mode
    const size_t numSentFrameHeadersSimulated = 1 * _numFramesPerSimStep * _numSimSteps;
    const size_t numSentFramesTrivial = 1 * _numFramesPerSimStep * _numSimSteps;
    const size_t numSentFrameHeadersTrivial = 1 * _numFramesPerSimStep * _numSimSteps;

    EXPECT_EQ(callCounts.simulatedNetwork.EventProducer, _numSimulatedNetworks);
    EXPECT_EQ(callCounts.simulatedNetwork.ProvideSimulatedController, numSimulatedLinControllers);

    // Each sent FrameHeader leads to a Transmission + 1 for GoToSleepFrame
    EXPECT_EQ(callCounts.silKitHandlersLinSimulated.FrameStatusHandler,
              (numSentFrameHeadersSimulated + 1) * numSimulatedLinControllers);

    // + 1 for GoToSleepFrame
    // Trivial: SendFrameHeader also leads to FrameStatusHandler if ID is configured for TX -> numSentFrameHeadersTrivial included
    EXPECT_EQ(callCounts.silKitHandlersLinTrivial.FrameStatusHandler,
              (numSentFramesTrivial + numSentFrameHeadersTrivial) * numTrivialLinControllers + 1);

    // Dynamic: Handler triggered
    // See: LinController::ReceiveMsg(const IServiceEndpoint* from, const LinSendFrameHeaderRequest& msg)
    EXPECT_EQ(callCounts.silKitHandlersLinSimulated.FrameHeaderHandler, numSentFrameHeadersSimulated * numSimulatedLinControllers);
    // Trivial: No Handler triggered, but a LinTransmission and thus FrameStatusHandler
    EXPECT_EQ(callCounts.silKitHandlersLinTrivial.FrameHeaderHandler, 0);

    // - 1 because the handler is not called on the master
    EXPECT_EQ(callCounts.silKitHandlersLinSimulated.GoToSleepHandler, numSimulatedLinControllers - 1);
    EXPECT_EQ(callCounts.silKitHandlersLinTrivial.GoToSleepHandler, numTrivialLinControllers - 1);

    EXPECT_EQ(callCounts.silKitHandlersLinSimulated.WakeupHandler, numSimulatedLinControllers);
    EXPECT_EQ(callCounts.silKitHandlersLinTrivial.WakeupHandler, numTrivialLinControllers);

    // Only the GoToSleepFrame comes in as a FrameRequest in dynamic mode
    EXPECT_EQ(callCounts.netSimLin.OnFrameRequest, 1);

    EXPECT_EQ(callCounts.netSimLin.OnFrameHeaderRequest, numSentFrameHeadersSimulated);

    // Oneshot by the masters ILinController::Wakeup
    EXPECT_EQ(callCounts.netSimLin.OnWakeupPulse, 1);

    EXPECT_EQ(callCounts.netSimLin.OnControllerConfig, numSimulatedLinControllers);

    // linController->SendFrameHeader leads to a LinFrameResponseUpdate via SendDynamicResponse
    EXPECT_EQ(callCounts.netSimLin.OnFrameResponseUpdate, numSentFrameHeadersSimulated);

    // Once via ILinController::Wakeup, once via ILinController::GoToSleep
    EXPECT_EQ(callCounts.netSimLin.OnControllerStatusUpdate, 2);

    EXPECT_EQ(callCounts.silKitSentMsgLin.SentFramesSimulated, numSentFramesSimulated);
    EXPECT_EQ(callCounts.silKitSentMsgLin.SentFrameHeadersSimulated, numSentFrameHeadersSimulated);
    EXPECT_EQ(callCounts.silKitSentMsgLin.SentFramesTrivial, numSentFramesTrivial);
    EXPECT_EQ(callCounts.silKitSentMsgLin.SentFrameHeadersTrivial, numSentFrameHeadersTrivial);
}

} //end namespace
