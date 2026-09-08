// SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>
#include <string>
#include <vector>

#include "silkit/services/flexray/all.hpp"
#include "silkit/services/flexray/string_utils.hpp"
#include "silkit/services/logging/ILogger.hpp"

using namespace SilKit::Services::Flexray;
using namespace std::chrono_literals;

// This is the common behavior used in FlexrayNode0 and FlexrayNode1
namespace FlexrayDemoCommon {

// Deliberately malformed FlexRay frames, used by FlexrayNode0Demo --invalid to exercise a receiver's frame
// validation. Each case violates exactly one check of CANoe's GetFlexRayFrameInvalidReason /
// GetFlexRayMessageInvalidReason (projects_source/CANoe/Source/RTEVENT/SilKit/SilKitFr.cpp) and makes it report write
// message 83-0505 with the quoted reason in its Details, on both the frame and the frame transmit route.
//
// A FlexRay frame is not handed over field by field like a CAN frame: the header a receiver sees is derived by SIL Kit
// from the TX buffer configuration and the running cycle. So a case either changes the buffer config
// (ApplyToBufferConfig) or the payload of the buffer update (BuildPayload), and SIL Kit may well reject or normalise
// the value before it reaches the receiver - whether a case arrives at all is part of what it tests.
//
// Not reachable from a sender, and needing a receiver-side unit test instead: the four null-pointer reasons, plus
// "payloadlength exceeds the maximum of 127 words" (UpdateTxBuffer accepts at most 254 payload bytes, which is
// exactly 127 words) and "cyclecount exceeds the maximum of 63" (the cycle counter comes from SIL Kit).
namespace InvalidFrames {

struct Case
{
    const char* name;
    const char* expectedReason;
    //! Applied to the last TX buffer config - never the key slot buffer, which FlexRay startup depends on.
    std::function<void(FlexrayTxBufferConfig&)> ApplyToBufferConfig;
    //! Replaces the payload of every buffer update. Empty means "leave the normal payload alone".
    std::function<void(std::vector<uint8_t>&)> BuildPayload;
};

inline auto All() -> const std::vector<Case>&
{
    static const std::vector<Case> cases{
        {"slotid-zero", "frameid is outside the valid slot id range of 1...2047",
         [](FlexrayTxBufferConfig& cfg) { cfg.slotId = 0; }, {}},
        {"slotid-max", "frameid is outside the valid slot id range of 1...2047",
         [](FlexrayTxBufferConfig& cfg) { cfg.slotId = 2048; }, {}},
        {"headercrc", "headercrc exceeds the 11 bit maximum of 2047",
         [](FlexrayTxBufferConfig& cfg) { cfg.headerCrc = 2048; }, {}},
        // Transmitting on both channels at once. SIL Kit may resolve this into one event per channel, in which case
        // the receiver never sees channel == AB and this case does not fire.
        {"channel", "channel is neither FlexRay channel A nor B",
         [](FlexrayTxBufferConfig& cfg) { cfg.channels = FlexrayChannel::AB; }, {}},
        // An odd payload size cannot be expressed as whole 2 byte words, so payloadLength * 2 never matches it.
        {"payload-size", "payloadsize does not match the two bytes per word derived from payloadlength", {},
         [](std::vector<uint8_t>& payload) { payload.assign(3, 0xAB); }},
    };
    return cases;
}

inline auto Find(const std::string& name) -> const Case*
{
    for (const auto& c : All())
    {
        if (c.name == name)
        {
            return &c;
        }
    }
    return nullptr;
}

inline auto NameList() -> std::string
{
    std::string list;
    for (const auto& c : All())
    {
        list += (list.empty() ? "" : ", ");
        list += c.name;
    }
    return list;
}

} // namespace InvalidFrames

class FlexrayNode
{
public:
    FlexrayNode(IFlexrayController* controller, FlexrayControllerConfig config, ILogger* logger,
                const InvalidFrames::Case* invalidCase = nullptr)
        : _flexrayController{controller}
        , _controllerConfig{std::move(config)}
        , _logger{logger}
        , _invalidCase{invalidCase}
    {
        _lastPocStatus.state = FlexrayPocState::DefaultConfig;
        _busState = FlexrayNode::MasterState::PerformWakeup;

        _flexrayController->AddFrameHandler([this](IFlexrayController* /*ctrl*/, FlexrayFrameEvent ev) {
            std::stringstream ss;
            ss << "Received " << ev;
            _logger->Info(ss.str());
        });

        _flexrayController->AddFrameTransmitHandler(
            [this](IFlexrayController* /*ctrl*/, const FlexrayFrameTransmitEvent& ev) {
            std::stringstream ss;
            ss << "Received " << ev;
            _logger->Info(ss.str());
        });

        _flexrayController->AddSymbolHandler([this](IFlexrayController* /*ctrl*/, FlexraySymbolEvent ev) {
            std::stringstream ss;
            ss << "Received " << ev;
            _logger->Info(ss.str());
        });

        _flexrayController->AddSymbolTransmitHandler(
            [this](IFlexrayController* /*ctrl*/, const FlexraySymbolTransmitEvent& ev) {
            std::stringstream ss;
            ss << "Received " << ev;
            _logger->Info(ss.str());
        });

        _flexrayController->AddCycleStartHandler(
            [this](IFlexrayController* /*ctrl*/, const FlexrayCycleStartEvent& ev) {
            std::stringstream ss;
            ss << "Received " << ev;
            _logger->Info(ss.str());
        });

        _flexrayController->AddWakeupHandler(
            [this](IFlexrayController* /*ctrl*/, const FlexrayWakeupEvent& wakeupEvent) {
            std::stringstream ss;
            ss << "Received WAKEUP! (" << wakeupEvent.pattern << ")";
            _logger->Info(ss.str());

            _flexrayController->AllowColdstart();
            _flexrayController->Run();
        });

        _flexrayController->AddPocStatusHandler(
            [this](IFlexrayController* /*ctrl*/, const FlexrayPocStatusEvent& pocStatusEvent) {
            {
                std::stringstream ss;
                ss << "Received POC=" << pocStatusEvent.state << ", Freeze=" << pocStatusEvent.freeze
                   << ", Wakeup=" << pocStatusEvent.wakeupStatus << ", Slot=" << pocStatusEvent.slotMode
                   << " @t=" << pocStatusEvent.timestamp;
                _logger->Info(ss.str());
            }

            if (_lastPocStatus.state == FlexrayPocState::Wakeup && pocStatusEvent.state == FlexrayPocState::Ready)
            {
                std::stringstream ss;
                ss << "   Wakeup finished...";
                _logger->Info(ss.str());
                _busState = MasterState::WakeupDone;
            }

            _lastPocStatus = pocStatusEvent;
        });
    }

private:
    IFlexrayController* _flexrayController{nullptr};
    FlexrayControllerConfig _controllerConfig{};
    FlexrayPocStatusEvent _lastPocStatus{};
    int _msgId = 0;
    bool _configured{false};
    ILogger* _logger;
    const InvalidFrames::Case* _invalidCase{nullptr};

    enum class MasterState
    {
        Ignore,
        PerformWakeup,
        WaitForWakeup,
        WakeupDone
    };
    MasterState _busState = MasterState::Ignore;

private:
    void PocReady()
    {
        switch (_busState)
        {
        case MasterState::PerformWakeup:
            _flexrayController->Wakeup();
            break;
        case MasterState::WaitForWakeup:
            break;
        case MasterState::WakeupDone:
            _flexrayController->AllowColdstart();
            _flexrayController->Run();
            break;
        default:
            break;
        }
    }

    void TxBufferUpdate()
    {
        if (_controllerConfig.bufferConfigs.empty())
            return;

        _msgId++;

        auto bufferIdx = _msgId % _controllerConfig.bufferConfigs.size();

        // prepare a friendly message as payload
        std::stringstream payloadStream;
        payloadStream << "FlexrayFrameEvent#" << std::setw(4) << _msgId << "; bufferId=" << bufferIdx;
        auto payloadString = payloadStream.str();

        std::vector<uint8_t> payloadBytes;
        payloadBytes.resize(payloadString.size());

        std::copy(payloadString.begin(), payloadString.end(), payloadBytes.begin());

        if (_invalidCase != nullptr && _invalidCase->BuildPayload)
        {
            _invalidCase->BuildPayload(payloadBytes);

            std::stringstream ss;
            ss << "Sending malformed FlexRay payload '" << _invalidCase->name
               << "': payloadsize=" << payloadBytes.size() << " - expecting reason '" << _invalidCase->expectedReason
               << "'";
            _logger->Info(ss.str());
        }

        FlexrayTxBufferUpdate update;
        update.payload = payloadBytes;
        update.payloadDataValid = true;
        update.txBufferIndex = static_cast<decltype(update.txBufferIndex)>(bufferIdx);

        _flexrayController->UpdateTxBuffer(update);
    }

    void SwapChannels()
    {
        std::stringstream ss;
        ss << "Reconfiguring TxBuffers: Swapping FlexrayChannel::A and FlexrayChannel::B";
        _logger->Info(ss.str());

        for (uint16_t idx = 0; idx < _controllerConfig.bufferConfigs.size(); idx++)
        {
            auto&& bufferConfig = _controllerConfig.bufferConfigs[idx];
            switch (bufferConfig.channels)
            {
            case FlexrayChannel::A:
                bufferConfig.channels = FlexrayChannel::B;
                _flexrayController->ReconfigureTxBuffer(idx, bufferConfig);
                break;
            case FlexrayChannel::B:
                bufferConfig.channels = FlexrayChannel::A;
                _flexrayController->ReconfigureTxBuffer(idx, bufferConfig);
                break;
            default:
                break;
            }
        }
    }

    void Init()
    {
        if (_configured)
            return;
        _flexrayController->Configure(_controllerConfig);
        _configured = true;
    }

public:
    void DoAction(std::chrono::nanoseconds now)
    {
        switch (_lastPocStatus.state)
        {
        case FlexrayPocState::DefaultConfig:
            Init();
            break;
        case FlexrayPocState::Ready:
            PocReady();
            break;
        case FlexrayPocState::NormalActive:
            if (now == 100ms)
            {
                SwapChannels();
            }
            else
            {
                TxBufferUpdate();
            }
            break;
        case FlexrayPocState::Config:
        case FlexrayPocState::Startup:
        case FlexrayPocState::Wakeup:
        case FlexrayPocState::NormalPassive:
        case FlexrayPocState::Halt:
        default:
            break;
        }
    }
};

inline auto MakeControllerConfig() -> FlexrayControllerConfig
{
    FlexrayClusterParameters clusterParams{};
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

    FlexrayNodeParameters nodeParams{};
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

    FlexrayControllerConfig config{};
    config.clusterParams = clusterParams;
    config.nodeParams = nodeParams;

    return config;
}


} // namespace FlexrayDemoCommon