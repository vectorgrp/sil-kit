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

#include "FlexraySerdes.hpp"

namespace SilKit {
namespace Services {
namespace Flexray {

using SilKit::Core::MessageBuffer;

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const FlexrayHeader& header)
{
    buffer
        << header.flags
        << header.frameId
        << header.payloadLength
        << header.headerCrc
        << header.cycleCount;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, FlexrayHeader& header)
{
    buffer
        >> header.flags
        >> header.frameId
        >> header.payloadLength
        >> header.headerCrc
        >> header.cycleCount;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const WireFlexrayFrame& frame)
{
    buffer
        << frame.header
        << frame.payload;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, WireFlexrayFrame& frame)
{
    buffer
        >> frame.header
        >> frame.payload;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const WireFlexrayFrameEvent& msg)
{
    buffer
        << msg.timestamp
        << msg.channel
        << msg.frame;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, WireFlexrayFrameEvent& msg)
{
    buffer
        >> msg.timestamp
        >> msg.channel
        >> msg.frame;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const WireFlexrayFrameTransmitEvent& msg)
{
    buffer
        << msg.timestamp
        << msg.txBufferIndex
        << msg.channel
        << msg.frame;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, WireFlexrayFrameTransmitEvent& msg)
{
    buffer
        >> msg.timestamp
        >> msg.txBufferIndex
        >> msg.channel
        >> msg.frame;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const FlexraySymbolEvent& symbol)
{
    buffer
        << symbol.timestamp
        << symbol.channel
        << symbol.pattern;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, FlexraySymbolEvent& symbol)
{
    buffer
        >> symbol.timestamp
        >> symbol.channel
        >> symbol.pattern;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const FlexraySymbolTransmitEvent& ack)
{
    auto&& symbol = static_cast<const FlexraySymbolEvent&>(ack);
    buffer
        << symbol;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, FlexraySymbolTransmitEvent& ack)
{
    auto&& symbol = static_cast<FlexraySymbolEvent&>(ack);
    buffer
        >> symbol;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer,
                                         const FlexrayCycleStartEvent& flexrayCycleStartEvent)
{
    buffer
        << flexrayCycleStartEvent.timestamp
        << flexrayCycleStartEvent.cycleCounter;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, FlexrayCycleStartEvent& flexrayCycleStartEvent)
{
    buffer
        >> flexrayCycleStartEvent.timestamp
        >> flexrayCycleStartEvent.cycleCounter;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const FlexrayHostCommand& cmd)
{
    buffer
        << cmd.command;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, FlexrayHostCommand& cmd)
{
    buffer
        >> cmd.command;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const FlexrayClusterParameters& clusterParam)
{
    buffer
        << clusterParam.gColdstartAttempts
        << clusterParam.gCycleCountMax
        << clusterParam.gdActionPointOffset
        << clusterParam.gdDynamicSlotIdlePhase
        << clusterParam.gdMiniSlot
        << clusterParam.gdMiniSlotActionPointOffset
        << clusterParam.gdStaticSlot
        << clusterParam.gdSymbolWindow
        << clusterParam.gdSymbolWindowActionPointOffset
        << clusterParam.gdTSSTransmitter
        << clusterParam.gdWakeupTxActive
        << clusterParam.gdWakeupTxIdle
        << clusterParam.gListenNoise
        << clusterParam.gMacroPerCycle
        << clusterParam.gMaxWithoutClockCorrectionFatal
        << clusterParam.gMaxWithoutClockCorrectionPassive
        << clusterParam.gNumberOfMiniSlots
        << clusterParam.gNumberOfStaticSlots
        << clusterParam.gPayloadLengthStatic
        << clusterParam.gSyncFrameIDCountMax;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, FlexrayClusterParameters& clusterParam)
{
    buffer
        >> clusterParam.gColdstartAttempts
        >> clusterParam.gCycleCountMax
        >> clusterParam.gdActionPointOffset
        >> clusterParam.gdDynamicSlotIdlePhase
        >> clusterParam.gdMiniSlot
        >> clusterParam.gdMiniSlotActionPointOffset
        >> clusterParam.gdStaticSlot
        >> clusterParam.gdSymbolWindow
        >> clusterParam.gdSymbolWindowActionPointOffset
        >> clusterParam.gdTSSTransmitter
        >> clusterParam.gdWakeupTxActive
        >> clusterParam.gdWakeupTxIdle
        >> clusterParam.gListenNoise
        >> clusterParam.gMacroPerCycle
        >> clusterParam.gMaxWithoutClockCorrectionFatal
        >> clusterParam.gMaxWithoutClockCorrectionPassive
        >> clusterParam.gNumberOfMiniSlots
        >> clusterParam.gNumberOfStaticSlots
        >> clusterParam.gPayloadLengthStatic
        >> clusterParam.gSyncFrameIDCountMax;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const FlexrayNodeParameters& nodeParams)
{
    buffer
        << nodeParams.pAllowHaltDueToClock
        << nodeParams.pAllowPassiveToActive
        << nodeParams.pChannels
        << nodeParams.pClusterDriftDamping
        << nodeParams.pdAcceptedStartupRange
        << nodeParams.pdListenTimeout
        << nodeParams.pKeySlotId
        << nodeParams.pKeySlotOnlyEnabled
        << nodeParams.pKeySlotUsedForStartup
        << nodeParams.pKeySlotUsedForSync
        << nodeParams.pLatestTx
        << nodeParams.pMacroInitialOffsetA
        << nodeParams.pMacroInitialOffsetB
        << nodeParams.pMicroInitialOffsetA
        << nodeParams.pMicroInitialOffsetB
        << nodeParams.pMicroPerCycle
        << nodeParams.pOffsetCorrectionOut
        << nodeParams.pOffsetCorrectionStart
        << nodeParams.pRateCorrectionOut
        << nodeParams.pWakeupChannel
        << nodeParams.pWakeupPattern
        << nodeParams.pdMicrotick
        << nodeParams.pSamplesPerMicrotick;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, FlexrayNodeParameters& nodeParams)
{
    buffer
        >> nodeParams.pAllowHaltDueToClock
        >> nodeParams.pAllowPassiveToActive
        >> nodeParams.pChannels
        >> nodeParams.pClusterDriftDamping
        >> nodeParams.pdAcceptedStartupRange
        >> nodeParams.pdListenTimeout
        >> nodeParams.pKeySlotId
        >> nodeParams.pKeySlotOnlyEnabled
        >> nodeParams.pKeySlotUsedForStartup
        >> nodeParams.pKeySlotUsedForSync
        >> nodeParams.pLatestTx
        >> nodeParams.pMacroInitialOffsetA
        >> nodeParams.pMacroInitialOffsetB
        >> nodeParams.pMicroInitialOffsetA
        >> nodeParams.pMicroInitialOffsetB
        >> nodeParams.pMicroPerCycle
        >> nodeParams.pOffsetCorrectionOut
        >> nodeParams.pOffsetCorrectionStart
        >> nodeParams.pRateCorrectionOut
        >> nodeParams.pWakeupChannel
        >> nodeParams.pWakeupPattern
        >> nodeParams.pdMicrotick
        >> nodeParams.pSamplesPerMicrotick;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const FlexrayTxBufferConfig& config)
{
    buffer
        << config.channels
        << config.slotId
        << config.offset
        << config.repetition
        << config.hasPayloadPreambleIndicator
        << config.headerCrc
        << config.transmissionMode;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, FlexrayTxBufferConfig& config)
{
    buffer
        >> config.channels
        >> config.slotId
        >> config.offset
        >> config.repetition
        >> config.hasPayloadPreambleIndicator
        >> config.headerCrc
        >> config.transmissionMode;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const FlexrayControllerConfig& config)
{
    buffer
        << config.clusterParams
        << config.nodeParams
        << config.bufferConfigs;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, FlexrayControllerConfig& config)
{
    buffer
        >> config.clusterParams
        >> config.nodeParams
        >> config.bufferConfigs;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const FlexrayTxBufferConfigUpdate& update)
{
    buffer
        << update.txBufferIndex
        << update.txBufferConfig;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, FlexrayTxBufferConfigUpdate& update)
{
    buffer
        >> update.txBufferIndex
        >> update.txBufferConfig;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const WireFlexrayTxBufferUpdate& update)
{
    buffer
        << update.txBufferIndex
        << update.payloadDataValid
        << update.payload;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, WireFlexrayTxBufferUpdate& update)
{
    buffer
        >> update.txBufferIndex
        >> update.payloadDataValid
        >> update.payload;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer,
                                         const FlexrayPocStatusEvent& flexrayPocStatusEvent)
{
    buffer
        << flexrayPocStatusEvent.timestamp
        << flexrayPocStatusEvent.chiHaltRequest
        << flexrayPocStatusEvent.coldstartNoise
        << flexrayPocStatusEvent.errorMode
        << flexrayPocStatusEvent.freeze
        << flexrayPocStatusEvent.slotMode
        << flexrayPocStatusEvent.startupState
        << flexrayPocStatusEvent.state
        << flexrayPocStatusEvent.wakeupStatus
        << flexrayPocStatusEvent.chiReadyRequest;
    return buffer;
}

inline SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, FlexrayPocStatusEvent& flexrayPocStatusEvent)
{
    buffer
      >> flexrayPocStatusEvent.timestamp
      >> flexrayPocStatusEvent.chiHaltRequest
      >> flexrayPocStatusEvent.coldstartNoise
      >> flexrayPocStatusEvent.errorMode
      >> flexrayPocStatusEvent.freeze
      >> flexrayPocStatusEvent.slotMode
      >> flexrayPocStatusEvent.startupState
      >> flexrayPocStatusEvent.state
      >> flexrayPocStatusEvent.wakeupStatus
      >> flexrayPocStatusEvent.chiReadyRequest;
    return buffer;
}


void Serialize(MessageBuffer& buffer, const WireFlexrayFrameEvent& msg)
{
    buffer << msg;
    return;
}

void Serialize(MessageBuffer& buffer, const WireFlexrayFrameTransmitEvent& msg)
{
    buffer << msg;
    return;
}

void Serialize(MessageBuffer& buffer, const FlexraySymbolEvent& msg)
{
    buffer << msg;
    return;
}

void Serialize(MessageBuffer& buffer, const FlexraySymbolTransmitEvent& msg)
{
    buffer << msg;
    return;
}

void Serialize(MessageBuffer& buffer, const FlexrayCycleStartEvent& msg)
{
    buffer << msg;
    return;
}

void Serialize(MessageBuffer& buffer, const FlexrayHostCommand& msg)
{
    buffer << msg;
    return;
}

void Serialize(MessageBuffer& buffer, const FlexrayControllerConfig& msg)
{
    buffer << msg;
    return;
}

void Serialize(MessageBuffer& buffer, const FlexrayTxBufferConfigUpdate& msg)
{
    buffer << msg;
    return;
}

void Serialize(MessageBuffer& buffer, const WireFlexrayTxBufferUpdate& msg)
{
    buffer << msg;
    return;
}

void Serialize(MessageBuffer& buffer, const FlexrayPocStatusEvent& msg)
{
    buffer << msg;
    return;
}

void Deserialize(MessageBuffer& buffer, WireFlexrayFrameEvent& out)
{
    buffer >> out;
}

void Deserialize(MessageBuffer& buffer, WireFlexrayFrameTransmitEvent& out)
{
    buffer >> out;
}

void Deserialize(MessageBuffer& buffer, FlexraySymbolEvent& out)
{
    buffer >> out;
}

void Deserialize(MessageBuffer& buffer, FlexraySymbolTransmitEvent& out)
{
    buffer >> out;
}

void Deserialize(MessageBuffer& buffer, FlexrayCycleStartEvent& out)
{
    buffer >> out;
}

void Deserialize(MessageBuffer& buffer, FlexrayHostCommand& out)
{
    buffer >> out;
}

void Deserialize(MessageBuffer& buffer, FlexrayControllerConfig& out)
{
    buffer >> out;
}

void Deserialize(MessageBuffer& buffer, FlexrayTxBufferConfigUpdate& out)
{
    buffer >> out;
}

void Deserialize(MessageBuffer& buffer, WireFlexrayTxBufferUpdate& out)
{
    buffer >> out;
}

void Deserialize(MessageBuffer& buffer, FlexrayPocStatusEvent& out)
{
    buffer >> out;
}

} // namespace Flexray
} // namespace Services
} // namespace SilKit
