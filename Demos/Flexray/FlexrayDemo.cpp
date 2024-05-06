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

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <sstream>
#include <thread>

#include "silkit/SilKit.hpp"
#include "silkit/services/orchestration/all.hpp"
#include "silkit/services/orchestration/string_utils.hpp"
#include "silkit/services/flexray/all.hpp"
#include "silkit/services/flexray/string_utils.hpp"

using namespace SilKit::Services::Orchestration;
using namespace SilKit::Services::Flexray;

using namespace std::chrono_literals;
using namespace std::placeholders;

std::ostream& operator<<(std::ostream& out, std::chrono::nanoseconds timestamp)
{
    auto seconds = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(timestamp);
    out << seconds.count() << "s";
    return out;
}

template<typename T>
void ReceiveMessage(IFlexrayController* /*controller*/, const T& t)
{
    std::cout << ">> " << t << "\n";
}

struct FlexrayNode
{
    FlexrayNode(IFlexrayController* controller, FlexrayControllerConfig config)
        : controller{controller}
        , controllerConfig{std::move(config)}
    {
      oldPocStatus.state = FlexrayPocState::DefaultConfig;
    }

    void SetStartupDelay(std::chrono::nanoseconds delay)
    {
        _startupDelay = delay;
    }

    void Init()
    {
        if (_configureCalled)
            return;

        controller->Configure(controllerConfig);
        _configureCalled = true;
    }

    void doAction(std::chrono::nanoseconds now)
    {
        if (now < _startupDelay)
            return;
        switch (oldPocStatus.state)
        {
        case FlexrayPocState::DefaultConfig:
            Init();
        case FlexrayPocState::Ready:
            return pocReady(now);
        case FlexrayPocState::NormalActive:
            if (now == 100ms + std::chrono::duration_cast<std::chrono::milliseconds>(_startupDelay))
            {
                return ReconfigureTxBuffers();
            }
            else
            {
                return txBufferUpdate(now);
            }
        case FlexrayPocState::Config:
        case FlexrayPocState::Startup:
        case FlexrayPocState::Wakeup:
        case FlexrayPocState::NormalPassive:
        case FlexrayPocState::Halt:
            return;
        }
    }

    void pocReady(std::chrono::nanoseconds /*now*/)
    {
        switch (busState)
        {
        case MasterState::PerformWakeup:
            controller->Wakeup();
            return;
        case MasterState::WaitForWakeup:
            return;
        case MasterState::WakeupDone:
            controller->AllowColdstart();
            controller->Run();
            return;
        default:
            return;
        }
    }

    void txBufferUpdate(std::chrono::nanoseconds /*now*/)
    {
        if (controllerConfig.bufferConfigs.empty())
            return;

        static auto msgNumber = -1;
        msgNumber++;

        auto bufferIdx = msgNumber % controllerConfig.bufferConfigs.size();

        // prepare a friendly message as payload
        std::stringstream payloadStream;
        payloadStream << "FlexrayFrameEvent#" << std::setw(4) << msgNumber
                      << "; bufferId=" << bufferIdx;
        auto payloadString = payloadStream.str();

        std::vector<uint8_t> payloadBytes;
        payloadBytes.resize(payloadString.size());

        std::copy(payloadString.begin(), payloadString.end(), payloadBytes.begin());

        FlexrayTxBufferUpdate update;
        update.payload = payloadBytes;
        update.payloadDataValid = true;
        update.txBufferIndex = static_cast<decltype(update.txBufferIndex)>(bufferIdx);

        controller->UpdateTxBuffer(update);
    }

    // Reconfigure buffers: Swap Channels A and B
    void ReconfigureTxBuffers()
    {
        std::cout << "Reconfiguring TxBuffers. Swapping FlexrayChannel::A and FlexrayChannel::B\n";
        for (uint16_t idx = 0; idx < controllerConfig.bufferConfigs.size(); idx++)
        {
            auto&& bufferConfig = controllerConfig.bufferConfigs[idx];
            switch (bufferConfig.channels)
            {
            case FlexrayChannel::A:
                bufferConfig.channels = FlexrayChannel::B;
                controller->ReconfigureTxBuffer(idx, bufferConfig);
                break;
            case FlexrayChannel::B:
                bufferConfig.channels = FlexrayChannel::A;
                controller->ReconfigureTxBuffer(idx, bufferConfig);
                break;
            default:
                break;
            }
        }
    }

    void PocStatusHandler(IFlexrayController* /*controller*/, const FlexrayPocStatusEvent& pocStatus)
    {
        std::cout << ">> POC=" << pocStatus.state
                  << ", Freeze=" <<  pocStatus.freeze
                  << ", Wakeup=" <<  pocStatus.wakeupStatus
                  << ", Slot=" <<  pocStatus.slotMode
                  << " @t=" << pocStatus.timestamp
                  << std::endl;

        if (oldPocStatus.state == FlexrayPocState::Wakeup
            && pocStatus.state == FlexrayPocState::Ready)
        {
            std::cout << "   Wakeup finished..." << std::endl;
            busState = MasterState::WakeupDone;
        }

        oldPocStatus = pocStatus;
    }

    void WakeupHandler(IFlexrayController* frController, const FlexrayWakeupEvent& flexrayWakeupEvent)
    {
        std::cout << ">> WAKEUP! (" << flexrayWakeupEvent.pattern << ")" << std::endl;
        frController->AllowColdstart();
        frController->Run();
    }


    IFlexrayController* controller = nullptr;

    FlexrayControllerConfig controllerConfig;
    FlexrayPocStatusEvent oldPocStatus{};
    bool _configureCalled = false;
    std::chrono::nanoseconds _startupDelay = 0ns;

    enum class MasterState
    {
        Ignore,
        PerformWakeup,
        WaitForWakeup,
        WakeupDone
    };
    MasterState busState = MasterState::Ignore;
};


auto MakeNodeParams(const std::string& participantName) -> FlexrayNodeParameters
{
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

    if (participantName == "Node0")
    {
        nodeParams.pKeySlotId = 40;
    }
    else if (participantName == "Node1")
    {
        nodeParams.pKeySlotId = 60;
    }
    else
    {
        throw std::runtime_error("Invalid participant name.");
    }

    return nodeParams;
}

/**************************************************************************************************
 * Main Function
 **************************************************************************************************/

int main(int argc, char** argv)
{
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

    if (argc < 3)
    {
        std::cerr << "Missing arguments! Start demo with: " << argv[0]
                  << " <ParticipantConfiguration.yaml|json> <ParticipantName> [RegistryUri]" << std::endl
                  << "Use \"Node0\" or \"Node1\" as <ParticipantName>." << std::endl;
        return -1;
    }

    try
    {
        std::string participantConfigurationFilename(argv[1]);
        std::string participantName(argv[2]);

        auto registryUri = "silkit://localhost:8500";
        if (argc >= 4)
        {
            registryUri = argv[3];
        }   

        auto participantConfiguration = SilKit::Config::ParticipantConfigurationFromFile(participantConfigurationFilename);

        std::cout << "Creating participant '" << participantName << "' with registry " << registryUri << std::endl;
        auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryUri);
        auto* controller = participant->CreateFlexrayController("FlexRay1", "PowerTrain1");
        auto* lifecycleService =
            participant->CreateLifecycleService({OperationMode::Coordinated});
        auto* timeSyncService = lifecycleService->CreateTimeSyncService();

        std::vector<FlexrayTxBufferConfig> bufferConfigs;

        if (participantName == "Node0")
        {
            // initialize bufferConfig to send some FrMessages
            FlexrayTxBufferConfig cfg;
            cfg.channels = FlexrayChannel::AB;
            cfg.slotId = 40;
            cfg.offset = 0;
            cfg.repetition = 1;
            cfg.hasPayloadPreambleIndicator = false;
            cfg.headerCrc = 5;
            cfg.transmissionMode = FlexrayTransmissionMode::SingleShot;
            bufferConfigs.push_back(cfg);

            cfg.channels = FlexrayChannel::A;
            cfg.slotId = 41;
            bufferConfigs.push_back(cfg);

            cfg.channels = FlexrayChannel::B;
            cfg.slotId = 42;
            bufferConfigs.push_back(cfg);
        }
        else if (participantName == "Node1")
        {
            // initialize bufferConfig to send some FrMessages
            FlexrayTxBufferConfig cfg;
            cfg.channels = FlexrayChannel::AB;
            cfg.slotId = 60;
            cfg.offset = 0;
            cfg.repetition = 1;
            cfg.hasPayloadPreambleIndicator = false;
            cfg.headerCrc = 5;
            cfg.transmissionMode = FlexrayTransmissionMode::SingleShot;
            bufferConfigs.push_back(cfg);

            cfg.channels = FlexrayChannel::A;
            cfg.slotId = 61;
            bufferConfigs.push_back(cfg);

            cfg.channels = FlexrayChannel::B;
            cfg.slotId = 62;
            bufferConfigs.push_back(cfg);
        }

        FlexrayControllerConfig config;
        config.bufferConfigs = bufferConfigs;

        config.clusterParams = clusterParams;
        
        config.nodeParams = MakeNodeParams(participantName);

        FlexrayNode frNode(controller, std::move(config));
        if (participantName == "Node0")
        {
            frNode.busState = FlexrayNode::MasterState::PerformWakeup;
        }
        else if (participantName == "Node1")
        {
            frNode.busState = FlexrayNode::MasterState::PerformWakeup;
            frNode.SetStartupDelay(0ms);
        }
        else
        {
            std::cout << "Wrong participant name provided. Use either \"Node0\" or \"Node1\"."
                      << std::endl;
            return 1;
        }

        controller->AddPocStatusHandler([&frNode](IFlexrayController* frController,
                                                  const FlexrayPocStatusEvent& pocStatusEvent) {
            frNode.PocStatusHandler(frController, pocStatusEvent);
        });
        controller->AddFrameHandler(&ReceiveMessage<FlexrayFrameEvent>);
        controller->AddFrameTransmitHandler(&ReceiveMessage<FlexrayFrameTransmitEvent>);
        controller->AddWakeupHandler([&frNode](IFlexrayController* frController,
                                               const FlexrayWakeupEvent& wakeupEvent) {
                frNode.WakeupHandler(frController, wakeupEvent);
        });
        controller->AddSymbolHandler(&ReceiveMessage<FlexraySymbolEvent>);
        controller->AddSymbolTransmitHandler(&ReceiveMessage<FlexraySymbolTransmitEvent>);
        controller->AddCycleStartHandler(&ReceiveMessage<FlexrayCycleStartEvent>);

        timeSyncService->SetSimulationStepHandler(
            [&frNode](std::chrono::nanoseconds now, std::chrono::nanoseconds /*duration*/) {
                
                auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now);
                std::cout << "now=" << nowMs.count() << "ms" << std::endl;
                frNode.doAction(now);
                std::this_thread::sleep_for(500ms);
                
        }, 1ms);

        auto finalStateFuture = lifecycleService->StartLifecycle();
        auto finalState = finalStateFuture.get();

        std::cout << "Simulation stopped. Final State: " << finalState << std::endl;
        std::cout << "Press enter to end the process..." << std::endl;
        std::cin.ignore();
    }
    catch (const SilKit::ConfigurationError& error)
    {
        std::cerr << "Invalid configuration: " << error.what() << std::endl;
        std::cout << "Press enter to end the process..." << std::endl;
        std::cin.ignore();
        return -2;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        std::cout << "Press enter to end the process..." << std::endl;
        std::cin.ignore();
        return -3;
    }

    return 0;
}
