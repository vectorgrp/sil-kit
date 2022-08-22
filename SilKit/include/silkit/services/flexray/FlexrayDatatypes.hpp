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

#pragma once

#include <chrono>
#include <vector>

#include "silkit/services/datatypes.hpp"
#include "silkit/util/Span.hpp"

#include "silkit/capi/Flexray.h"

namespace SilKit {
namespace Services {
namespace Flexray {

using FlexrayMicroTick = SilKit_FlexrayMicroTick; //!< FlexRay micro tick

// Not used in the Parameter structures.
// using FlexrayMacroTick = SilKit_FlexrayMacroTick; //!< FlexRay macro tick

//! \brief Type and constants for the FlexRay channel parameter A, B, or AB
enum class FlexrayChannel : SilKit_FlexrayChannel
{
    None = SilKit_FlexrayChannel_None, //!< Invalid Channel
    A    = SilKit_FlexrayChannel_A,    //!< Channel A
    B    = SilKit_FlexrayChannel_B,    //!< Channel B
    AB   = SilKit_FlexrayChannel_AB,   //!< Channel AB
};

//! \brief Period of the clock (used for micro tick period and sample clock period).
enum class FlexrayClockPeriod : SilKit_FlexrayClockPeriod
{
    T12_5NS = SilKit_FlexrayClockPeriod_T12_5NS, //!< 12.5ns / 80MHz
    T25NS   = SilKit_FlexrayClockPeriod_T25NS,   //!< 25ns   / 40MHz
    T50NS   = SilKit_FlexrayClockPeriod_T50NS,   //!< 50ns   / 20MHz
};

/*!
 * \brief Protocol relevant global cluster parameters
 *
 *  Cf. 'FlexRay Protocol Specification Version 3.0.1' Appendix B.3.1.1 Parameters.
 */
struct FlexrayClusterParameters
{
    //! Number of attempts for a cold start before giving up (range 2-31).
    uint8_t gColdstartAttempts;

    //! Max cycle count value in a given cluster (range 7-63, must be an odd integer).
    uint8_t gCycleCountMax;

    //! Time offset for a static slot in MacroTicks (MT) (range 1-63).
    uint16_t gdActionPointOffset;

    ////! Not used by network simulator
    //gdCASRxLowMax   

    //! Duration of the idle phase within a dynamic slot in gdMiniSlots (range 0-2).
    uint16_t gdDynamicSlotIdlePhase;

    ////! Not used by network simulator
    //gdIgnoreAfterTx

    //! Duration of a mini slot in MacroTicks (MT) (2-63).
    uint16_t gdMiniSlot;

    //! Time offset for a mini slot in MacroTicks (MT) (range 1-31).
    uint16_t gdMiniSlotActionPointOffset;

    //! Duration of a static slot in MacroTicks (MT) (3-664).
    uint16_t gdStaticSlot;

    //! Duration of the symbol window in MacroTicks (MT) (range 0-162).
    uint16_t gdSymbolWindow;

    //! Time offset for a static symbol windows in MacroTicks (MT) (range 1-63).
    uint16_t gdSymbolWindowActionPointOffset;

    //! Duration of TSS (Transmission Start Sequence) in gdBits (range 1-15).
    uint16_t gdTSSTransmitter;

    ////! Not used by network simulator
    //gdWakeupRxIdle

    ////! Not used by network simulator
    //gdWakeupRxLow

    ////! Not used by network simulator
    //gdWakeupRxWindow

    //! Duration of LOW Phase of a wakeup symbol in gdBit (range 15-60).
    uint16_t gdWakeupTxActive;

    //! Duration of the idle of a wakeup symbol in gdBit (45-180).
    uint16_t gdWakeupTxIdle;

    /*!
     * Upper limit for the startup listen timeout and wakeup listen timeout in the
     * presence of noise. Used as a multiplier of pdListenTimeout (range 2-16).
     */
    uint8_t gListenNoise;

    //! Number of MacroTicks (MT) per cycle, (range 8-16000).
    uint16_t gMacroPerCycle;

    //! Threshold used for testing the vClockCorrectionFailed counter (range 1-15).
    uint8_t gMaxWithoutClockCorrectionFatal;

    //! Threshold used for testing the vClockCorrectionFailed counter (range 1-15).
    uint8_t gMaxWithoutClockCorrectionPassive;

    //! Number of mini slots (range 0-7988).
    uint16_t gNumberOfMiniSlots;

    //! Number of static slots in a cycle (range 2-1023).
    uint16_t gNumberOfStaticSlots;

    //! Length of the payload of a static frame in 16-Bits words (range 0-127).
    uint16_t gPayloadLengthStatic;

    //! Max number of distinct sync frame identifiers present in a given cluster. (range 2-15).
    uint8_t gSyncFrameIDCountMax;
};

/*!
 * \brief Protocol relevant global node parameters
 *
 *  Cf. 'FlexRay Protocol Specification Version 3.0.1' Appendix B.3.2 Parameters.
 */
struct FlexrayNodeParameters
{
    // ----------------------------------------------------------------------
    // Parameters according to B.3.2.1

    //! Controls the transition to halt state due to clock synchronization errors. (0,1).
    uint8_t pAllowHaltDueToClock;

    //! Required number of consecutive even / odd cycle pairs for normal passive to normal active (range 0-31).
    uint8_t pAllowPassiveToActive;

    //! Channel(s) to which the controller is connected (values FlexrayChannel::A, FlexrayChannel::B, FlexrayChannel::AB).
    FlexrayChannel pChannels;

    //! Cluster drift damping factor for rate correction in MicroTicks (range 0-10).
    uint8_t pClusterDriftDamping;

    //! Allowed deviation for startup frames during integration in MicroTicks (range 29-2743).
    FlexrayMicroTick pdAcceptedStartupRange;

    ////! Not used by network simulator
    //pDecodingCorrection

    ////! Not used by network simulator
    //pDelayCompensationA

    ////! Not used by network simulator
    //pDelayCompensationB

    //! Duration of listen phase in MicroTicks (range 1926-2567692).
    FlexrayMicroTick pdListenTimeout;

    ////! Not used by network simulator
    //pExternalSync

    ////! Not used by network simulator
    //pExternOffsetCorrection

    ////! Not used by network simulator
    //pExternRateCorrection

    ////! Not used by network simulator
    //pFallBackInternal

    //! Slot ID of the key slot (range 0-1023, value 0 means that there is no key slot).
    uint16_t pKeySlotId;

    //! Shall the node enter key slot only mode after startup. (values 0, 1) (AUTOSAR pSingleSlotEnabled).
    uint8_t pKeySlotOnlyEnabled;

    //! Key slot is used for startup (range 0, 1).
    uint8_t pKeySlotUsedForStartup;
    
    //! Key slot is used for sync (range 0, 1).
    uint8_t pKeySlotUsedForSync;

    //! Last mini slot which can be transmitted (range 0-7988).
    uint16_t pLatestTx;

    //! Initial startup offset for frame reference point on channel A (rang 2-68 MacroTicks (MT)).
    uint8_t pMacroInitialOffsetA;

    //! Initial startup offset for frame reference point on channel B (rang 2-68 MacroTicks (MT)).
    uint8_t pMacroInitialOffsetB;

    //! Offset between secondary time reference and MT boundary (range 0-239 MicroTicks).
    FlexrayMicroTick pMicroInitialOffsetA;

    //! Offset between secondary time reference and MT boundary (range 0-239 MicroTicks).
    FlexrayMicroTick pMicroInitialOffsetB;

    //! Nominal number of MicroTicks in the communication cycle (range 960-1280000).
    FlexrayMicroTick pMicroPerCycle;

    //! Maximum permissible offset correction value (range 15-16082 MicroTicks).
    FlexrayMicroTick pOffsetCorrectionOut;

    //! Start of the offset correction phase within the NIT, (7-15999 MT).
    uint16_t pOffsetCorrectionStart;

    //! Maximum permissible rate correction value (range 3-3846 MicroTicks).
    FlexrayMicroTick pRateCorrectionOut;

    ////! Not used by network simulator
    //pSecondKeySlotID

    ////! Not used by network simulator
    //pTwoKeySlotMode

    //! Channel used by the node to send a wakeup pattern (values FlexrayChannel::A, FlexrayChannel::B).
    FlexrayChannel pWakeupChannel;

    //! Number of repetitions of the wakeup symbol (range 0-63, value 0 or 1 prevents sending of WUP).
    uint8_t pWakeupPattern; 

    // ----------------------------------------------------------------------
    // Parameters according to B.3.2.2

    //! Duration of a FlexRay MicroTick (12.5ns, 25ns or 50ns).
    FlexrayClockPeriod pdMicrotick;

    ////! Not used by network simulator
    //pNMVectorEarlyUpdate

    ////! Not used by network simulator
    //pPayloadLengthDynMax

    //! Number of samples per MicroTick (values 1 or 2).
    uint8_t pSamplesPerMicrotick;
};

//! Transmission mode for FlexRay Tx-Buffer
enum class FlexrayTransmissionMode : SilKit_FlexrayTransmissionMode
{
    SingleShot = SilKit_FlexrayTransmissionMode_SingleShot, //!< Send TX Buffer only once
    Continuous = SilKit_FlexrayTransmissionMode_Continuous, //!< Send TX Buffer repeatedly
};

//! Configuration of Tx-Buffer, used in struct FlexrayControllerConfig
struct FlexrayTxBufferConfig
{
    //! (values FlexrayChannel::A, FlexrayChannel::B, FlexrayChannel::AB)
    FlexrayChannel channels;

    //! The slot Id of frame
    uint16_t slotId;

    //! Base offset for cycle multiplexing (values 0-63).
    uint8_t offset;

    //! Repetition for cycle multiplexing (values 1,2,4,8,16,32,64).
    uint8_t repetition;

    //! Set the PPindicator
    bool hasPayloadPreambleIndicator;

    //! Header CRC, 11 bits
    uint16_t headerCrc;

    //! FlexrayTransmissionMode::SingleShot or FlexrayTransmissionMode::Continuous
    FlexrayTransmissionMode transmissionMode;
};

//! Configure the communication parameters of the FlexRay controller.
struct FlexrayControllerConfig
{
    //! FlexRay cluster parameters
    FlexrayClusterParameters clusterParams;
    //! FlexRay node parameters
    FlexrayNodeParameters nodeParams;

    //! FlexRay buffer configs
    std::vector<FlexrayTxBufferConfig> bufferConfigs;
};

//! Update the content of a FlexRay TX-Buffer
struct FlexrayTxBufferUpdate
{
    //! Index of the TX Buffers according to the configured buffers (cf. FlexrayControllerConfig).
    uint16_t txBufferIndex;

    //! Payload data valid flag
    bool payloadDataValid;

    //! Raw payload containing 0 to 254 bytes.
    Util::Span<const uint8_t> payload;
};

struct FlexrayHeader
{
    using FlagMask = SilKit_FlexrayHeader_Flag;

    //! Flag BitMask definition for helper methods
    enum class Flag : FlagMask
    {
        SuFIndicator = SilKit_FlexrayHeader_SuFIndicator,
        SyFIndicator = SilKit_FlexrayHeader_SyFIndicator,
        NFIndicator = SilKit_FlexrayHeader_NFIndicator,
        PPIndicator = SilKit_FlexrayHeader_PPIndicator,
    };

    /*!
     * \brief Flags bit map according to FlagMask. Description:
     *  - [7-5]: unused
     *  - [4]: Reserved bit
     *  - [3]: PPIndicator: 0, regular payload; 1, NM vector or message ID
     *  - [2]: NFIndicator: 0, no valid payload data and PPIndicator = 0; 1, valid payload data
     *  - [1]: SyFIndicator: 0, frame not used for synchronization; 1, frame shall be used for sync
     *  - [0]: SuFIndicator: 0, not a startup frame; 1, a startup frame
     */
    FlagMask flags = 0;
    uint16_t frameId = 0; //!< Slot ID in which the frame was sent: 1 - 2047
    uint8_t payloadLength = 0; //!< Payload length, 7 bits
    uint16_t headerCrc = 0; //!< Header CRC, 11 bits
    uint8_t cycleCount = 0; //!< Cycle in which the frame was sent: 0 - 63
};

struct FlexrayFrame
{
    FlexrayHeader header; //!< Header flags, slot, crc, and cycle indidcators
    Util::Span<const uint8_t> payload; //!< Raw payload containing 0 to 254 bytes
};

// Receive a frame from the Bus.
struct FlexrayFrameEvent
{
    std::chrono::nanoseconds timestamp; //!< Time at end of frame transmission
    FlexrayChannel channel; //!< FlexRay channel A or B. (Valid values: FlexrayChannel::A, FlexrayChannel::B).
    FlexrayFrame frame; //!< Received FlexRay frame
};

/*!
 * \brief Acknowledge for the transmit on the FlexRay bus
 */
struct FlexrayFrameTransmitEvent
{
    std::chrono::nanoseconds timestamp; //!< Time at end of frame transmission
    uint16_t txBufferIndex; //!< Tx buffer, that was used for the transmission
    FlexrayChannel channel; //!< FlexRay channel A or B. (Valid values: FlexrayChannel::A, FlexrayChannel::B).
    FlexrayFrame frame; //!< Copy of the FlexRay frame that was successfully transmitted
};

/*!
 * \brief FlexRay symbols patterns.
 */
enum class FlexraySymbolPattern : SilKit_FlexraySymbolPattern
{
    CasMts = SilKit_FlexraySymbolPattern_CasMts, //!< Collision avoidance symbol (CAS) OR media access test symbol (MTS).
    Wus = SilKit_FlexraySymbolPattern_Wus,       //!< Wakeup symbol (WUS).
    Wudop = SilKit_FlexraySymbolPattern_Wudop,   //!< Wakeup During Operation Pattern (WUDOP).
};

/*!
 * \brief A FlexRay Symbol as received on the FlexRay bus.
 */
struct FlexraySymbolEvent
{
    std::chrono::nanoseconds timestamp; //!< End time of symbol reception.
    FlexrayChannel channel; //!< FlexRay channel A or B (values: FlexrayChannel::A, FlexrayChannel::B).
    FlexraySymbolPattern pattern; //!< The received symbol, e.g. wakeup pattern
};

/*!
 * \brief Acknowledges the transmission of a \ref FlexraySymbolPattern on a particular channel.
 */
struct FlexraySymbolTransmitEvent : FlexraySymbolEvent
{
};

/*!
 * \brief A FlexRay WUS or WUDOP symbol was received on the FlexRay bus.
 */
struct FlexrayWakeupEvent : FlexraySymbolEvent
{
    FlexrayWakeupEvent() = default;

    //! \brief Construct a \ref FlexrayWakeupEvent from the \ref FlexraySymbolEvent that triggered the wakeup.
    explicit FlexrayWakeupEvent(const FlexraySymbolEvent& flexraySymbolEvent);
};

/*!
 * \brief Indicate the start of a FlexRay cycle.
 */
struct FlexrayCycleStartEvent
{
    std::chrono::nanoseconds timestamp; //!< Cycle starting time.
    uint8_t cycleCounter; //!< Counter of FlexRay cycles.
};

/*!
 * \brief Protocol Operation Control (POC) state of the FlexRay communication controller
 * *AUTOSAR Name:* Fr_POCStateType
 */
enum class FlexrayPocState : SilKit_FlexrayPocState
{
    DefaultConfig = SilKit_FlexrayPocState_DefaultConfig, //!< CC expects configuration. Initial state after reset.
    Config        = SilKit_FlexrayPocState_Config,        //!< CC is in configuration mode for setting communication parameters
    Ready         = SilKit_FlexrayPocState_Ready,         //!< intermediate state for initialization process (after Config).
    Startup       = SilKit_FlexrayPocState_Startup,       //!< FlexRay startup phase
    Wakeup        = SilKit_FlexrayPocState_Wakeup,        //!< FlexRay wakeup phase
    NormalActive  = SilKit_FlexrayPocState_NormalActive,  //!< Normal operating mode
    NormalPassive = SilKit_FlexrayPocState_NormalPassive, //!< Operating mode with transient or tolerable errors
    Halt          = SilKit_FlexrayPocState_Halt,          //!< CC is halted (caused by the application (FlexrayChiCommand::DEFERRED_HALT) or by a fatal error).
};

/*!
* \brief Indicates what slot mode the POC is in.
* *AUTOSAR Name:* Fr_SlotModeType
*/
enum class FlexraySlotModeType : SilKit_FlexraySlotModeType
{
    KeySlot = SilKit_FlexraySlotModeType_KeySlot,
    AllPending = SilKit_FlexraySlotModeType_AllPending,
    All = SilKit_FlexraySlotModeType_All,
};

/*!
* \brief Indicates what error mode the POC is in.
* *AUTOSAR Name:* Fr_ErrorModeType
*/
enum class FlexrayErrorModeType : SilKit_FlexrayErrorModeType
{
    Active = SilKit_FlexrayErrorModeType_Active,
    Passive = SilKit_FlexrayErrorModeType_Passive,
    CommHalt = SilKit_FlexrayErrorModeType_CommHalt,
};

/*!
* \brief Indicates the current substate in the startup procedure.
* *AUTOSAR Name:* Fr_StartupStateType
*/

enum class FlexrayStartupStateType : SilKit_FlexrayStartupStateType
{
    Undefined = SilKit_FlexrayStartupStateType_Undefined,
    ColdStartListen = SilKit_FlexrayStartupStateType_ColdStartListen,
    IntegrationColdstartCheck = SilKit_FlexrayStartupStateType_IntegrationColdstartCheck,
    ColdStartJoin = SilKit_FlexrayStartupStateType_ColdStartJoin,
    ColdStartCollisionResolution = SilKit_FlexrayStartupStateType_ColdStartCollisionResolution,
    ColdStartConsistencyCheck = SilKit_FlexrayStartupStateType_ColdStartConsistencyCheck,
    IntegrationListen = SilKit_FlexrayStartupStateType_IntegrationListen,
    InitializeSchedule = SilKit_FlexrayStartupStateType_InitializeSchedule,
    IntegrationConsistencyCheck = SilKit_FlexrayStartupStateType_IntegrationConsistencyCheck,
    ColdStartGap = SilKit_FlexrayStartupStateType_ColdStartGap,
    ExternalStartup = SilKit_FlexrayStartupStateType_ExternalStartup,
};

/*!
* \brief Indicates the outcome of the wake-up mechanism.
* *AUTOSAR Name:* Fr_WakeupStateType
*/
enum class FlexrayWakeupStatusType : SilKit_FlexrayWakeupStatusType
{
    Undefined = SilKit_FlexrayWakeupStatusType_Undefined,
    ReceivedHeader = SilKit_FlexrayWakeupStatusType_ReceivedHeader,
    ReceivedWup = SilKit_FlexrayWakeupStatusType_ReceivedWup,
    CollisionHeader = SilKit_FlexrayWakeupStatusType_CollisionHeader,
    CollisionWup = SilKit_FlexrayWakeupStatusType_CollisionWup,
    CollisionUnknown = SilKit_FlexrayWakeupStatusType_CollisionUnknown,
    Transmitted = SilKit_FlexrayWakeupStatusType_Transmitted,
};

/*!
 * \brief Protocol Operation Control status as available in the AUTOSAR
 *        FlexRay driver model.
 *
 * This enhances the deprecated struct ControllerStatus by adding  members
 * that are available through the Controller Host Interface.
 * *AUTOSAR Name:* Fr_POCStatusType
 * 
 */
struct FlexrayPocStatusEvent
{
    std::chrono::nanoseconds timestamp; //!< SIL Kit timestamp

    FlexrayPocState state; //!< Status of the Protocol Operation Control (POC).
    bool chiHaltRequest; //!< indicates whether a halt request was received from the CHI
    bool coldstartNoise; //!< indicates noisy channel conditions during coldstart 
    bool freeze; //!< indicates that the POC entered a halt state due to an error condition requiring immediate halt.
    bool chiReadyRequest; //!< indicates that the CHI requested to enter ready state at the end of the communication cycle.
    FlexrayErrorModeType errorMode; //!< indicates the error mode of the POC
    FlexraySlotModeType slotMode; //!< indicates the slot mode of the POC
    FlexrayStartupStateType startupState; //!< indicates states within the STARTUP mechanism
    FlexrayWakeupStatusType wakeupStatus; //!< outcome of the execution of the WAKEUP mechanism
};

// ================================================================================
//  Inline Implementations
// ================================================================================

inline bool operator==(const FlexrayClusterParameters& lhs, const FlexrayClusterParameters& rhs)
{
    return lhs.gColdstartAttempts == rhs.gColdstartAttempts
        && lhs.gCycleCountMax == rhs.gCycleCountMax
        && lhs.gdActionPointOffset == rhs.gdActionPointOffset
        && lhs.gdDynamicSlotIdlePhase == rhs.gdDynamicSlotIdlePhase
        && lhs.gdMiniSlot == rhs.gdMiniSlot
        && lhs.gdMiniSlotActionPointOffset == rhs.gdMiniSlotActionPointOffset
        && lhs.gdStaticSlot == rhs.gdStaticSlot
        && lhs.gdSymbolWindow == rhs.gdSymbolWindow
        && lhs.gdSymbolWindowActionPointOffset == rhs.gdSymbolWindowActionPointOffset
        && lhs.gdTSSTransmitter == rhs.gdTSSTransmitter
        && lhs.gdWakeupTxActive == rhs.gdWakeupTxActive
        && lhs.gdWakeupTxIdle == rhs.gdWakeupTxIdle
        && lhs.gListenNoise == rhs.gListenNoise
        && lhs.gMacroPerCycle == rhs.gMacroPerCycle
        && lhs.gMaxWithoutClockCorrectionFatal == rhs.gMaxWithoutClockCorrectionFatal
        && lhs.gMaxWithoutClockCorrectionPassive == rhs.gMaxWithoutClockCorrectionPassive
        && lhs.gNumberOfMiniSlots == rhs.gNumberOfMiniSlots
        && lhs.gNumberOfStaticSlots == rhs.gNumberOfStaticSlots
        && lhs.gPayloadLengthStatic == rhs.gPayloadLengthStatic
        && lhs.gSyncFrameIDCountMax == rhs.gSyncFrameIDCountMax;
}

inline bool operator==(const FlexrayNodeParameters& lhs, const FlexrayNodeParameters& rhs)
{
    return lhs.pAllowHaltDueToClock == rhs.pAllowHaltDueToClock
        && lhs.pAllowPassiveToActive == rhs.pAllowPassiveToActive
        && lhs.pChannels == rhs.pChannels
        && lhs.pClusterDriftDamping == rhs.pClusterDriftDamping
        && lhs.pdAcceptedStartupRange == rhs.pdAcceptedStartupRange
        && lhs.pdListenTimeout == rhs.pdListenTimeout
        && lhs.pKeySlotId == rhs.pKeySlotId
        && lhs.pKeySlotOnlyEnabled == rhs.pKeySlotOnlyEnabled
        && lhs.pKeySlotUsedForStartup == rhs.pKeySlotUsedForStartup
        && lhs.pKeySlotUsedForSync == rhs.pKeySlotUsedForSync
        && lhs.pLatestTx == rhs.pLatestTx
        && lhs.pMacroInitialOffsetA == rhs.pMacroInitialOffsetA
        && lhs.pMacroInitialOffsetB == rhs.pMacroInitialOffsetB
        && lhs.pMicroInitialOffsetA == rhs.pMicroInitialOffsetA
        && lhs.pMicroInitialOffsetB == rhs.pMicroInitialOffsetB
        && lhs.pMicroPerCycle == rhs.pMicroPerCycle
        && lhs.pOffsetCorrectionOut == rhs.pOffsetCorrectionOut
        && lhs.pOffsetCorrectionStart == rhs.pOffsetCorrectionStart
        && lhs.pRateCorrectionOut == rhs.pRateCorrectionOut
        && lhs.pWakeupChannel == rhs.pWakeupChannel
        && lhs.pWakeupPattern == rhs.pWakeupPattern
        && lhs.pdMicrotick == rhs.pdMicrotick
        && lhs.pSamplesPerMicrotick == rhs.pSamplesPerMicrotick;
}

inline bool operator==(const FlexrayTxBufferConfig& lhs, const FlexrayTxBufferConfig& rhs)
{
    return lhs.channels == rhs.channels
        && lhs.slotId == rhs.slotId
        && lhs.offset == rhs.offset
        && lhs.repetition == rhs.repetition
        && lhs.hasPayloadPreambleIndicator == rhs.hasPayloadPreambleIndicator
        && lhs.headerCrc == rhs.headerCrc
        && lhs.transmissionMode == rhs.transmissionMode;
}

inline FlexrayWakeupEvent::FlexrayWakeupEvent(const FlexraySymbolEvent& flexraySymbolEvent)
    : FlexraySymbolEvent{flexraySymbolEvent}
{
}

} // namespace Flexray
} // namespace Services
} // namespace SilKit
