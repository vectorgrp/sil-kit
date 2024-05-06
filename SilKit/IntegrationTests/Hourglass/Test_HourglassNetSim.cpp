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

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "silkit/capi/SilKit.h"

#include "silkit/SilKit.hpp"
#include "silkit/detail/impl/ThrowOnError.hpp"

#include "MockCapiTest.hpp"

namespace {

using testing::DoAll;
using testing::SetArgPointee;
using testing::Return;

using namespace SilKit::Experimental::NetworkSimulation;

MATCHER_P(ReceiverMatcher, controlReceivers, "")
{
    *result_listener << "Matches SilKit_Experimental_EventReceivers (arg, C-API) to EventReceivers (controlFrame Cpp API)";
    const SilKit::Util::Span<ControllerDescriptor> cppReceivers = controlReceivers;
    const SilKit_Experimental_EventReceivers* cReceivers = arg;
    if (cppReceivers.size() != cReceivers->numReceivers)
    {
        return false;
    }
    for (size_t i = 0; i < cppReceivers.size(); i++)
    {
        if (cppReceivers[i] != cReceivers->controllerDescriptors[i])
        {
            return false;
        }
    }
    return true;
}

// -----------------------------
// Matcher for Can
// -----------------------------

bool CompareCanFrame(const SilKit::Services::Can::CanFrame& cppFrame, const SilKit_CanFrame* cFrame)
{
    using namespace SilKit::Services::Can;

    if (cppFrame.canId != cFrame->id || cppFrame.dlc != cFrame->dlc
        || cppFrame.dataField.size() != cFrame->data.size)
    {
        return false;
    }
    for (size_t i = 0; i < cppFrame.dataField.size(); i++)
    {
        if (cppFrame.dataField[i] != cFrame->data.data[i])
        {
            return false;
        }
    }
    if (cppFrame.sdt != cFrame->sdt || cppFrame.vcid != cFrame->vcid
        || cppFrame.af != cFrame->af)
    {
        return false;
    }
    if (((cppFrame.flags & static_cast<CanFrameFlagMask>(CanFrameFlag::Ide)) != 0)
        != ((cFrame->flags & SilKit_CanFrameFlag_ide) != 0))
    {
        return false;
    }
    if (((cppFrame.flags & static_cast<CanFrameFlagMask>(CanFrameFlag::Fdf)) != 0)
        != ((cFrame->flags & SilKit_CanFrameFlag_fdf) != 0))
    {
        return false;
    }
    if (((cppFrame.flags & static_cast<CanFrameFlagMask>(CanFrameFlag::Brs)) != 0)
        != ((cFrame->flags & SilKit_CanFrameFlag_brs) != 0))
    {
        return false;
    }
    if (((cppFrame.flags & static_cast<CanFrameFlagMask>(CanFrameFlag::Esi)) != 0)
        != ((cFrame->flags & SilKit_CanFrameFlag_esi) != 0))
    {
        return false;
    }
    if (((cppFrame.flags & static_cast<CanFrameFlagMask>(CanFrameFlag::Rtr)) != 0)
        != ((cFrame->flags & SilKit_CanFrameFlag_rtr) != 0))
    {
        return false;
    }
    if (((cppFrame.flags & static_cast<CanFrameFlagMask>(CanFrameFlag::Xlf)) != 0)
        != ((cFrame->flags & SilKit_CanFrameFlag_xlf) != 0))
    {
        return false;
    }
    if (((cppFrame.flags & static_cast<CanFrameFlagMask>(CanFrameFlag::Sec)) != 0)
        != ((cFrame->flags & SilKit_CanFrameFlag_sec) != 0))
    {
        return false;
    }
    return true;
}

MATCHER_P(CanFrameEventMatcher, controlFrame, "")
{
    using namespace SilKit::Services::Can;
    *result_listener << "Matches SilKit_CanFrameEvent (arg, C-API) to CanFrameEvent (controlFrame Cpp API)";
    const CanFrameEvent cppFrameEvent = controlFrame;
    const SilKit_CanFrameEvent* cFrameEvent = (SilKit_CanFrameEvent*)arg;

    if ((SilKit_Direction)cppFrameEvent.direction != cFrameEvent->direction
        || static_cast<uint64_t>(cppFrameEvent.timestamp.count()) != cFrameEvent->timestamp
        || cppFrameEvent.userContext != cFrameEvent->userContext)
    {
        return false;
    }

    return CompareCanFrame(cppFrameEvent.frame, cFrameEvent->frame);
}

MATCHER_P(NetSimCanFrameRequestMatcher, controlMsg, "")
{
    *result_listener << "Matches SilKit_Experimental_NetSim_CanFrameRequest (arg, C-API) to "
                        "SilKit::Experimental::NetworkSimulation::CanFrameRequest (controlMsg Cpp API)";
    SilKit_Experimental_NetSim_CanFrameRequest cFrameRequest = (SilKit_Experimental_NetSim_CanFrameRequest)controlMsg;
    SilKit::Experimental::NetworkSimulation::Can::CanFrameRequest cppFrameRequest =
        (SilKit::Experimental::NetworkSimulation::Can::CanFrameRequest)arg;

    if (cppFrameRequest.userContext != cFrameRequest.userContext)
    {
        return false;
    }

    return CompareCanFrame(cppFrameRequest.frame, cFrameRequest.frame);
}

MATCHER_P(NetSimCanConfigureBaudrateMatcher, controlMsg, "")
{
    *result_listener << "Matches SilKit_Experimental_NetSim_CanConfigureBaudrate (arg, C-API) to "
                        "SilKit::Experimental::NetworkSimulation::CanConfigureBaudrate (controlMsg Cpp API)";
    SilKit_Experimental_NetSim_CanConfigureBaudrate cCanConfigureBaudrate = (SilKit_Experimental_NetSim_CanConfigureBaudrate)controlMsg;
    SilKit::Experimental::NetworkSimulation::Can::CanConfigureBaudrate cppCanConfigureBaudrate =
        (SilKit::Experimental::NetworkSimulation::Can::CanConfigureBaudrate)arg;

    if (cppCanConfigureBaudrate.baudRate != cCanConfigureBaudrate.rate
        || cppCanConfigureBaudrate.fdBaudRate != cCanConfigureBaudrate.fdRate
        || cppCanConfigureBaudrate.xlBaudRate != cCanConfigureBaudrate.xlRate)
    {
        return false;
    }

    return true;
}


MATCHER_P(NetSimCanControllerModeMatcher, controlMsg, "")
{
    *result_listener << "Matches SilKit_Experimental_NetSim_CanControllerMode (arg, C-API) to "
                        "SilKit::Experimental::NetworkSimulation::CanControllerMode (controlMsg Cpp API)";
    SilKit_Experimental_NetSim_CanControllerMode cCanControllerMode = (SilKit_Experimental_NetSim_CanControllerMode)controlMsg;
    SilKit::Experimental::NetworkSimulation::Can::CanControllerMode cppCanControllerMode =
        (SilKit::Experimental::NetworkSimulation::Can::CanControllerMode)arg;

    if (cppCanControllerMode.canControllerModeFlags != cCanControllerMode.canControllerModeFlags
        || (SilKit_CanControllerState)cppCanControllerMode.state != cCanControllerMode.state)
    {
        return false;
    }

    return true;
}

// -----------------------------
// Matcher for Ethernet
// -----------------------------

bool CompareEthernetFrame(const SilKit::Services::Ethernet::EthernetFrame& cppFrame, const SilKit_EthernetFrame* cFrame)
{
    using namespace SilKit::Services::Ethernet;

    if (cppFrame.raw.size() != cFrame->raw.size)
    {
        return false;
    }
    for (size_t i = 0; i < cppFrame.raw.size(); i++)
    {
        if (cppFrame.raw[i] != cFrame->raw.data[i])
        {
            return false;
        }
    }
    return true;
}

MATCHER_P(EthernetFrameEventMatcher, controlFrame, "")
{
    using namespace SilKit::Services::Ethernet;
    *result_listener << "Matches SilKit_EthernetFrameEvent (arg, C-API) to EthernetFrameEvent (controlFrame Cpp API)";
    const EthernetFrameEvent cppFrameEvent = controlFrame;
    const SilKit_EthernetFrameEvent* cFrameEvent = (SilKit_EthernetFrameEvent*)arg;

    if ((SilKit_Direction)cppFrameEvent.direction != cFrameEvent->direction
        || static_cast<uint64_t>(cppFrameEvent.timestamp.count()) != cFrameEvent->timestamp
        || cppFrameEvent.userContext != cFrameEvent->userContext)
    {
        return false;
    }

    return CompareEthernetFrame(cppFrameEvent.frame, cFrameEvent->ethernetFrame);
}

MATCHER_P(NetSimEthernetFrameRequestMatcher, controlMsg, "")
{
    *result_listener << "Matches SilKit_Experimental_NetSim_EthernetFrameRequest (arg, C-API) to "
                        "SilKit::Experimental::NetworkSimulation::EthernetFrameRequest (controlMsg Cpp API)";
    SilKit_Experimental_NetSim_EthernetFrameRequest cFrameRequest = (SilKit_Experimental_NetSim_EthernetFrameRequest)controlMsg;
    SilKit::Experimental::NetworkSimulation::Ethernet::EthernetFrameRequest cppFrameRequest =
        (SilKit::Experimental::NetworkSimulation::Ethernet::EthernetFrameRequest)arg;

    if (cppFrameRequest.userContext != cFrameRequest.userContext)
    {
        return false;
    }

    return CompareEthernetFrame(cppFrameRequest.ethernetFrame, cFrameRequest.ethernetFrame);
}

MATCHER_P(NetSimEthernetControllerModeMatcher, controlMsg, "")
{
    *result_listener << "Matches SilKit_Experimental_NetSim_EthernetControllerMode (arg, C-API) to "
                        "SilKit::Experimental::NetworkSimulation::EthernetControllerMode (controlMsg Cpp API)";
    SilKit_Experimental_NetSim_EthernetControllerMode cControllerMode = (SilKit_Experimental_NetSim_EthernetControllerMode)controlMsg;
    SilKit::Experimental::NetworkSimulation::Ethernet::EthernetControllerMode cppControllerMode =
        (SilKit::Experimental::NetworkSimulation::Ethernet::EthernetControllerMode)arg;

    if ((SilKit_EthernetControllerMode)cppControllerMode.mode != cControllerMode.mode)
    {
        return false;
    }

    return true;
}

// -----------------------------
// Matcher for FlexRay
// -----------------------------

bool CompareFlexrayFrame(const SilKit::Services::Flexray::FlexrayFrame& cppFrame, const SilKit_FlexrayFrame* cFrame)
{
    if (cppFrame.header.cycleCount != cFrame->header->cycleCount
        || cppFrame.header.flags != cFrame->header->flags
        || cppFrame.header.frameId != cFrame->header->frameId
        || cppFrame.header.headerCrc != cFrame->header->headerCrc
        || cppFrame.header.payloadLength != cFrame->header->payloadLength)
    {
        return false;
    }
    if (cppFrame.payload.size() != cFrame->payload.size)
    {
        return false;
    }
    for (size_t i = 0; i < cppFrame.payload.size(); i++)
    {
        if (cppFrame.payload[i] != cFrame->payload.data[i])
        {
            return false;
        }
    }
    return true;
}


bool CompareFlexrayTxBufferConfig(const SilKit::Services::Flexray::FlexrayTxBufferConfig& cppTxBufferConfig,
                                  const SilKit_FlexrayTxBufferConfig* cTxBufferConfig)
{
    if ((SilKit_FlexrayChannel)cppTxBufferConfig.channels != cTxBufferConfig->channels
        || (SilKit_Bool)cppTxBufferConfig.hasPayloadPreambleIndicator != cTxBufferConfig->hasPayloadPreambleIndicator
        || cppTxBufferConfig.headerCrc != cTxBufferConfig->headerCrc
        || cppTxBufferConfig.offset != cTxBufferConfig->offset
        || cppTxBufferConfig.repetition != cTxBufferConfig->repetition
        || cppTxBufferConfig.slotId != cTxBufferConfig->slotId
        || (SilKit_FlexrayTransmissionMode)cppTxBufferConfig.transmissionMode != cTxBufferConfig->transmissionMode)
    {
        return false;
    }
    return true;
}

MATCHER_P(FlexrayFrameEventMatcher, controlFrame, "")
{
    using namespace SilKit::Services::Flexray;
    *result_listener << "Matches SilKit_FlexrayFrameEvent (arg, C-API) to FlexrayFrameEvent (controlFrame Cpp API)";
    const FlexrayFrameEvent cppFrameEvent = controlFrame;
    const SilKit_FlexrayFrameEvent* cFrameEvent = (SilKit_FlexrayFrameEvent*)arg;

    if ((SilKit_FlexrayChannel)cppFrameEvent.channel != cFrameEvent->channel
        || static_cast<uint64_t>(cppFrameEvent.timestamp.count()) != cFrameEvent->timestamp)
    {
        return false;
    }

    return CompareFlexrayFrame(cppFrameEvent.frame, cFrameEvent->frame);
}

MATCHER_P(NetSimFlexrayHostCommandMatcher, controlMsg, "")
{
    *result_listener << "Matches SilKit_Experimental_NetSim_FlexrayHostCommand (arg, C-API) to "
        "SilKit::Experimental::NetworkSimulation::Flexray::FlexrayHostCommand (controlMsg Cpp API)";
    SilKit_Experimental_NetSim_FlexrayHostCommand cHostCommand = (SilKit_Experimental_NetSim_FlexrayHostCommand)controlMsg;
    SilKit::Experimental::NetworkSimulation::Flexray::FlexrayHostCommand cppHostCommand =
        (SilKit::Experimental::NetworkSimulation::Flexray::FlexrayHostCommand)arg;

    if ((uint8_t)cppHostCommand.command != cHostCommand.chiCommand)
    {
        return false;
    }

    return true;
}

MATCHER_P(NetSimFlexrayControllerConfigMatcher, controlMsg, "")
{
    *result_listener << "Matches SilKit_Experimental_NetSim_FlexrayControllerConfig (arg, C-API) to "
        "SilKit::Experimental::NetworkSimulation::Flexray::FlexrayControllerConfig (controlMsg Cpp API)";
    SilKit_Experimental_NetSim_FlexrayControllerConfig cControllerConfig = (SilKit_Experimental_NetSim_FlexrayControllerConfig)controlMsg;
    SilKit::Experimental::NetworkSimulation::Flexray::FlexrayControllerConfig cppControllerConfig =
        (SilKit::Experimental::NetworkSimulation::Flexray::FlexrayControllerConfig)arg;

    if (cppControllerConfig.bufferConfigs.size() != cControllerConfig.numBufferConfigs)
    {
        return false;
    }
    for (size_t i = 0; i < cppControllerConfig.bufferConfigs.size(); i++)
    {
        if (!CompareFlexrayTxBufferConfig(cppControllerConfig.bufferConfigs[i], &cControllerConfig.bufferConfigs[i]))
        {
            return false;
        }
    }

    if (cppControllerConfig.clusterParams.gColdstartAttempts != cControllerConfig.clusterParams->gColdstartAttempts
        || cppControllerConfig.clusterParams.gCycleCountMax != cControllerConfig.clusterParams->gCycleCountMax
        || cppControllerConfig.clusterParams.gdActionPointOffset != cControllerConfig.clusterParams->gdActionPointOffset
        || cppControllerConfig.clusterParams.gdDynamicSlotIdlePhase
               != cControllerConfig.clusterParams->gdDynamicSlotIdlePhase
        || cppControllerConfig.clusterParams.gdMiniSlot != cControllerConfig.clusterParams->gdMiniSlot
        || cppControllerConfig.clusterParams.gdMiniSlotActionPointOffset
               != cControllerConfig.clusterParams->gdMiniSlotActionPointOffset
        || cppControllerConfig.clusterParams.gdStaticSlot != cControllerConfig.clusterParams->gdStaticSlot
        || cppControllerConfig.clusterParams.gdSymbolWindow != cControllerConfig.clusterParams->gdSymbolWindow
        || cppControllerConfig.clusterParams.gdSymbolWindowActionPointOffset
               != cControllerConfig.clusterParams->gdSymbolWindowActionPointOffset
        || cppControllerConfig.clusterParams.gdTSSTransmitter != cControllerConfig.clusterParams->gdTSSTransmitter
        || cppControllerConfig.clusterParams.gdWakeupTxActive != cControllerConfig.clusterParams->gdWakeupTxActive
        || cppControllerConfig.clusterParams.gdWakeupTxIdle != cControllerConfig.clusterParams->gdWakeupTxIdle
        || cppControllerConfig.clusterParams.gListenNoise != cControllerConfig.clusterParams->gListenNoise
        || cppControllerConfig.clusterParams.gMacroPerCycle != cControllerConfig.clusterParams->gMacroPerCycle
        || cppControllerConfig.clusterParams.gMaxWithoutClockCorrectionFatal
               != cControllerConfig.clusterParams->gMaxWithoutClockCorrectionFatal
        || cppControllerConfig.clusterParams.gMaxWithoutClockCorrectionPassive
               != cControllerConfig.clusterParams->gMaxWithoutClockCorrectionPassive
        || cppControllerConfig.clusterParams.gNumberOfMiniSlots != cControllerConfig.clusterParams->gNumberOfMiniSlots
        || cppControllerConfig.clusterParams.gNumberOfStaticSlots
               != cControllerConfig.clusterParams->gNumberOfStaticSlots
        || cppControllerConfig.clusterParams.gPayloadLengthStatic
               != cControllerConfig.clusterParams->gPayloadLengthStatic
        || cppControllerConfig.clusterParams.gSyncFrameIDCountMax
               != cControllerConfig.clusterParams->gSyncFrameIDCountMax)
    {
        return false;
    }

    if (cppControllerConfig.nodeParams.pAllowHaltDueToClock != cControllerConfig.nodeParams->pAllowHaltDueToClock
        || cppControllerConfig.nodeParams.pAllowPassiveToActive != cControllerConfig.nodeParams->pAllowPassiveToActive
        || (SilKit_FlexrayChannel)cppControllerConfig.nodeParams.pChannels != cControllerConfig.nodeParams->pChannels
        || cppControllerConfig.nodeParams.pClusterDriftDamping != cControllerConfig.nodeParams->pClusterDriftDamping
        || cppControllerConfig.nodeParams.pdAcceptedStartupRange != cControllerConfig.nodeParams->pdAcceptedStartupRange
        || cppControllerConfig.nodeParams.pdListenTimeout != cControllerConfig.nodeParams->pdListenTimeout
        || (SilKit_FlexrayClockPeriod)cppControllerConfig.nodeParams.pdMicrotick
               != cControllerConfig.nodeParams->pdMicrotick
        || cppControllerConfig.nodeParams.pKeySlotId != cControllerConfig.nodeParams->pKeySlotId
        || cppControllerConfig.nodeParams.pKeySlotOnlyEnabled != cControllerConfig.nodeParams->pKeySlotOnlyEnabled
        || cppControllerConfig.nodeParams.pKeySlotUsedForStartup != cControllerConfig.nodeParams->pKeySlotUsedForStartup
        || cppControllerConfig.nodeParams.pKeySlotUsedForSync != cControllerConfig.nodeParams->pKeySlotUsedForSync
        || cppControllerConfig.nodeParams.pLatestTx != cControllerConfig.nodeParams->pLatestTx
        || cppControllerConfig.nodeParams.pMacroInitialOffsetA != cControllerConfig.nodeParams->pMacroInitialOffsetA
        || cppControllerConfig.nodeParams.pMacroInitialOffsetB != cControllerConfig.nodeParams->pMacroInitialOffsetB
        || cppControllerConfig.nodeParams.pMicroInitialOffsetA != cControllerConfig.nodeParams->pMicroInitialOffsetA
        || cppControllerConfig.nodeParams.pMicroInitialOffsetB != cControllerConfig.nodeParams->pMicroInitialOffsetB
        || cppControllerConfig.nodeParams.pMicroPerCycle != cControllerConfig.nodeParams->pMicroPerCycle
        || cppControllerConfig.nodeParams.pOffsetCorrectionOut != cControllerConfig.nodeParams->pOffsetCorrectionOut
        || cppControllerConfig.nodeParams.pOffsetCorrectionStart != cControllerConfig.nodeParams->pOffsetCorrectionStart
        || cppControllerConfig.nodeParams.pRateCorrectionOut != cControllerConfig.nodeParams->pRateCorrectionOut
        || cppControllerConfig.nodeParams.pSamplesPerMicrotick != cControllerConfig.nodeParams->pSamplesPerMicrotick
        || (SilKit_FlexrayChannel)cppControllerConfig.nodeParams.pWakeupChannel
               != cControllerConfig.nodeParams->pWakeupChannel
        || cppControllerConfig.nodeParams.pWakeupPattern != cControllerConfig.nodeParams->pWakeupPattern)
    {
        return false;
    }

    return true;
}

MATCHER_P(NetSimFlexrayTxBufferConfigUpdateMatcher, controlMsg, "")
{
    *result_listener << "Matches SilKit_Experimental_NetSim_FlexrayTxBufferConfigUpdate (arg, C-API) to "
                        "SilKit::Experimental::NetworkSimulation::Flexray::FlexrayTxBufferConfigUpdate (controlMsg Cpp API)";
    SilKit_Experimental_NetSim_FlexrayTxBufferConfigUpdate cTxBufferConfigUpdate =
        (SilKit_Experimental_NetSim_FlexrayTxBufferConfigUpdate)controlMsg;
    SilKit::Experimental::NetworkSimulation::Flexray::FlexrayTxBufferConfigUpdate cppTxBufferConfigUpdate =
        (SilKit::Experimental::NetworkSimulation::Flexray::FlexrayTxBufferConfigUpdate)arg;

    if (cppTxBufferConfigUpdate.txBufferIndex != cTxBufferConfigUpdate.txBufferIdx)
    {
        return false;
    }
    
    return CompareFlexrayTxBufferConfig(cppTxBufferConfigUpdate.txBufferConfig, cTxBufferConfigUpdate.txBufferConfig);
}

MATCHER_P(NetSimFlexrayTxBufferUpdateMatcher, controlMsg, "")
{
    *result_listener << "Matches SilKit_Experimental_NetSim_FlexrayTxBufferUpdate (arg, C-API) to "
                        "SilKit::Experimental::NetworkSimulation::Flexray::FlexrayTxBufferUpdate (controlMsg Cpp API)";
    SilKit_Experimental_NetSim_FlexrayTxBufferUpdate cTxBufferUpdate = (SilKit_Experimental_NetSim_FlexrayTxBufferUpdate)controlMsg;
    SilKit::Experimental::NetworkSimulation::Flexray::FlexrayTxBufferUpdate cppTxBufferUpdate =
        (SilKit::Experimental::NetworkSimulation::Flexray::FlexrayTxBufferUpdate)arg;

    if ((SilKit_Bool)cppTxBufferUpdate.payloadDataValid != cTxBufferUpdate.payloadDataValid)
    {
        return false;
    }

    if (cppTxBufferUpdate.payload.size() != cTxBufferUpdate.payload.size)
    {
        return false;
    }
    for (size_t i = 0; i < cppTxBufferUpdate.payload.size(); i++)
    {
        if (cppTxBufferUpdate.payload[i] != cTxBufferUpdate.payload.data[i])
        {
            return false;
        }
    }
    return true;
}

// -----------------------------
// Matcher for Lin
// -----------------------------

bool CompareLinFrame(const SilKit::Services::Lin::LinFrame& cppFrame, const SilKit_LinFrame* cFrame)
{
    using namespace SilKit::Services::Lin;
    if ((SilKit_LinChecksumModel)cppFrame.checksumModel != cFrame->checksumModel
        || cppFrame.dataLength != cFrame->dataLength || cppFrame.id != cFrame->id)
    {
        return false;
    }
    for (size_t i = 0; i < cppFrame.data.size(); i++)
    {
        if (cppFrame.data[i] != cFrame->data[i])
        {
            return false;
        }
    }
    return true;
}

MATCHER_P(LinFrameStatusEventMatcher, controlFrame, "")
{
    using namespace SilKit::Services::Lin;
    *result_listener << "Matches SilKit_LinFrameStatusEvent (arg, C-API) to LinFrameStatusEvent (controlFrame Cpp API)";
    const LinFrameStatusEvent cppFrameEvent = controlFrame;
    const SilKit_LinFrameStatusEvent* cFrameEvent = (SilKit_LinFrameStatusEvent*)arg;

    if ((SilKit_LinFrameStatus)cppFrameEvent.status != cFrameEvent->status
        || static_cast<uint64_t>(cppFrameEvent.timestamp.count()) != cFrameEvent->timestamp)
    {
        return false;
    }
    
    return CompareLinFrame(cppFrameEvent.frame, cFrameEvent->frame);
}


MATCHER_P(NetSimLinFrameRequestMatcher, controlMsg, "")
{
    *result_listener << "Matches SilKit_Experimental_NetSim_LinFrameRequest (arg, C-API) to "
                        "SilKit::Experimental::NetworkSimulation::LinFrameRequest (controlMsg Cpp API)";
    SilKit_Experimental_NetSim_LinFrameRequest cFrameRequest = (SilKit_Experimental_NetSim_LinFrameRequest)controlMsg;
    SilKit::Experimental::NetworkSimulation::Lin::LinFrameRequest cppFrameRequest =
        (SilKit::Experimental::NetworkSimulation::Lin::LinFrameRequest)arg;

    if ((SilKit_LinFrameResponseType)cppFrameRequest.responseType != cFrameRequest.responseType)
    {
        return false;
    }

    return CompareLinFrame(cppFrameRequest.frame, cFrameRequest.frame);
}


MATCHER_P(NetSimLinFrameHeaderRequestMatcher, controlMsg, "")
{
    *result_listener << "Matches SilKit_Experimental_NetSim_LinFrameHeaderRequest (arg, C-API) to "
                        "SilKit::Experimental::NetworkSimulation::LinFrameHeaderRequest (controlMsg Cpp API)";
    SilKit_Experimental_NetSim_LinFrameHeaderRequest cFrameHeaderRequest = (SilKit_Experimental_NetSim_LinFrameHeaderRequest)controlMsg;
    SilKit::Experimental::NetworkSimulation::Lin::LinFrameHeaderRequest cppFrameHeaderRequest =
        (SilKit::Experimental::NetworkSimulation::Lin::LinFrameHeaderRequest)arg;

    if (cppFrameHeaderRequest.id != cFrameHeaderRequest.id)
    {
        return false;
    }

    return true;
}

MATCHER_P(NetSimLinWakeupPulseMatcher, controlMsg, "")
{
    *result_listener << "Matches SilKit_Experimental_NetSim_LinWakeupPulse (arg, C-API) to "
                        "SilKit::Experimental::NetworkSimulation::LinWakeupPulse (controlMsg Cpp API)";
    SilKit_Experimental_NetSim_LinWakeupPulse cWakeupPulse = (SilKit_Experimental_NetSim_LinWakeupPulse)controlMsg;
    SilKit::Experimental::NetworkSimulation::Lin::LinWakeupPulse cppWakeupPulse =
        (SilKit::Experimental::NetworkSimulation::Lin::LinWakeupPulse)arg;

    if (static_cast<uint64_t>(cppWakeupPulse.timestamp.count()) != cWakeupPulse.timestamp)
    {
        return false;
    }

    return true;
}

MATCHER_P(NetSimLinControllerConfigMatcher, controlMsg, "")
{
    *result_listener << "Matches SilKit_Experimental_NetSim_LinControllerConfig (arg, C-API) to "
                        "SilKit::Experimental::NetworkSimulation::LinControllerConfig (controlMsg Cpp API)";
    SilKit_Experimental_NetSim_LinControllerConfig cControllerConfig = (SilKit_Experimental_NetSim_LinControllerConfig)controlMsg;
    SilKit::Experimental::NetworkSimulation::Lin::LinControllerConfig cppControllerConfig =
        (SilKit::Experimental::NetworkSimulation::Lin::LinControllerConfig)arg;

    if (cppControllerConfig.baudRate != cControllerConfig.baudRate
        || (SilKit_LinControllerMode)cppControllerConfig.controllerMode != cControllerConfig.controllerMode
        || (uint8_t)cppControllerConfig.simulationMode != cControllerConfig.simulationMode)
    {
        return false;
    }

    if (cppControllerConfig.frameResponses.size() != cControllerConfig.numFrameResponses)
    {
        return false;
    }

    for (size_t i = 0; i < cppControllerConfig.frameResponses.size(); i++)
    {
        if ((SilKit_LinFrameResponseMode)cppControllerConfig.frameResponses[i].responseMode != cControllerConfig.frameResponses[i].responseMode)
        {
            return false;
        }
        if (!CompareLinFrame(cppControllerConfig.frameResponses[i].frame, cControllerConfig.frameResponses[i].frame))
        {
            return false;
        }
    }

    return true;
}


MATCHER_P(NetSimLinFrameResponseUpdateMatcher, controlMsg, "")
{
    *result_listener << "Matches SilKit_Experimental_NetSim_LinFrameResponseUpdate (arg, C-API) to "
                        "SilKit::Experimental::NetworkSimulation::LinFrameResponseUpdate (controlMsg Cpp API)";
    SilKit_Experimental_NetSim_LinFrameResponseUpdate cFrameResponseUpdate = (SilKit_Experimental_NetSim_LinFrameResponseUpdate)controlMsg;
    SilKit::Experimental::NetworkSimulation::Lin::LinFrameResponseUpdate cppFrameResponseUpdate =
        (SilKit::Experimental::NetworkSimulation::Lin::LinFrameResponseUpdate)arg;
    
    for (size_t i = 0; i < cppFrameResponseUpdate.frameResponses.size(); i++)
    {
        if ((SilKit_LinFrameResponseMode)cppFrameResponseUpdate.frameResponses[i].responseMode
            != cFrameResponseUpdate.frameResponses[i].responseMode)
        {
            return false;
        }
        if (!CompareLinFrame(cppFrameResponseUpdate.frameResponses[i].frame,
                             cFrameResponseUpdate.frameResponses[i].frame))
        {
            return false;
        }
    }

    return true;
}


MATCHER_P(NetSimLinControllerStatusUpdateMatcher, controlMsg, "")
{
    *result_listener << "Matches SilKit_Experimental_NetSim_LinControllerStatusUpdate (arg, C-API) to "
                        "SilKit::Experimental::NetworkSimulation::LinControllerStatusUpdate (controlMsg Cpp API)";
    SilKit_Experimental_NetSim_LinControllerStatusUpdate cControllerStatusUpdate =
        (SilKit_Experimental_NetSim_LinControllerStatusUpdate)controlMsg;
    SilKit::Experimental::NetworkSimulation::Lin::LinControllerStatusUpdate cppControllerStatusUpdate =
        (SilKit::Experimental::NetworkSimulation::Lin::LinControllerStatusUpdate)arg;

    if (static_cast<uint64_t>(cppControllerStatusUpdate.timestamp.count()) != cControllerStatusUpdate.timestamp)
    {
        return false;
    }

    if ((SilKit_LinControllerStatus)cppControllerStatusUpdate.status != cControllerStatusUpdate.status)
    {
        return false;
    }

    return true;
}

// -----------------------

class Test_HourglassNetSim : public SilKitHourglassTests::MockCapiTest
{
public:
    SilKit_Experimental_NetworkSimulator* mockNetSim{reinterpret_cast<SilKit_Experimental_NetworkSimulator*>(uintptr_t(0x12345678))};
    SilKit_Experimental_CanEventProducer* mockCanEventProducer{reinterpret_cast<SilKit_Experimental_CanEventProducer*>(uintptr_t(0x12345678))};

    Test_HourglassNetSim()
    {
        using testing::_;
        ON_CALL(capi, SilKit_Experimental_NetworkSimulator_Create(_, _))
            .WillByDefault(DoAll(SetArgPointee<0>(mockNetSim), Return(SilKit_ReturnCode_SUCCESS)));
    }
};

TEST_F(Test_HourglassNetSim, SilKit_Experimental_NetworkSimulator_Create)
{
    SilKit_Participant* participant{(SilKit_Participant*)123456};
    EXPECT_CALL(capi, SilKit_Experimental_NetworkSimulator_Create(testing::_, participant)).Times(1);
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME:: Impl::Experimental::NetworkSimulation::NetworkSimulator netSim(participant);
}

TEST_F(Test_HourglassNetSim, SilKit_Experimental_NetworkSimulator_Start)
{
    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME:: Impl::Experimental::NetworkSimulation::NetworkSimulator netSim(nullptr);
    EXPECT_CALL(capi, SilKit_Experimental_NetworkSimulator_Start(mockNetSim)).Times(1);
    netSim.Start();
}

class MockSimulatedCanController : public SilKit::Experimental::NetworkSimulation::Can::ISimulatedCanController
{
public:

    MOCK_METHOD(void, OnSetBaudrate, (const SilKit::Experimental::NetworkSimulation::Can::CanConfigureBaudrate& msg), (override));
    MOCK_METHOD(void, OnFrameRequest, (const SilKit::Experimental::NetworkSimulation::Can::CanFrameRequest& msg), (override));
    MOCK_METHOD(void, OnSetControllerMode, (const SilKit::Experimental::NetworkSimulation::Can::CanControllerMode& msg),
                (override));
};

class MockSimulatedEthernetController : public SilKit::Experimental::NetworkSimulation::Ethernet::ISimulatedEthernetController
{
public:
    MOCK_METHOD(void, OnFrameRequest, (const SilKit::Experimental::NetworkSimulation::Ethernet::EthernetFrameRequest& msg),
                (override));
    MOCK_METHOD(void, OnSetControllerMode, (const SilKit::Experimental::NetworkSimulation::Ethernet::EthernetControllerMode& msg),
                (override));
};

class MockSimulatedLinController : public SilKit::Experimental::NetworkSimulation::Lin::ISimulatedLinController
{
public:
    MOCK_METHOD(void, OnFrameRequest, (const SilKit::Experimental::NetworkSimulation::Lin::LinFrameRequest& msg), (override));
    MOCK_METHOD(void, OnFrameHeaderRequest, (const SilKit::Experimental::NetworkSimulation::Lin::LinFrameHeaderRequest& msg),
                (override));
    MOCK_METHOD(void, OnWakeupPulse, (const SilKit::Experimental::NetworkSimulation::Lin::LinWakeupPulse& msg), (override));
    MOCK_METHOD(void, OnControllerConfig, (const SilKit::Experimental::NetworkSimulation::Lin::LinControllerConfig& msg), (override));
    MOCK_METHOD(void, OnFrameResponseUpdate, (const SilKit::Experimental::NetworkSimulation::Lin::LinFrameResponseUpdate& msg),
                (override));
    MOCK_METHOD(void, OnControllerStatusUpdate, (const SilKit::Experimental::NetworkSimulation::Lin::LinControllerStatusUpdate& msg),
                (override));
};

class MockSimulatedFlexRayController : public SilKit::Experimental::NetworkSimulation::Flexray::ISimulatedFlexRayController
{
public:
    MOCK_METHOD(void, OnHostCommand,(const SilKit::Experimental::NetworkSimulation::Flexray::FlexrayHostCommand& msg), (override));
    MOCK_METHOD(void, OnControllerConfig,(const SilKit::Experimental::NetworkSimulation::Flexray::FlexrayControllerConfig& msg),
                (override));
    MOCK_METHOD(void,
                OnTxBufferConfigUpdate,(const SilKit::Experimental::NetworkSimulation::Flexray::FlexrayTxBufferConfigUpdate& msg),
                (override));
    MOCK_METHOD(void, OnTxBufferUpdate,(const SilKit::Experimental::NetworkSimulation::Flexray::FlexrayTxBufferUpdate& msg),
                (override));
};


template<typename ControllerType>
class TestSimulatedNetwork : public SilKit::Experimental::NetworkSimulation::ISimulatedNetwork
{
public:
    ControllerType testSimulatedController;

    auto ProvideSimulatedController(SilKit::Experimental::NetworkSimulation::ControllerDescriptor /*controllerDescriptor*/)
        -> SilKit::Experimental::NetworkSimulation::ISimulatedController* override
    {
        return &testSimulatedController;
    }

    void SimulatedControllerRemoved(SilKit::Experimental::NetworkSimulation::ControllerDescriptor /*controllerDescriptor*/) override 
    {
    }

    void SetEventProducer(std::unique_ptr<SilKit::Experimental::NetworkSimulation::IEventProducer> /*eventProducer*/) override 
    {
    }
};

// --------------------------
// Test reception callbacks
// --------------------------

TEST_F(Test_HourglassNetSim, SilKit_Experimental_SimulatedCanControllerFunctions) 
{
    // This implements the cpp interfaces and provides a simulated can controller
    TestSimulatedNetwork<MockSimulatedCanController> testSimulatedNetwork;

    // The C-API call populates the functions
    void* simulatedController{nullptr};
    const void* simulatedControllerFunctions{nullptr};
    SilKit_Experimental_ControllerDescriptor controllerDescriptor{34};
    auto simulatedNetworkFunctions =
        SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME:: Impl::Experimental::NetworkSimulation::MakeSimulatedNetworkFunctions(SimulatedNetworkType::CAN);
    simulatedNetworkFunctions.ProvideSimulatedController(&simulatedController, &simulatedControllerFunctions,
                                                         controllerDescriptor, (void*)(&testSimulatedNetwork));

    // We should get the simulated controller we provided
    EXPECT_EQ(simulatedController, &testSimulatedNetwork.testSimulatedController);
    auto functions = (SilKit_Experimental_SimulatedCanControllerFunctions*)simulatedControllerFunctions;

    // ----------------------------
    // OnFrameRequest
    // --------------------------

    SilKit_CanFrame cFrame;
    SilKit_Struct_Init(SilKit_CanFrame, cFrame);
    cFrame.af = 5;
    auto payload = std::vector<uint8_t>{1, 2, 3, 4};
    cFrame.data = SilKit::Util::ToSilKitByteVector(payload);
    cFrame.dlc = static_cast<uint16_t>(payload.size());
    cFrame.flags |= SilKit_CanFrameFlag_fdf | SilKit_CanFrameFlag_brs;
    cFrame.id = 3;
    cFrame.sdt = 4;
    cFrame.vcid = 6;

    SilKit_Experimental_NetSim_CanFrameRequest cCanFrameRequest;
    SilKit_Struct_Init(SilKit_Experimental_NetSim_CanFrameRequest, cCanFrameRequest);
    cCanFrameRequest.userContext = reinterpret_cast<void*>(0x13356489);
    cCanFrameRequest.frame = &cFrame;

    EXPECT_CALL(testSimulatedNetwork.testSimulatedController,
                OnFrameRequest(NetSimCanFrameRequestMatcher(cCanFrameRequest)))
        .Times(1);
    functions->OnFrameRequest(simulatedController, &cCanFrameRequest);
    
    // ----------------------------
    // OnSetBaudrate
    // --------------------------

    SilKit_Experimental_NetSim_CanConfigureBaudrate cCanConfigureBaudrate;
    SilKit_Struct_Init(SilKit_Experimental_NetSim_CanConfigureBaudrate, cCanConfigureBaudrate);
    cCanConfigureBaudrate.fdRate = 42;
    cCanConfigureBaudrate.rate = 43;
    cCanConfigureBaudrate.xlRate = 44;

    EXPECT_CALL(testSimulatedNetwork.testSimulatedController,
                OnSetBaudrate(NetSimCanConfigureBaudrateMatcher(cCanConfigureBaudrate)))
        .Times(1);
    functions->OnSetBaudrate(simulatedController, &cCanConfigureBaudrate);

    // ----------------------------
    // OnSetControllerMode
    // --------------------------

    SilKit_Experimental_NetSim_CanControllerMode cCanControllerMode;
    SilKit_Struct_Init(SilKit_Experimental_NetSim_CanControllerMode, cCanControllerMode);
    cCanControllerMode.canControllerModeFlags = SilKit_Experimental_NetSim_CanControllerModeFlags_CancelTransmitRequests
                               | SilKit_Experimental_NetSim_CanControllerModeFlags_ResetErrorHandling;
    cCanControllerMode.state = SilKit_CanControllerState_Started;

    EXPECT_CALL(testSimulatedNetwork.testSimulatedController,
                OnSetControllerMode(NetSimCanControllerModeMatcher(cCanControllerMode)))
        .Times(1);
    functions->OnSetControllerMode(simulatedController, &cCanControllerMode);
}


TEST_F(Test_HourglassNetSim, SilKit_Experimental_SimulatedEthernetControllerFunctions)
{
    // This implements the cpp interfaces and provides a simulated can controller
    TestSimulatedNetwork<MockSimulatedEthernetController> testSimulatedNetwork;

    // The C-API call populates the functions
    void* simulatedController{nullptr};
    const void* simulatedControllerFunctions{nullptr};
    SilKit_Experimental_ControllerDescriptor controllerDescriptor{34};
    auto simulatedNetworkFunctions =
        SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME:: Impl::Experimental::NetworkSimulation::MakeSimulatedNetworkFunctions(
            SimulatedNetworkType::Ethernet);
    simulatedNetworkFunctions.ProvideSimulatedController(&simulatedController, &simulatedControllerFunctions,
                                                         controllerDescriptor, (void*)(&testSimulatedNetwork));

    // We should get the simulated controller we provided
    EXPECT_EQ(simulatedController, &testSimulatedNetwork.testSimulatedController);
    auto functions = (SilKit_Experimental_SimulatedEthernetControllerFunctions*)simulatedControllerFunctions;

    // ----------------------------
    // OnFrameRequest
    // --------------------------

    SilKit_EthernetFrame cFrame;
    SilKit_Struct_Init(SilKit_EthernetFrame, cFrame);
    std::array<uint8_t, 10> payload{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    cFrame.raw = {payload.data(), payload.size()};

    SilKit_Experimental_NetSim_EthernetFrameRequest cEthernetFrameRequest;
    SilKit_Struct_Init(SilKit_Experimental_NetSim_EthernetFrameRequest, cEthernetFrameRequest);
    cEthernetFrameRequest.userContext = reinterpret_cast<void*>(0x13356489);
    cEthernetFrameRequest.ethernetFrame = &cFrame;

    EXPECT_CALL(testSimulatedNetwork.testSimulatedController,
                OnFrameRequest(NetSimEthernetFrameRequestMatcher(cEthernetFrameRequest)))
        .Times(1);
    functions->OnFrameRequest(simulatedController, &cEthernetFrameRequest);

    // ----------------------------
    // OnSetControllerMode

    SilKit_Experimental_NetSim_EthernetControllerMode cEthernetControllerMode;
    SilKit_Struct_Init(SilKit_Experimental_NetSim_EthernetControllerMode, cEthernetControllerMode);
    cEthernetControllerMode.mode = SilKit_EthernetControllerMode_Active;

    EXPECT_CALL(testSimulatedNetwork.testSimulatedController,
                OnSetControllerMode(NetSimEthernetControllerModeMatcher(cEthernetControllerMode)))
        .Times(1);
    functions->OnSetControllerMode(simulatedController, &cEthernetControllerMode);
}

TEST_F(Test_HourglassNetSim, SilKit_Experimental_SimulatedFlexRayControllerFunctions)
{
    // This implements the cpp interfaces and provides a simulated can controller
    TestSimulatedNetwork<MockSimulatedFlexRayController> testSimulatedNetwork;

    // The C-API call populates the functions
    void* simulatedController{nullptr};
    const void* simulatedControllerFunctions{nullptr};
    SilKit_Experimental_ControllerDescriptor controllerDescriptor{34};
    auto simulatedNetworkFunctions =
        SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME:: Impl::Experimental::NetworkSimulation::MakeSimulatedNetworkFunctions(
            SimulatedNetworkType::FlexRay);
    simulatedNetworkFunctions.ProvideSimulatedController(&simulatedController, &simulatedControllerFunctions,
                                                         controllerDescriptor, (void*)(&testSimulatedNetwork));

    // We should get the simulated controller we provided
    EXPECT_EQ(simulatedController, &testSimulatedNetwork.testSimulatedController);
    auto functions = (SilKit_Experimental_SimulatedFlexRayControllerFunctions*)simulatedControllerFunctions;

    // ----------------------------
    // OnHostCommand
    // ----------------------------

    SilKit_Experimental_NetSim_FlexrayHostCommand cHostCommand;
    SilKit_Struct_Init(SilKit_Experimental_NetSim_FlexrayHostCommand, cHostCommand);
    cHostCommand.chiCommand = SilKit_FlexrayChiCommand_ALL_SLOTS;

    EXPECT_CALL(testSimulatedNetwork.testSimulatedController,
                OnHostCommand(NetSimFlexrayHostCommandMatcher(cHostCommand)))
        .Times(1);
    functions->OnHostCommand(simulatedController, &cHostCommand);

    // ----------------------------
    // OnControllerConfig
    // ----------------------------

    SilKit_FlexrayTxBufferConfig cTxBufferConfig1;
    SilKit_Struct_Init(SilKit_FlexrayTxBufferConfig, cTxBufferConfig1);
    cTxBufferConfig1.channels = SilKit_FlexrayChannel_AB;
    cTxBufferConfig1.hasPayloadPreambleIndicator = SilKit_False;
    cTxBufferConfig1.headerCrc = 23;
    cTxBufferConfig1.offset = 21;
    cTxBufferConfig1.repetition = 2;
    cTxBufferConfig1.slotId = 25;
    cTxBufferConfig1.transmissionMode = SilKit_FlexrayTransmissionMode_Continuous;

    SilKit_FlexrayTxBufferConfig cTxBufferConfig2;
    SilKit_Struct_Init(SilKit_FlexrayTxBufferConfig, cTxBufferConfig2);
    cTxBufferConfig2.channels = SilKit_FlexrayChannel_A;
    cTxBufferConfig2.hasPayloadPreambleIndicator = SilKit_True;
    cTxBufferConfig2.headerCrc = 13;
    cTxBufferConfig2.offset = 51;
    cTxBufferConfig2.repetition = 3;
    cTxBufferConfig2.slotId = 15;
    cTxBufferConfig2.transmissionMode = SilKit_FlexrayTransmissionMode_SingleShot;

    SilKit_FlexrayClusterParameters cClusterParameters;
    SilKit_Struct_Init(SilKit_FlexrayClusterParameters, cClusterParameters);
    cClusterParameters.gColdstartAttempts = 8;
    cClusterParameters.gCycleCountMax = 63;
    cClusterParameters.gdActionPointOffset = 2;
    cClusterParameters.gdDynamicSlotIdlePhase = 1;
    cClusterParameters.gdMiniSlot = 5;
    cClusterParameters.gdMiniSlotActionPointOffset = 2;
    cClusterParameters.gdStaticSlot = 31;
    cClusterParameters.gdSymbolWindow = 0;
    cClusterParameters.gdSymbolWindowActionPointOffset = 1;
    cClusterParameters.gdTSSTransmitter = 9;
    cClusterParameters.gdWakeupTxActive = 60;
    cClusterParameters.gdWakeupTxIdle = 180;
    cClusterParameters.gListenNoise = 2;
    cClusterParameters.gMacroPerCycle = 3636;
    cClusterParameters.gMaxWithoutClockCorrectionFatal = 2;
    cClusterParameters.gMaxWithoutClockCorrectionPassive = 2;
    cClusterParameters.gNumberOfMiniSlots = 291;
    cClusterParameters.gNumberOfStaticSlots = 70;
    cClusterParameters.gPayloadLengthStatic = 13;
    cClusterParameters.gSyncFrameIDCountMax = 15;

    SilKit_FlexrayNodeParameters cNodeParameters;
    SilKit_Struct_Init(SilKit_FlexrayNodeParameters, cNodeParameters);
    cNodeParameters.pAllowHaltDueToClock = 1;
    cNodeParameters.pAllowPassiveToActive = 2;
    cNodeParameters.pChannels = SilKit_FlexrayChannel_AB;
    cNodeParameters.pClusterDriftDamping = 10;
    cNodeParameters.pdAcceptedStartupRange = 2743;
    cNodeParameters.pdListenTimeout = 2567692;
    cNodeParameters.pdMicrotick = SilKit_FlexrayClockPeriod_T25NS;
    cNodeParameters.pKeySlotId = 1023;
    cNodeParameters.pKeySlotOnlyEnabled = 3;
    cNodeParameters.pKeySlotUsedForStartup = 4;
    cNodeParameters.pKeySlotUsedForSync = 5;
    cNodeParameters.pLatestTx = 7988;
    cNodeParameters.pMacroInitialOffsetA = 68;
    cNodeParameters.pMacroInitialOffsetB = 67;
    cNodeParameters.pMicroInitialOffsetA = 239;
    cNodeParameters.pMicroInitialOffsetB = 238;
    cNodeParameters.pMicroPerCycle = 1280000;
    cNodeParameters.pOffsetCorrectionOut = 16082;
    cNodeParameters.pOffsetCorrectionStart = 15999;
    cNodeParameters.pRateCorrectionOut = 3846;
    cNodeParameters.pSamplesPerMicrotick = 6;
    cNodeParameters.pWakeupChannel = SilKit_FlexrayChannel_B;
    cNodeParameters.pWakeupPattern = 7;

    SilKit_Experimental_NetSim_FlexrayControllerConfig cFlexrayControllerConfig;
    SilKit_Struct_Init(SilKit_Experimental_NetSim_FlexrayControllerConfig, cFlexrayControllerConfig);
    std::vector<SilKit_FlexrayTxBufferConfig> bufferConfigs{cTxBufferConfig1, cTxBufferConfig2};
    cFlexrayControllerConfig.bufferConfigs = bufferConfigs.data();
    cFlexrayControllerConfig.numBufferConfigs = 2;
    cFlexrayControllerConfig.clusterParams = &cClusterParameters;
    cFlexrayControllerConfig.nodeParams = &cNodeParameters;

    EXPECT_CALL(testSimulatedNetwork.testSimulatedController,
                OnControllerConfig(NetSimFlexrayControllerConfigMatcher(cFlexrayControllerConfig)))
        .Times(1);
    functions->OnControllerConfig(simulatedController, &cFlexrayControllerConfig);

    // ----------------------------
    // OnTxBufferConfigUpdate
    // ----------------------------

    SilKit_Experimental_NetSim_FlexrayTxBufferConfigUpdate cFlexrayTxBufferConfigUpdate;
    SilKit_Struct_Init(SilKit_Experimental_NetSim_FlexrayTxBufferConfigUpdate, cFlexrayTxBufferConfigUpdate);
    cFlexrayTxBufferConfigUpdate.txBufferIdx = 3;
    cFlexrayTxBufferConfigUpdate.txBufferConfig = &cTxBufferConfig1;

    EXPECT_CALL(testSimulatedNetwork.testSimulatedController,
                OnTxBufferConfigUpdate(NetSimFlexrayTxBufferConfigUpdateMatcher(cFlexrayTxBufferConfigUpdate)))
        .Times(1);
    functions->OnTxBufferConfigUpdate(simulatedController, &cFlexrayTxBufferConfigUpdate);

    // ----------------------------
    // OnTxBufferUpdate
    // ----------------------------
    
    SilKit_Experimental_NetSim_FlexrayTxBufferUpdate cFlexrayTxBufferUpdate;
    SilKit_Struct_Init(SilKit_Experimental_NetSim_FlexrayTxBufferUpdate, cFlexrayTxBufferUpdate);
    std::vector<uint8_t> payload{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    cFlexrayTxBufferUpdate.payload = {payload.data(), payload.size()};
    cFlexrayTxBufferUpdate.payloadDataValid = SilKit_True;
    cFlexrayTxBufferUpdate.txBufferIndex = 12;

    EXPECT_CALL(testSimulatedNetwork.testSimulatedController,
                OnTxBufferUpdate(NetSimFlexrayTxBufferUpdateMatcher(cFlexrayTxBufferUpdate)))
        .Times(1);
    functions->OnTxBufferUpdate(simulatedController, &cFlexrayTxBufferUpdate);

}

TEST_F(Test_HourglassNetSim, SilKit_Experimental_SimulatedLinControllerFunctions)
{
    // This implements the cpp interfaces and provides a simulated can controller
    TestSimulatedNetwork<MockSimulatedLinController> testSimulatedNetwork;

    // The C-API call populates the functions
    void* simulatedController{nullptr};
    const void* simulatedControllerFunctions{nullptr};
    SilKit_Experimental_ControllerDescriptor controllerDescriptor{34};
    auto simulatedNetworkFunctions =
        SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME:: Impl::Experimental::NetworkSimulation::MakeSimulatedNetworkFunctions(
            SimulatedNetworkType::LIN);
    simulatedNetworkFunctions.ProvideSimulatedController(&simulatedController, &simulatedControllerFunctions,
                                                         controllerDescriptor, (void*)(&testSimulatedNetwork));

    // We should get the simulated controller we provided
    EXPECT_EQ(simulatedController, &testSimulatedNetwork.testSimulatedController);
    auto functions = (SilKit_Experimental_SimulatedLinControllerFunctions*)simulatedControllerFunctions;

    // ----------------------------
    // OnFrameRequest
    // ----------------------------

    SilKit_LinFrame cFrame1;
    SilKit_Struct_Init(SilKit_LinFrame, cFrame1);
    cFrame1.checksumModel = SilKit_LinChecksumModel_Classic;
    uint8_t frame1Payload[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    memcpy(cFrame1.data, frame1Payload, sizeof(cFrame1.data));
    cFrame1.dataLength = 6;
    cFrame1.id = 16;

    SilKit_LinFrame cFrame2;
    SilKit_Struct_Init(SilKit_LinFrame, cFrame2);
    cFrame1.checksumModel = SilKit_LinChecksumModel_Enhanced;
    uint8_t frame2Payload[8] = {2, 3, 4, 5, 6, 7, 8, 9};
    memcpy(cFrame2.data, frame2Payload, sizeof(cFrame2.data));
    cFrame2.dataLength = 5;
    cFrame2.id = 17;

    SilKit_Experimental_NetSim_LinFrameRequest cLinFrameRequest;
    SilKit_Struct_Init(SilKit_Experimental_NetSim_LinFrameRequest, cLinFrameRequest);
    cLinFrameRequest.responseType = SilKit_LinFrameResponseType_SlaveResponse;
    cLinFrameRequest.frame = &cFrame1;

    EXPECT_CALL(testSimulatedNetwork.testSimulatedController,
                OnFrameRequest(NetSimLinFrameRequestMatcher(cLinFrameRequest)))
        .Times(1);
    functions->OnFrameRequest(simulatedController, &cLinFrameRequest);

    // ----------------------------
    // OnFrameHeaderRequest
    // ----------------------------

    SilKit_Experimental_NetSim_LinFrameHeaderRequest cLinFrameHeaderRequest;
    SilKit_Struct_Init(SilKit_Experimental_NetSim_LinFrameHeaderRequest, cLinFrameHeaderRequest);
    cLinFrameHeaderRequest.id = 23;

    EXPECT_CALL(testSimulatedNetwork.testSimulatedController,
                OnFrameHeaderRequest(NetSimLinFrameHeaderRequestMatcher(cLinFrameHeaderRequest)))
        .Times(1);
    functions->OnFrameHeaderRequest(simulatedController, &cLinFrameHeaderRequest);

    // ----------------------------
    // OnWakeupPulse
    // ----------------------------

    SilKit_Experimental_NetSim_LinWakeupPulse cLinWakeupPulse;
    SilKit_Struct_Init(SilKit_Experimental_NetSim_LinWakeupPulse, cLinWakeupPulse);
    cLinWakeupPulse.timestamp = 12353543;

        EXPECT_CALL(testSimulatedNetwork.testSimulatedController,
                    OnWakeupPulse(NetSimLinWakeupPulseMatcher(cLinWakeupPulse)))
        .Times(1);
    functions->OnWakeupPulse(simulatedController, &cLinWakeupPulse);

    // ----------------------------
    // OnControllerConfig
    // ----------------------------

    SilKit_Experimental_NetSim_LinControllerConfig cLinControllerConfig;
    SilKit_Struct_Init(SilKit_Experimental_NetSim_LinControllerConfig, cLinControllerConfig);
    cLinControllerConfig.baudRate = 23;
    cLinControllerConfig.controllerMode = SilKit_LinControllerMode_Master;

    SilKit_LinFrameResponse frameResponse1;
    SilKit_Struct_Init(SilKit_LinFrameResponse, frameResponse1);
    frameResponse1.frame = &cFrame1;
    frameResponse1.responseMode = SilKit_LinFrameResponseMode_TxUnconditional;
    SilKit_LinFrameResponse frameResponse2;
    SilKit_Struct_Init(SilKit_LinFrameResponse, frameResponse2);
    frameResponse2.frame = &cFrame2;
    frameResponse2.responseMode = SilKit_LinFrameResponseMode_Rx;
    std::vector<SilKit_LinFrameResponse> frameResponses{frameResponse1, frameResponse2};
    cLinControllerConfig.frameResponses = frameResponses.data();

    cLinControllerConfig.numFrameResponses = frameResponses.size();
    cLinControllerConfig.simulationMode = SilKit_Experimental_NetSim_LinSimulationMode_Dynamic;

        EXPECT_CALL(testSimulatedNetwork.testSimulatedController,
                    OnControllerConfig(NetSimLinControllerConfigMatcher(cLinControllerConfig)))
        .Times(1);
    functions->OnControllerConfig(simulatedController, &cLinControllerConfig);

    // ----------------------------
    // OnFrameResponseUpdate
    // ----------------------------

    SilKit_Experimental_NetSim_LinFrameResponseUpdate cLinFrameResponseUpdate;
    SilKit_Struct_Init(SilKit_Experimental_NetSim_LinFrameResponseUpdate, cLinFrameResponseUpdate);

    cLinFrameResponseUpdate.frameResponses = frameResponses.data();
    cLinFrameResponseUpdate.numFrameResponses = frameResponses.size();

        EXPECT_CALL(testSimulatedNetwork.testSimulatedController,
                    OnFrameResponseUpdate(NetSimLinFrameResponseUpdateMatcher(cLinFrameResponseUpdate)))
        .Times(1);
    functions->OnFrameResponseUpdate(simulatedController, &cLinFrameResponseUpdate);


    // ----------------------------
    // OnControllerStatusUpdate
    // ----------------------------

    SilKit_Experimental_NetSim_LinControllerStatusUpdate cLinControllerStatusUpdate;
    SilKit_Struct_Init(SilKit_Experimental_NetSim_LinControllerStatusUpdate, cLinControllerStatusUpdate);
    cLinControllerStatusUpdate.status = SilKit_LinControllerStatus_Operational;
    cLinControllerStatusUpdate.timestamp = 4328974;

        EXPECT_CALL(testSimulatedNetwork.testSimulatedController,
                    OnControllerStatusUpdate(NetSimLinControllerStatusUpdateMatcher(cLinControllerStatusUpdate)))
        .Times(1);
    functions->OnControllerStatusUpdate(simulatedController, &cLinControllerStatusUpdate);
}

// ----------------------
// Test EventProducer
// ----------------------

TEST_F(Test_HourglassNetSim, SilKit_Experimental_CanEventProducer_Produce)
{
    using namespace SilKit::Services::Can;
    SilKit_Experimental_CanEventProducer* cCanEventProducer{(SilKit_Experimental_CanEventProducer*)123456};
    // C++ event
    CanFrame canFrame{};
    canFrame.af = 21;
    canFrame.canId = 3;
    auto payload = std::vector<uint8_t>{1, 2, 3, 4};
    canFrame.dataField = payload;
    canFrame.dlc = static_cast<uint16_t>(payload.size());
    canFrame.flags |=
        static_cast<CanFrameFlagMask>(CanFrameFlag::Fdf) | static_cast<CanFrameFlagMask>(CanFrameFlag::Brs);
    canFrame.sdt = 4;
    canFrame.vcid = 6;

    CanFrameEvent canFrameEvent;
    canFrameEvent.direction = SilKit::Services::TransmitDirection::TX;
    canFrameEvent.frame = canFrame;
    canFrameEvent.userContext = reinterpret_cast<void*>(0x23456789);
    canFrameEvent.timestamp = std::chrono::nanoseconds{123};

    std::array<ControllerDescriptor, 3> receiverArray{1, 2, 3};
    auto receivers = SilKit::Util::MakeSpan(receiverArray);

    EXPECT_CALL(capi, SilKit_Experimental_CanEventProducer_Produce(cCanEventProducer, CanFrameEventMatcher(canFrameEvent),
                                                      ReceiverMatcher(receivers)))
        .Times(1);

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME:: Impl::Experimental::NetworkSimulation::CanEventProducer canEventProducer(
        cCanEventProducer);
    canEventProducer.Produce(canFrameEvent, receivers);
}


TEST_F(Test_HourglassNetSim, SilKit_Experimental_EthernetEventProducer_Produce)
{
    using namespace SilKit::Services::Ethernet;
    SilKit_Experimental_EthernetEventProducer* cEthernetEventProducer{(SilKit_Experimental_EthernetEventProducer*)123456};
    // C++ event
    EthernetFrame ethernetFrame{};
    std::array<uint8_t, 10> payload{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    ethernetFrame.raw = {payload.data(), payload.size()};

    EthernetFrameEvent ethernetFrameEvent;
    ethernetFrameEvent.direction = SilKit::Services::TransmitDirection::TX;
    ethernetFrameEvent.frame = ethernetFrame;
    ethernetFrameEvent.userContext = reinterpret_cast<void*>(0x23456789);
    ethernetFrameEvent.timestamp = std::chrono::nanoseconds{123};

    std::array<ControllerDescriptor, 3> receiverArray{1, 2, 3};
    auto receivers = SilKit::Util::MakeSpan(receiverArray);

    EXPECT_CALL(capi, SilKit_Experimental_EthernetEventProducer_Produce(cEthernetEventProducer, EthernetFrameEventMatcher(ethernetFrameEvent),
                                                      ReceiverMatcher(receivers)))
        .Times(1);

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME:: Impl::Experimental::NetworkSimulation::EthernetEventProducer ethernetEventProducer(
        cEthernetEventProducer);
    ethernetEventProducer.Produce(ethernetFrameEvent, receivers);
}

TEST_F(Test_HourglassNetSim, SilKit_Experimental_FlexRayEventProducer_Produce)
{
    using namespace SilKit::Services::Flexray;
    SilKit_Experimental_FlexRayEventProducer* cFlexRayEventProducer{(SilKit_Experimental_FlexRayEventProducer*)123456};
    
    // C++ event
    const uint8_t payloadLength = 3;
    FlexrayHeader header;
    header.cycleCount = 5;
    header.flags = 1;
    header.frameId = 2;
    header.headerCrc = 4;
    header.payloadLength = payloadLength;

    FlexrayFrame flexrayFrame{};
    std::array<uint8_t, payloadLength> payload{0, 1, 2};
    flexrayFrame.header = header;
    flexrayFrame.payload = {payload.data(), payload.size()};

    FlexrayFrameEvent flexrayFrameEvent;
    flexrayFrameEvent.channel = FlexrayChannel::AB;
    flexrayFrameEvent.frame = flexrayFrame;
    flexrayFrameEvent.timestamp = std::chrono::nanoseconds{123};

    std::array<ControllerDescriptor, 3> receiverArray{1, 2, 3};
    auto receivers = SilKit::Util::MakeSpan(receiverArray);

    EXPECT_CALL(capi,
                SilKit_Experimental_FlexRayEventProducer_Produce(cFlexRayEventProducer, FlexrayFrameEventMatcher(flexrayFrameEvent),
                                                    ReceiverMatcher(receivers)))
        .Times(1);

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME:: Impl::Experimental::NetworkSimulation::FlexRayEventProducer flexrayEventProducer(
        cFlexRayEventProducer);
    flexrayEventProducer.Produce(flexrayFrameEvent, receivers);
}

TEST_F(Test_HourglassNetSim, SilKit_Experimental_LinEventProducer_Produce)
{
    using namespace SilKit::Services::Lin;
    SilKit_Experimental_LinEventProducer* cLinEventProducer{(SilKit_Experimental_LinEventProducer*)123456};
    // C++ event
    LinFrame linFrame{};
    linFrame.checksumModel = LinChecksumModel::Enhanced;
    linFrame.dataLength = 4;
    linFrame.id = 42;
    auto payload = std::array<uint8_t, 8>{1, 2, 3, 4, 5, 6, 7, 8};
    linFrame.data = payload;

    LinFrameStatusEvent linFrameStatusEvent;
    linFrameStatusEvent.frame = linFrame;
    linFrameStatusEvent.status = LinFrameStatus::LIN_TX_OK;
    linFrameStatusEvent.timestamp = std::chrono::nanoseconds{123};

    std::array<ControllerDescriptor, 3> receiverArray{1, 2, 3};
    auto receivers = SilKit::Util::MakeSpan(receiverArray);

    EXPECT_CALL(capi,
                SilKit_Experimental_LinEventProducer_Produce(cLinEventProducer, LinFrameStatusEventMatcher(linFrameStatusEvent),
                                                      ReceiverMatcher(receivers)))
        .Times(1);

    SilKit::DETAIL_SILKIT_DETAIL_NAMESPACE_NAME:: Impl::Experimental::NetworkSimulation::LinEventProducer linEventProducer(
        cLinEventProducer);
    linEventProducer.Produce(linFrameStatusEvent, receivers);
}

} //namespace
