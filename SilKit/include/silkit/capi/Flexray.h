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

#include <stdint.h>
#include "silkit/capi/SilKitMacros.h"
#include "silkit/capi/Types.h"
#include "silkit/capi/InterfaceIdentifiers.h"

#pragma pack(push)
#pragma pack(8)

SILKIT_BEGIN_DECLS

//! FlexRay micro tick
typedef int32_t SilKit_FlexrayMicroTick;
//! FlexRay macro tick
typedef int32_t SilKit_FlexrayMacroTick;

//! \brief Type and constants for the FlexRay channel parameter A, B, or AB
typedef uint32_t SilKit_FlexrayChannel;
#define SilKit_FlexrayChannel_None ((SilKit_FlexrayChannel)0x00)
#define SilKit_FlexrayChannel_A ((SilKit_FlexrayChannel)0x01)
#define SilKit_FlexrayChannel_B ((SilKit_FlexrayChannel)0x02)
#define SilKit_FlexrayChannel_AB ((SilKit_FlexrayChannel)0x03)

//! \brief Period of the clock (used for micro tick period and sample clock period).
typedef uint32_t SilKit_FlexrayClockPeriod;
//!< 12.5ns / 80MHz
#define SilKit_FlexrayClockPeriod_T12_5NS ((SilKit_FlexrayClockPeriod)1)
//!< 25ns   / 40MHz
#define SilKit_FlexrayClockPeriod_T25NS ((SilKit_FlexrayClockPeriod)2)
//!< 50ns   / 20MHz
#define SilKit_FlexrayClockPeriod_T50NS ((SilKit_FlexrayClockPeriod)3)

/*!
 * \brief Protocol relevant global cluster parameters
 *
 *  Cf. 'FlexRay Protocol Specification Version 3.0.1' Appendix B.3.1.1 Parameters.
 */
struct SilKit_FlexrayClusterParameters
{
    //! The interface id specifying which version of this struct was obtained
    SilKit_StructHeader structHeader;

    //! Number of attempts for a cold start before giving up (range 2-31).
    uint8_t gColdstartAttempts;

    //! Max cycle count value in a given cluster (range 7-63, must be an odd integer).
    uint8_t gCycleCountMax;

    //! Time offset for a static slot in MacroTicks (MT) (range 1-63).
    uint16_t gdActionPointOffset;

    ////! Not used by network simulator
    // gdCASRxLowMax

    //! Duration of the idle phase within a dynamic slot in gdMiniSlots (range 0-2).
    uint16_t gdDynamicSlotIdlePhase;

    ////! Not used by network simulator
    // gdIgnoreAfterTx

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
    // gdWakeupRxIdle

    ////! Not used by network simulator
    // gdWakeupRxLow

    ////! Not used by network simulator
    // gdWakeupRxWindow

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
typedef struct SilKit_FlexrayClusterParameters SilKit_FlexrayClusterParameters;

/*!
 * \brief Protocol relevant global node parameters
 *
 *  Cf. 'FlexRay Protocol Specification Version 3.0.1' Appendix B.3.2 Parameters.
 */
struct SilKit_FlexrayNodeParameters
{
    //! The interface id specifying which version of this struct was obtained
    SilKit_StructHeader structHeader;

    // ----------------------------------------------------------------------
    // Parameters according to B.3.2.1

    //! Controls the transition to halt state due to clock synchronization errors. (0,1).
    uint8_t pAllowHaltDueToClock;

    //! Required number of consecutive even / odd cycle pairs for normal passive to normal active (range 0-31).
    uint8_t pAllowPassiveToActive;

    //! Channel(s) to which the controller is connected (values FlexrayChannel::A, FlexrayChannel::B, FlexrayChannel::AB).
    SilKit_FlexrayChannel pChannels;

    //! Cluster drift damping factor for rate correction in MicroTicks (range 0-10).
    uint8_t pClusterDriftDamping;

    //! Allowed deviation for startup frames during integration in MicroTicks (range 29-2743).
    SilKit_FlexrayMicroTick pdAcceptedStartupRange;

    ////! Not used by network simulator
    // pDecodingCorrection

    ////! Not used by network simulator
    // pDelayCompensationA

    ////! Not used by network simulator
    // pDelayCompensationB

    //! Duration of listen phase in MicroTicks (range 1926-2567692).
    SilKit_FlexrayMicroTick pdListenTimeout;

    ////! Not used by network simulator
    // pExternalSync

    ////! Not used by network simulator
    // pExternOffsetCorrection

    ////! Not used by network simulator
    // pExternRateCorrection

    ////! Not used by network simulator
    // pFallBackInternal

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
    SilKit_FlexrayMicroTick pMicroInitialOffsetA;

    //! Offset between secondary time reference and MT boundary (range 0-239 MicroTicks).
    SilKit_FlexrayMicroTick pMicroInitialOffsetB;

    //! Nominal number of MicroTicks in the communication cycle (range 960-1280000).
    SilKit_FlexrayMicroTick pMicroPerCycle;

    //! Maximum permissible offset correction value (range 15-16082 MicroTicks).
    SilKit_FlexrayMicroTick pOffsetCorrectionOut;

    //! Start of the offset correction phase within the NIT, (7-15999 MT).
    uint16_t pOffsetCorrectionStart;

    //! Maximum permissible rate correction value (range 3-3846 MicroTicks).
    SilKit_FlexrayMicroTick pRateCorrectionOut;

    ////! Not used by network simulator
    // pSecondKeySlotID

    ////! Not used by network simulator
    // pTwoKeySlotMode

    //! Channel used by the node to send a wakeup pattern (values FlexrayChannel::A, FlexrayChannel::B).
    SilKit_FlexrayChannel pWakeupChannel;

    //! Number of repetitions of the wakeup symbol (range 0-63, value 0 or 1 prevents sending of WUP).
    uint8_t pWakeupPattern;

    // ----------------------------------------------------------------------
    // Parameters according to B.3.2.2

    //! Duration of a FlexRay MicroTick (12.5ns, 25ns or 50ns).
    SilKit_FlexrayClockPeriod pdMicrotick;

    ////! Not used by network simulator
    // pNMVectorEarlyUpdate

    ////! Not used by network simulator
    // pPayloadLengthDynMax

    //! Number of samples per MicroTick (values 1 or 2).
    uint8_t pSamplesPerMicrotick;
};
typedef struct SilKit_FlexrayNodeParameters SilKit_FlexrayNodeParameters;

//! Transmission mode for FlexRay Tx-Buffer
typedef uint8_t SilKit_FlexrayTransmissionMode;
#define SilKit_FlexrayTransmissionMode_SingleShot ((SilKit_FlexrayTransmissionMode)0)
#define SilKit_FlexrayTransmissionMode_Continuous ((SilKit_FlexrayTransmissionMode)1)

//! Configuration of Tx-Buffer, used in struct FlexrayControllerConfig
struct SilKit_FlexrayTxBufferConfig
{
    //! The interface id specifying which version of this struct was obtained
    SilKit_StructHeader structHeader;

    //! (values FlexrayChannel::A, FlexrayChannel::B, FlexrayChannel::AB)
    SilKit_FlexrayChannel channels;

    //! The slot Id of frame
    uint16_t slotId;

    //! Base offset for cycle multiplexing (values 0-63).
    uint8_t offset;

    //! Repetition for cycle multiplexing (values 1,2,4,8,16,32,64).
    uint8_t repetition;

    //! Set the PPindicator
    SilKit_Bool hasPayloadPreambleIndicator;

    //! Header CRC, 11 bits
    uint16_t headerCrc;

    //! FlexrayTransmissionMode::SingleShot or FlexrayTransmissionMode::Continuous
    SilKit_FlexrayTransmissionMode transmissionMode;
};
typedef struct SilKit_FlexrayTxBufferConfig SilKit_FlexrayTxBufferConfig;

//! Configure the communication parameters of the FlexRay controller.
struct SilKit_FlexrayControllerConfig
{
    //! The interface id specifying which version of this struct was obtained
    SilKit_StructHeader structHeader;
    //! FlexRay cluster parameters
    SilKit_FlexrayClusterParameters* clusterParams;
    //! FlexRay node parameters
    SilKit_FlexrayNodeParameters* nodeParams;

    //! FlexRay buffer configs
    uint32_t numBufferConfigs;
    SilKit_FlexrayTxBufferConfig* bufferConfigs;
};
typedef struct SilKit_FlexrayControllerConfig SilKit_FlexrayControllerConfig;

//! Update the content of a FlexRay TX-Buffer
struct SilKit_FlexrayTxBufferUpdate
{
    //! The interface id specifying which version of this struct was obtained
    SilKit_StructHeader structHeader;

    //! Index of the TX Buffers according to the configured buffers (cf. FlexrayControllerConfig).
    uint16_t txBufferIndex;

    //! Payload data valid flag
    SilKit_Bool payloadDataValid;

    //! Raw payload containing 0 to 254 bytes.
    SilKit_ByteVector payload;
};
typedef struct SilKit_FlexrayTxBufferUpdate SilKit_FlexrayTxBufferUpdate;

//! FlexRay controller commands
typedef uint8_t SilKit_FlexrayChiCommand;
#define SilKit_FlexrayChiCommand_RUN ((SilKit_FlexrayChiCommand)0x00)
#define SilKit_FlexrayChiCommand_DEFERRED_HALT ((SilKit_FlexrayChiCommand)0x01)
#define SilKit_FlexrayChiCommand_FREEZE ((SilKit_FlexrayChiCommand)0x02)
#define SilKit_FlexrayChiCommand_ALLOW_COLDSTART ((SilKit_FlexrayChiCommand)0x03)
#define SilKit_FlexrayChiCommand_ALL_SLOTS ((SilKit_FlexrayChiCommand)0x04)
#define SilKit_FlexrayChiCommand_WAKEUP ((SilKit_FlexrayChiCommand)0x05)

typedef uint8_t SilKit_FlexrayHeader_Flag;
#define SilKit_FlexrayHeader_SuFIndicator ((SilKit_FlexrayHeader_Flag)0x01)
#define SilKit_FlexrayHeader_SyFIndicator ((SilKit_FlexrayHeader_Flag)0x02)
#define SilKit_FlexrayHeader_NFIndicator ((SilKit_FlexrayHeader_Flag)0x04)
#define SilKit_FlexrayHeader_PPIndicator ((SilKit_FlexrayHeader_Flag)0x08)

struct SilKit_FlexrayHeader
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    /*!
    * \brief Flags bit map according to FlagMask. Description:
    *  - [7-5]: unused
    *  - [4]: Reserved bit
    *  - [3]: PPIndicator: 0, regular payload; 1, NM vector or message ID
    *  - [2]: NFIndicator: 0, no valid payload data and PPIndicator = 0; 1, valid payload data
    *  - [1]: SyFIndicator: 0, frame not used for synchronization; 1, frame shall be used for sync
    *  - [0]: SuFIndicator: 0, not a startup frame; 1, a startup frame
    */
    uint8_t flags; //  = 0;
    uint16_t frameId; //  = 0; //!< Slot ID in which the frame was sent: 1 - 2047
    uint8_t payloadLength; //  = 0; //!< Payload length, 7 bits
    uint16_t headerCrc; //  = 0; //!< Header CRC, 11 bits
    uint8_t cycleCount; //  = 0; //!< Cycle in which the frame was sent: 0 - 63
};
typedef struct SilKit_FlexrayHeader SilKit_FlexrayHeader;

struct SilKit_FlexrayFrame
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    SilKit_FlexrayHeader* header; //!< Header flags, slot, crc, and cycle indidcators
    SilKit_ByteVector payload;
};
typedef struct SilKit_FlexrayFrame SilKit_FlexrayFrame;

struct SilKit_FlexrayFrameEvent
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    SilKit_NanosecondsTime timestamp; //!< Time at end of frame transmission
    SilKit_FlexrayChannel channel; //!< FlexRay channel A or B. (Valid values: FlexrayChannel::A, FlexrayChannel::B).
    SilKit_FlexrayFrame* frame; //!< Received FlexRay frame
};
typedef struct SilKit_FlexrayFrameEvent SilKit_FlexrayFrameEvent;

/*!
 * \brief Acknowledge for the transmit on the FlexRay bus
 */
struct SilKit_FlexrayFrameTransmitEvent
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    SilKit_NanosecondsTime timestamp; //!< Time at end of frame transmission
    uint16_t txBufferIndex; //!< Tx buffer, that was used for the transmission
    SilKit_FlexrayChannel channel; //!< FlexRay channel A or B. (Valid values: FlexrayChannel::A, FlexrayChannel::B).
    SilKit_FlexrayFrame* frame; //!< Copy of the FlexRay frame that was successfully transmitted
};
typedef struct SilKit_FlexrayFrameTransmitEvent SilKit_FlexrayFrameTransmitEvent;

/*!
 * \brief FlexRay symbols patterns.
 */
typedef uint8_t SilKit_FlexraySymbolPattern;
//! Collision avoidance symbol (CAS) OR media access test symbol (MTS).
#define SilKit_FlexraySymbolPattern_CasMts ((SilKit_FlexraySymbolPattern)0x00)
//! Wakeup symbol (WUS).
#define SilKit_FlexraySymbolPattern_Wus ((SilKit_FlexraySymbolPattern)0x01)
//! Wakeup During Operation Pattern (WUDOP).
#define SilKit_FlexraySymbolPattern_Wudop ((SilKit_FlexraySymbolPattern)0x02)

/*!
 * \brief A FlexRay Symbol as received on the FlexRay bus.
 */
struct SilKit_FlexraySymbolEvent
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    SilKit_NanosecondsTime timestamp; //!< End time of symbol reception.
    SilKit_FlexrayChannel channel; //!< FlexRay channel A or B (values: FlexrayChannel::A, FlexrayChannel::B).
    SilKit_FlexraySymbolPattern pattern; //!< The received symbol, e.g. wakeup pattern
};
typedef struct SilKit_FlexraySymbolEvent SilKit_FlexraySymbolEvent;
typedef struct SilKit_FlexraySymbolEvent SilKit_FlexraySymbolTransmitEvent;
typedef struct SilKit_FlexraySymbolEvent SilKit_FlexrayWakeupEvent;

/*!
 * \brief Indicate the start of a FlexRay cycle.
 */
struct SilKit_FlexrayCycleStartEvent
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    SilKit_NanosecondsTime timestamp; //!< Cycle starting time.
    uint8_t cycleCounter; //!< Counter of FlexRay cycles.
};
typedef struct SilKit_FlexrayCycleStartEvent SilKit_FlexrayCycleStartEvent;

/*!
 * \brief Protocol Operation Control (POC) state of the FlexRay communication controller
 * *AUTOSAR Name:* Fr_POCStateType
 */
typedef uint8_t SilKit_FlexrayPocState;
//! CC expects configuration. Initial state after reset.
#define SilKit_FlexrayPocState_DefaultConfig ((SilKit_FlexrayPocState)0x00)
//! CC is in configuration mode for setting communication parameters
#define SilKit_FlexrayPocState_Config ((SilKit_FlexrayPocState)0x01)
//! intermediate state for initialization process (after Config).
#define SilKit_FlexrayPocState_Ready ((SilKit_FlexrayPocState)0x02)
//! FlexRay startup phase
#define SilKit_FlexrayPocState_Startup ((SilKit_FlexrayPocState)0x03)
//! FlexRay wakeup phase
#define SilKit_FlexrayPocState_Wakeup ((SilKit_FlexrayPocState)0x04)
//! Normal operating mode
#define SilKit_FlexrayPocState_NormalActive ((SilKit_FlexrayPocState)0x05)
//! Operating mode with transient or tolerable errors
#define SilKit_FlexrayPocState_NormalPassive ((SilKit_FlexrayPocState)0x06)
//! CC is halted (caused by the application (FlexrayChiCommand::DEFERRED_HALT) or by a fatal error).
#define SilKit_FlexrayPocState_Halt ((SilKit_FlexrayPocState)0x07)

/*!
* \brief Indicates what slot mode the POC is in.
* *AUTOSAR Name:* Fr_SlotModeType
*/
typedef uint8_t SilKit_FlexraySlotModeType;
#define SilKit_FlexraySlotModeType_KeySlot ((SilKit_FlexraySlotModeType)0x00)
#define SilKit_FlexraySlotModeType_AllPending ((SilKit_FlexraySlotModeType)0x01)
#define SilKit_FlexraySlotModeType_All ((SilKit_FlexraySlotModeType)0x02)

/*!
* \brief Indicates what error mode the POC is in.
* *AUTOSAR Name:* Fr_ErrorModeType
*/
typedef uint8_t SilKit_FlexrayErrorModeType;
#define SilKit_FlexrayErrorModeType_Active ((SilKit_FlexrayErrorModeType)0x00)
#define SilKit_FlexrayErrorModeType_Passive ((SilKit_FlexrayErrorModeType)0x01)
#define SilKit_FlexrayErrorModeType_CommHalt ((SilKit_FlexrayErrorModeType)0x02)

/*!
* \brief Indicates the current substate in the startup procedure.
* *AUTOSAR Name:* Fr_StartupStateType
*/
typedef uint8_t SilKit_FlexrayStartupStateType;
#define SilKit_FlexrayStartupStateType_Undefined ((SilKit_FlexrayStartupStateType)0x00)
#define SilKit_FlexrayStartupStateType_ColdStartListen ((SilKit_FlexrayStartupStateType)0x01)
#define SilKit_FlexrayStartupStateType_IntegrationColdstartCheck ((SilKit_FlexrayStartupStateType)0x02)
#define SilKit_FlexrayStartupStateType_ColdStartJoin ((SilKit_FlexrayStartupStateType)0x03)
#define SilKit_FlexrayStartupStateType_ColdStartCollisionResolution ((SilKit_FlexrayStartupStateType)0x04)
#define SilKit_FlexrayStartupStateType_ColdStartConsistencyCheck ((SilKit_FlexrayStartupStateType)0x05)
#define SilKit_FlexrayStartupStateType_IntegrationListen ((SilKit_FlexrayStartupStateType)0x06)
#define SilKit_FlexrayStartupStateType_InitializeSchedule ((SilKit_FlexrayStartupStateType)0x07)
#define SilKit_FlexrayStartupStateType_IntegrationConsistencyCheck ((SilKit_FlexrayStartupStateType)0x08)
#define SilKit_FlexrayStartupStateType_ColdStartGap ((SilKit_FlexrayStartupStateType)0x09)
#define SilKit_FlexrayStartupStateType_ExternalStartup ((SilKit_FlexrayStartupStateType)0x0A)

/*!
* \brief Indicates the outcome of the wake-up mechanism.
* *AUTOSAR Name:* Fr_WakeupStateType
*/
typedef uint8_t SilKit_FlexrayWakeupStatusType;
#define SilKit_FlexrayWakeupStatusType_Undefined ((SilKit_FlexrayWakeupStatusType)0x00)
#define SilKit_FlexrayWakeupStatusType_ReceivedHeader ((SilKit_FlexrayWakeupStatusType)0x01)
#define SilKit_FlexrayWakeupStatusType_ReceivedWup ((SilKit_FlexrayWakeupStatusType)0x02)
#define SilKit_FlexrayWakeupStatusType_CollisionHeader ((SilKit_FlexrayWakeupStatusType)0x03)
#define SilKit_FlexrayWakeupStatusType_CollisionWup ((SilKit_FlexrayWakeupStatusType)0x04)
#define SilKit_FlexrayWakeupStatusType_CollisionUnknown ((SilKit_FlexrayWakeupStatusType)0x05)
#define SilKit_FlexrayWakeupStatusType_Transmitted ((SilKit_FlexrayWakeupStatusType)0x06)

/*!
 * \brief Protocol Operation Control status as available in the AUTOSAR
 *        FlexRay driver model.
 *
 * This enhances the deprecated struct ControllerStatus by adding  members
 * that are available through the Controller Host Interface.
 * *AUTOSAR Name:* Fr_POCStatusType
 * 
 */
struct SilKit_FlexrayPocStatusEvent
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    SilKit_NanosecondsTime timestamp; //!< SIL Kit timestamp
    SilKit_FlexrayPocState state;
    /* = SilKit_FlexrayPocState_DefaultConfig; */ //!< Status of the Protocol Operation Control (POC).
    SilKit_Bool chiHaltRequest; /* = false; */ //!< indicates whether a halt request was received from the CHI
    SilKit_Bool coldstartNoise; /* = false; */ //!< indicates noisy channel conditions during coldstart
    SilKit_Bool freeze;
    /* = false; */ //!< indicates that the POC entered a halt state due to an error condition requiring immediate halt.
    SilKit_Bool chiReadyRequest;
    /* = false; */ //!< indicates that the CHI requested to enter ready state at the end of the communication cycle.
    SilKit_FlexrayErrorModeType errorMode;
    /* = SilKit_FlexrayErrorModeType_Active; */ //!< indicates the error mode of the POC
    SilKit_FlexraySlotModeType slotMode;
    /* = SilKit_FlexraySlotModeType_KeySlot; */ //!< indicates the slot mode of the POC
    SilKit_FlexrayStartupStateType startupState;
    /* = SilKit_FlexrayStartupStateType_Undefined; */ //!< indicates states within the STARTUP mechanism
    SilKit_FlexrayWakeupStatusType wakeupStatus;
    /* = SilKit_FlexrayWakeupStatusType_Undefined; */ //!< outcome of the execution of the WAKEUP mechanism
};
typedef struct SilKit_FlexrayPocStatusEvent SilKit_FlexrayPocStatusEvent;

/*! \brief abstract FlexRay Controller object to be used by vECUs
 */
typedef void SilKit_FlexrayController;

/*! Callback type to indicate that a FlexRay message has been received.
  *  Cf. \ref SilKit_FlexrayController_AddFrameHandler;
  */
typedef void(SilKitFPTR* SilKit_FlexrayFrameHandler_t)(void* context, SilKit_FlexrayController* controller,
                                                       const SilKit_FlexrayFrameEvent* message);

/*! Callback type to indicate that a FlexrayFrameTransmitEvent has been received.
  *  Cf. \ref SilKit_FlexrayController_AddFrameTransmitHandler;
  */
typedef void(SilKitFPTR* SilKit_FlexrayFrameTransmitHandler_t)(void* context, SilKit_FlexrayController* controller,
                                                               const SilKit_FlexrayFrameTransmitEvent* acknowledge);

/*! Callback type to indicate that a wakeup has been received.
  *   Should be answered by a call to Run(). Cf. \ref SilKit_FlexrayController_AddWakeupHandler;
  */
typedef void(SilKitFPTR* SilKit_FlexrayWakeupHandler_t)(void* context, SilKit_FlexrayController* controller,
                                                        const SilKit_FlexrayWakeupEvent* symbol);

/*! Callback type to indicate that the POC status (including state variables, modes and error codes) has changed.
  *
  */
typedef void(SilKitFPTR* SilKit_FlexrayPocStatusHandler_t)(void* context, SilKit_FlexrayController* controller,
                                                           const SilKit_FlexrayPocStatusEvent* status);

/*! Callback type to indicate that the controller has received a symbol.
  *  Cf. \ref SilKit_FlexrayController_AddSymbolHandler;
  */
typedef void(SilKitFPTR* SilKit_FlexraySymbolHandler_t)(void* context, SilKit_FlexrayController* controller,
                                                        const SilKit_FlexraySymbolEvent* symbol);

/*! Callback type to indicate that the controller has sent a symbol.
  *  Cf. \ref SilKit_FlexrayController_AddSymbolTransmitHandler;
  */
typedef void(SilKitFPTR* SilKit_FlexraySymbolTransmitHandler_t)(void* context, SilKit_FlexrayController* controller,
                                                                const SilKit_FlexraySymbolTransmitEvent* acknowledge);

/*! Callback type to indicate that a new FlexRay cycle did start.
  *  Cf. \ref SilKit_FlexrayController_AddCycleStartHandler;
  */
typedef void(SilKitFPTR* SilKit_FlexrayCycleStartHandler_t)(void* context, SilKit_FlexrayController* controller,
                                                            const SilKit_FlexrayCycleStartEvent* cycleStart);

/* ! \brief Create a FlexRay controller with the given name.
 * ! \note The object returned must not be deallocated using free()!
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_Create(SilKit_FlexrayController** outController,
                                                                       SilKit_Participant* participant,
                                                                       const char* name, const char* network);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_FlexrayController_Create_t)(SilKit_FlexrayController** outController,
                                                                         SilKit_Participant* participant,
                                                                         const char* name, const char* network);

/*! \brief Apply the given controller configuration to the controller.*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_Configure(SilKit_FlexrayController* controller,
                                                                          const SilKit_FlexrayControllerConfig* config);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_FlexrayController_Configure_t)(
    SilKit_FlexrayController* controller, const SilKit_FlexrayControllerConfig* config);

/*! \brief Reconfigure a TX Buffer that was previously setup with SilKit_FlexrayController_Configure()*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_ReconfigureTxBuffer(
    SilKit_FlexrayController* controller, uint16_t txBufferIdx, const SilKit_FlexrayTxBufferConfig* config);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_FlexrayController_ReconfigureTxBuffer_t)(
    SilKit_FlexrayController* controller, uint16_t txBufferIdx, const SilKit_FlexrayTxBufferConfig* config);

/*! \brief Update the content of a previously configured TX buffer.
  *
  * A FlexRay message will be sent at the time matching the configured Slot ID. 
  * If the buffer was configured with FlexrayTransmissionMode::SingleShot,
  * the content is sent exactly once. If it is configured as FlexrayTransmissionMode::Continuous,
  * the content is sent repeatedly according to the offset and repetition configuration.
  *
  *  \see SilKit_FlexrayController_Configure(const FlexrayControllerConfig&)
  */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_UpdateTxBuffer(
    SilKit_FlexrayController* controller, const SilKit_FlexrayTxBufferUpdate* update);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_FlexrayController_UpdateTxBuffer_t)(
    SilKit_FlexrayController* controller, const SilKit_FlexrayTxBufferUpdate* update);

//! \brief Send the given FlexrayChiCommand.
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_ExecuteCmd(SilKit_FlexrayController* controller,
                                                                           SilKit_FlexrayChiCommand cmd);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_FlexrayController_ExecuteCmd_t)(SilKit_FlexrayController* controller,
                                                                             SilKit_FlexrayChiCommand cmd);

/*! \brief Receive a FlexRay message from the given controller.
 *
 * \param controller The FlexRay controller for which the callback should be registered.
 * \param context The user provided context pointer, that is reobtained in the callback.
 * \param handler The handler to be called.
 * \param outHandlerId The handler identifier that can be used to remove the callback.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_AddFrameHandler(SilKit_FlexrayController* controller,
                                                                                void* context,
                                                                                SilKit_FlexrayFrameHandler_t handler,
                                                                                SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_FlexrayController_AddFrameHandler_t)(SilKit_FlexrayController* controller,
                                                                                  void* context,
                                                                                  SilKit_FlexrayFrameHandler_t handler,
                                                                                  SilKit_HandlerId* outHandlerId);

/*! \brief  Remove a \ref SilKit_FlexrayFrameHandler_t by SilKit_HandlerId on this controller 
*
* \param controller The FlexRay controller for which the callback should be removed.
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_RemoveFrameHandler(SilKit_FlexrayController* controller,
                                                                                   SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_FlexrayController_RemoveFrameHandler_t)(
    SilKit_FlexrayController* controller, SilKit_HandlerId handlerId);

/*! \brief Notification that a FlexRay message has been successfully sent.
 *
 * \param controller The FlexRay controller for which the callback should be registered.
 * \param context The user provided context pointer, that is reobtained in the callback.
 * \param handler The handler to be called.
 * \param outHandlerId The handler identifier that can be used to remove the callback.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_AddFrameTransmitHandler(
    SilKit_FlexrayController* controller, void* context, SilKit_FlexrayFrameTransmitHandler_t handler,
    SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_FlexrayController_AddFrameTransmitHandler_t)(
    SilKit_FlexrayController* controller, void* context, SilKit_FlexrayFrameTransmitHandler_t handler,
    SilKit_HandlerId* outHandlerId);

/*! \brief  Remove a \ref SilKit_FlexrayFrameTransmitHandler_t by SilKit_HandlerId on this controller 
*
* \param controller The FlexRay controller for which the callback should be removed.
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL
SilKit_FlexrayController_RemoveFrameTransmitHandler(SilKit_FlexrayController* controller, SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_FlexrayController_RemoveFrameTransmitHandler_t)(
    SilKit_FlexrayController* controller, SilKit_HandlerId handlerId);

/*! \brief Notification that a wakeup has been received.
 *
 * \param controller The FlexRay controller for which the callback should be registered.
 * \param context The user provided context pointer, that is reobtained in the callback.
 * \param handler The handler to be called.
 * \param outHandlerId The handler identifier that can be used to remove the callback.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_AddWakeupHandler(SilKit_FlexrayController* controller,
                                                                                 void* context,
                                                                                 SilKit_FlexrayWakeupHandler_t handler,
                                                                                 SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_FlexrayController_AddWakeupHandler_t)(
    SilKit_FlexrayController* controller, void* context, SilKit_FlexrayWakeupHandler_t handler,
    SilKit_HandlerId* outHandlerId);

/*! \brief  Remove a \ref SilKit_FlexrayWakeupHandler_t by SilKit_HandlerId on this controller 
*
* \param controller The FlexRay controller for which the callback should be removed.
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL
SilKit_FlexrayController_RemoveWakeupHandler(SilKit_FlexrayController* controller, SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_FlexrayController_RemoveWakeupHandler_t)(
    SilKit_FlexrayController* controller, SilKit_HandlerId handlerId);

/*! \brief Notification that the POC status has changed.
 *
 * \param controller The FlexRay controller for which the callback should be registered.
 * \param context The user provided context pointer, that is reobtained in the callback.
 * \param handler The handler to be called.
 * \param outHandlerId The handler identifier that can be used to remove the callback.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL
SilKit_FlexrayController_AddPocStatusHandler(SilKit_FlexrayController* controller, void* context,
                                             SilKit_FlexrayPocStatusHandler_t handler, SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_FlexrayController_AddPocStatusHandler_t)(
    SilKit_FlexrayController* controller, void* context, SilKit_FlexrayPocStatusHandler_t handler,
    SilKit_HandlerId* outHandlerId);

/*! \brief  Remove a \ref SilKit_FlexrayPocStatusHandler_t by SilKit_HandlerId on this controller 
*
* \param controller The FlexRay controller for which the callback should be removed.
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL
SilKit_FlexrayController_RemovePocStatusHandler(SilKit_FlexrayController* controller, SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_FlexrayController_RemovePocStatusHandler_t)(
    SilKit_FlexrayController* controller, SilKit_HandlerId handlerId);

/*! \brief Notification that the controller has received a symbol.
  *
  * The symbols relevant for interaction trigger also an additional callback,
  * e.g., \ref SilKit_FlexrayWakeupHandler_t.
  *
  * \param controller The FlexRay controller for which the callback should be registered.
  * \param context The user provided context pointer, that is reobtained in the callback.
  * \param handler The handler to be called.
  * \param outHandlerId The handler identifier that can be used to remove the callback.
  */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_AddSymbolHandler(SilKit_FlexrayController* controller,
                                                                                 void* context,
                                                                                 SilKit_FlexraySymbolHandler_t handler,
                                                                                 SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_FlexrayController_AddSymbolHandler_t)(
    SilKit_FlexrayController* controller, void* context, SilKit_FlexraySymbolHandler_t handler,
    SilKit_HandlerId* outHandlerId);

/*! \brief  Remove a \ref SilKit_FlexraySymbolHandler_t by SilKit_HandlerId on this controller 
*
* \param controller The FlexRay controller for which the callback should be removed.
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL
SilKit_FlexrayController_RemoveSymbolHandler(SilKit_FlexrayController* controller, SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_FlexrayController_RemoveSymbolHandler_t)(
    SilKit_FlexrayController* controller, SilKit_HandlerId handlerId);

/*! \brief Notification that the controller has sent a symbol.
  *
  * Currently, the following SymbolPatterns can occur:
  *  - Wakeup() will cause sending the FlexraySymbolPattern::Wus if the bus is idle.
  *  - Run() will cause the transmission of FlexraySymbolPattern::CasMts if configured to coldstart the bus.
  *
  * \param controller The FlexRay controller for which the callback should be registered.
  * \param context The user provided context pointer, that is reobtained in the callback.
  * \param handler The handler to be called.
  * \param outHandlerId The handler identifier that can be used to remove the callback.
  */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_AddSymbolTransmitHandler(
    SilKit_FlexrayController* controller, void* context, SilKit_FlexraySymbolTransmitHandler_t handler,
    SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_FlexrayController_AddSymbolTransmitHandler_t)(
    SilKit_FlexrayController* controller, void* context, SilKit_FlexraySymbolTransmitHandler_t handler,
    SilKit_HandlerId* outHandlerId);

/*! \brief  Remove a \ref SilKit_FlexraySymbolTransmitHandler_t by SilKit_HandlerId on this controller 
*
* \param controller The FlexRay controller for which the callback should be removed.
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL
SilKit_FlexrayController_RemoveSymbolTransmitHandler(SilKit_FlexrayController* controller, SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_FlexrayController_RemoveSymbolTransmitHandler_t)(
    SilKit_FlexrayController* controller, SilKit_HandlerId handlerId);

/*! \brief Notification that a new FlexRay cycle did start.
 *
 * \param controller The FlexRay controller for which the callback should be registered.
 * \param context The user provided context pointer, that is reobtained in the callback.
 * \param handler The handler to be called.
 * \param outHandlerId The handler identifier that can be used to remove the callback.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_FlexrayController_AddCycleStartHandler(
    SilKit_FlexrayController* controller, void* context, SilKit_FlexrayCycleStartHandler_t handler,
    SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_FlexrayController_AddCycleStartHandler_t)(
    SilKit_FlexrayController* controller, void* context, SilKit_FlexrayCycleStartHandler_t handler,
    SilKit_HandlerId* outHandlerId);


/*! \brief  Remove a \ref SilKit_FlexrayCycleStartHandler_t by SilKit_HandlerId on this controller 
*
* \param controller The FlexRay controller for which the callback should be removed.
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL
SilKit_FlexrayController_RemoveCycleStartHandler(SilKit_FlexrayController* controller, SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_FlexrayController_RemoveCycleStartHandler_t)(
    SilKit_FlexrayController* controller, SilKit_HandlerId handlerId);

SILKIT_END_DECLS

#pragma pack(pop)
