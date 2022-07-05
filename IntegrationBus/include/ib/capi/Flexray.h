/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#pragma once

#include <stdint.h>
#include "ib/capi/IbMacros.h"
#include "ib/capi/Types.h"
#include "ib/capi/InterfaceIdentifiers.h"

#pragma pack(push)
#pragma pack(8)

IB_BEGIN_DECLS

//!\typedef FlexRay micro tick
typedef int32_t ib_Flexray_MicroTick;
//!\typedef FlexRay macro tick
typedef int32_t ib_Flexray_MacroTick;

//! \brief Type and constants for the FlexRay channel parameter A, B, or AB
typedef uint32_t ib_Flexray_Channel;
#define ib_Flexray_Channel_None ((ib_Flexray_Channel)0x00)
#define ib_Flexray_Channel_A    ((ib_Flexray_Channel)0x01)
#define ib_Flexray_Channel_B    ((ib_Flexray_Channel)0x02)
#define ib_Flexray_Channel_AB   ((ib_Flexray_Channel)0x03)

//! \brief Period of the clock (used for micro tick period and sample clock period).
typedef uint32_t ib_Flexray_ClockPeriod;
//!< 12.5ns / 80MHz
#define ib_Flexray_ClockPeriod_T12_5NS ((ib_Flexray_ClockPeriod)1)
//!< 25ns   / 40MHz
#define ib_Flexray_ClockPeriod_T25NS   ((ib_Flexray_ClockPeriod)2)
//!< 50ns   / 20MHz
#define ib_Flexray_ClockPeriod_T50NS   ((ib_Flexray_ClockPeriod)3)

/*!
 * \brief Protocol relevant global cluster parameters
 *
 *  Cf. 'FlexRay Protocol Specification Version 3.0.1' Appendix B.3.1.1 Parameters.
 */
struct ib_Flexray_ClusterParameters
{
  //! The interface id specifying which version of this struct was obtained
  ib_InterfaceIdentifier interfaceId;

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
typedef struct ib_Flexray_ClusterParameters ib_Flexray_ClusterParameters;

/*!
 * \brief Protocol relevant global node parameters
 *
 *  Cf. 'FlexRay Protocol Specification Version 3.0.1' Appendix B.3.2 Parameters.
 */
struct ib_Flexray_NodeParameters
{
  //! The interface id specifying which version of this struct was obtained
  ib_InterfaceIdentifier interfaceId;

  // ----------------------------------------------------------------------
  // Parameters according to B.3.2.1

  //! Controls the transition to halt state due to clock synchronization errors. (0,1).
  uint8_t pAllowHaltDueToClock;

  //! Required number of consecutive even / odd cycle pairs for normal passive to normal active (range 0-31).
  uint8_t pAllowPassiveToActive;

  //! Channel(s) to which the controller is connected (values FlexrayChannel::A, FlexrayChannel::B, FlexrayChannel::AB).
  ib_Flexray_Channel pChannels;

  //! Cluster drift damping factor for rate correction in MicroTicks (range 0-10).
  uint8_t pClusterDriftDamping;

  //! Allowed deviation for startup frames during integration in MicroTicks (range 29-2743).
  ib_Flexray_MicroTick pdAcceptedStartupRange;

  ////! Not used by network simulator
  // pDecodingCorrection

  ////! Not used by network simulator
  // pDelayCompensationA

  ////! Not used by network simulator
  // pDelayCompensationB

  //! Duration of listen phase in MicroTicks (range 1926-2567692).
  ib_Flexray_MicroTick pdListenTimeout;

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
  ib_Flexray_MicroTick pMicroInitialOffsetA;

  //! Offset between secondary time reference and MT boundary (range 0-239 MicroTicks).
  ib_Flexray_MicroTick pMicroInitialOffsetB;

  //! Nominal number of MicroTicks in the communication cycle (range 960-1280000).
  ib_Flexray_MicroTick pMicroPerCycle;

  //! Maximum permissible offset correction value (range 15-16082 MicroTicks).
  ib_Flexray_MicroTick pOffsetCorrectionOut;

  //! Start of the offset correction phase within the NIT, (7-15999 MT).
  uint16_t pOffsetCorrectionStart;

  //! Maximum permissible rate correction value (range 3-3846 MicroTicks).
  ib_Flexray_MicroTick pRateCorrectionOut;

  ////! Not used by network simulator
  // pSecondKeySlotID

  ////! Not used by network simulator
  // pTwoKeySlotMode

  //! Channel used by the node to send a wakeup pattern (values FlexrayChannel::A, FlexrayChannel::B).
  ib_Flexray_Channel pWakeupChannel;

  //! Number of repetitions of the wakeup symbol (range 0-63, value 0 or 1 prevents sending of WUP).
  uint8_t pWakeupPattern;

  // ----------------------------------------------------------------------
  // Parameters according to B.3.2.2

  //! Duration of a FlexRay MicroTick (12.5ns, 25ns or 50ns).
  ib_Flexray_ClockPeriod pdMicrotick;

  ////! Not used by network simulator
  // pNMVectorEarlyUpdate

  ////! Not used by network simulator
  // pPayloadLengthDynMax

  //! Number of samples per MicroTick (values 1 or 2).
  uint8_t pSamplesPerMicrotick;
};
typedef struct ib_Flexray_NodeParameters ib_Flexray_NodeParameters;

//! Transmission mode for FlexRay Tx-Buffer
typedef uint8_t ib_Flexray_TransmissionMode;
#define ib_Flexray_TransmissionMode_SingleShot ((ib_Flexray_TransmissionMode)0)
#define ib_Flexray_TransmissionMode_Continuous ((ib_Flexray_TransmissionMode)1)

//! Configuration of Tx-Buffer, used in struct FlexrayControllerConfig
struct ib_Flexray_TxBufferConfig
{
  //! The interface id specifying which version of this struct was obtained
  ib_InterfaceIdentifier interfaceId;

  //! (values FlexrayChannel::A, FlexrayChannel::B, FlexrayChannel::AB)
  ib_Flexray_Channel channels;

  //! The slot Id of frame
  uint16_t slotId;

  //! Base offset for cycle multiplexing (values 0-63).
  uint8_t offset;

  //! Repetition for cycle multiplexing (values 1,2,4,8,16,32,64).
  uint8_t repetition;

  //! Set the PPindicator
  ib_Bool hasPayloadPreambleIndicator;

  //! Header CRC, 11 bits
  uint16_t headerCrc;

  //! FlexrayTransmissionMode::SingleShot or FlexrayTransmissionMode::Continuous
  ib_Flexray_TransmissionMode transmissionMode;
};
typedef struct ib_Flexray_TxBufferConfig ib_Flexray_TxBufferConfig;

//! Configure the communication parameters of the FlexRay controller.
struct ib_Flexray_ControllerConfig
{
  //! The interface id specifying which version of this struct was obtained
  ib_InterfaceIdentifier interfaceId;
  //! FlexRay cluster parameters
  ib_Flexray_ClusterParameters* clusterParams;
  //! FlexRay node parameters
  ib_Flexray_NodeParameters* nodeParams;

  //! FlexRay buffer configs
  uint32_t                   numBufferConfigs;
  ib_Flexray_TxBufferConfig* bufferConfigs;
};
typedef struct ib_Flexray_ControllerConfig ib_Flexray_ControllerConfig;

//! Update the content of a FlexRay TX-Buffer
struct ib_Flexray_TxBufferUpdate
{
  //! The interface id specifying which version of this struct was obtained
  ib_InterfaceIdentifier interfaceId;

  //! Index of the TX Buffers according to the configured buffers (cf. FlexrayControllerConfig).
  uint16_t txBufferIndex;

  //! Payload data valid flag
  ib_Bool payloadDataValid;

  //! Raw payload containing 0 to 254 bytes.
  ib_ByteVector payload;
};
typedef struct ib_Flexray_TxBufferUpdate ib_Flexray_TxBufferUpdate;

//! FlexRay controller commands
typedef uint8_t ib_Flexray_ChiCommand;
#define ib_Flexray_ChiCommand_RUN             ((ib_Flexray_ChiCommand)0x00)
#define ib_Flexray_ChiCommand_DEFERRED_HALT   ((ib_Flexray_ChiCommand)0x01)
#define ib_Flexray_ChiCommand_FREEZE          ((ib_Flexray_ChiCommand)0x02)
#define ib_Flexray_ChiCommand_ALLOW_COLDSTART ((ib_Flexray_ChiCommand)0x03)
#define ib_Flexray_ChiCommand_ALL_SLOTS       ((ib_Flexray_ChiCommand)0x04)
#define ib_Flexray_ChiCommand_WAKEUP          ((ib_Flexray_ChiCommand)0x05)

struct ib_Flexray_HostCommand
{
    //! The interface id specifying which version of this struct was obtained
    ib_InterfaceIdentifier interfaceId;

    ib_Flexray_ChiCommand command;
};

typedef uint8_t ib_Flexray_Header_Flag;
#define ib_Flexray_Header_SuFIndicator ((ib_Flexray_Header_Flag)0x01)
#define ib_Flexray_Header_SyFIndicator ((ib_Flexray_Header_Flag)0x02)
#define ib_Flexray_Header_NFIndicator  ((ib_Flexray_Header_Flag)0x04)
#define ib_Flexray_Header_PPIndicator  ((ib_Flexray_Header_Flag)0x08)

struct ib_Flexray_Header
{
  ib_InterfaceIdentifier interfaceId; //!< The interface id specifying which version of this struct was obtained
  /*!
    * \brief Flags bit map according to FlagMask. Description:
    *  - [7-5]: unused
    *  - [4]: Reserved bit
    *  - [3]: PPIndicator: 0, regular payload; 1, NM vector or message ID
    *  - [2]: NFIndicator: 0, no valid payload data and PPIndicator = 0; 1, valid payload data
    *  - [1]: SyFIndicator: 0, frame not used for synchronization; 1, frame shall be used for sync
    *  - [0]: SuFIndicator: 0, not a startup frame; 1, a startup frame
    */
  uint8_t  flags;         //  = 0;
  uint16_t frameId;       //  = 0; //!< Slot ID in which the frame was sent: 1 - 2047
  uint8_t  payloadLength; //  = 0; //!< Payload length, 7 bits
  uint16_t headerCrc;     //  = 0; //!< Header CRC, 11 bits
  uint8_t  cycleCount;    //  = 0; //!< Cycle in which the frame was sent: 0 - 63
};
typedef struct ib_Flexray_Header ib_Flexray_Header;

struct ib_Flexray_Frame
{
  ib_InterfaceIdentifier interfaceId; //!< The interface id specifying which version of this struct was obtained
  ib_Flexray_Header* header;  //!< Header flags, slot, crc, and cycle indidcators
  ib_ByteVector     payload;
};
typedef struct ib_Flexray_Frame ib_Flexray_Frame;

struct ib_Flexray_FrameEvent
{
  ib_InterfaceIdentifier interfaceId; //!< The interface id specifying which version of this struct was obtained
  ib_NanosecondsTime     timestamp;   //!< Time at end of frame transmission
  ib_Flexray_Channel     channel;     //!< FlexRay channel A or B. (Valid values: FlexrayChannel::A, FlexrayChannel::B).
  ib_Flexray_Frame*      frame;       //!< Received FlexRay frame
};
typedef struct ib_Flexray_FrameEvent ib_Flexray_FrameEvent;

/*!
 * \brief Acknowledge for the transmit on the FlexRay bus
 */
struct ib_Flexray_FrameTransmitEvent
{
  ib_InterfaceIdentifier interfaceId;   //!< The interface id specifying which version of this struct was obtained
  ib_NanosecondsTime     timestamp;     //!< Time at end of frame transmission
  uint16_t               txBufferIndex; //!< Tx buffer, that was used for the transmission
  ib_Flexray_Channel     channel;       //!< FlexRay channel A or B. (Valid values: FlexrayChannel::A, FlexrayChannel::B).
  ib_Flexray_Frame*      frame;         //!< Copy of the FlexRay frame that was successfully transmitted
};
typedef struct ib_Flexray_FrameTransmitEvent ib_Flexray_FrameTransmitEvent;

/*!
 * \brief FlexRay symbols patterns.
 */
typedef uint8_t ib_Flexray_SymbolPattern;
//!< Collision avoidance symbol (CAS) OR media access test symbol (MTS).
#define ib_Flexray_SymbolPattern_CasMts ((ib_Flexray_SymbolPattern)0x00)
//!< Wakeup symbol (WUS).
#define ib_Flexray_SymbolPattern_Wus    ((ib_Flexray_SymbolPattern)0x01)
//!< Wakeup During Operation Pattern (WUDOP).
#define ib_Flexray_SymbolPattern_Wudop  ((ib_Flexray_SymbolPattern)0x02)

/*!
 * \brief A FlexRay Symbol as received on the FlexRay bus.
 */
struct ib_Flexray_SymbolEvent
{
  ib_InterfaceIdentifier   interfaceId; //!< The interface id specifying which version of this struct was obtained
  ib_NanosecondsTime       timestamp;   //!< End time of symbol reception.
  ib_Flexray_Channel       channel;     //!< FlexRay channel A or B (values: FlexrayChannel::A, FlexrayChannel::B).
  ib_Flexray_SymbolPattern pattern;     //!< The received symbol, e.g. wakeup pattern
};
typedef struct ib_Flexray_SymbolEvent ib_Flexray_SymbolEvent;
typedef struct ib_Flexray_SymbolEvent ib_Flexray_SymbolTransmitEvent;
typedef struct ib_Flexray_SymbolEvent ib_Flexray_WakeupEvent;

/*!
 * \brief Indicate the start of a FlexRay cycle.
 */
struct ib_Flexray_CycleStartEvent
{
  ib_InterfaceIdentifier interfaceId;  //!< The interface id specifying which version of this struct was obtained
  ib_NanosecondsTime     timestamp;    //!< Cycle starting time.
  uint8_t                cycleCounter; //!< Counter of FlexRay cycles.
};
typedef struct ib_Flexray_CycleStartEvent ib_Flexray_CycleStartEvent;

/*!
 * \brief Protocol Operation Control (POC) state of the FlexRay communication controller
 * *AUTOSAR Name:* Fr_POCStateType
 */
typedef uint8_t ib_Flexray_PocState;
//!< CC expects configuration. Initial state after reset.
#define ib_Flexray_PocState_DefaultConfig ((ib_Flexray_PocState)0x00)
//!< CC is in configuration mode for setting communication parameters
#define ib_Flexray_PocState_Config        ((ib_Flexray_PocState)0x01)
//!< intermediate state for initialization process (after Config).
#define ib_Flexray_PocState_Ready         ((ib_Flexray_PocState)0x02)
//!< FlexRay startup phase
#define ib_Flexray_PocState_Startup       ((ib_Flexray_PocState)0x03)
//!< FlexRay wakeup phase
#define ib_Flexray_PocState_Wakeup        ((ib_Flexray_PocState)0x04)
//!< Normal operating mode
#define ib_Flexray_PocState_NormalActive  ((ib_Flexray_PocState)0x05)
//!< Operating mode with transient or tolerable errors
#define ib_Flexray_PocState_NormalPassive ((ib_Flexray_PocState)0x06)
//!< CC is halted (caused by the application (FlexrayChiCommand::DEFERRED_HALT) or by a fatal error).
#define ib_Flexray_PocState_Halt          ((ib_Flexray_PocState)0x07)

/*!
* \brief Indicates what slot mode the POC is in.
* *AUTOSAR Name:* Fr_SlotModeType
*/
typedef uint8_t ib_Flexray_SlotModeType;
#define ib_Flexray_SlotModeType_KeySlot    ((ib_Flexray_SlotModeType)0x00)
#define ib_Flexray_SlotModeType_AllPending ((ib_Flexray_SlotModeType)0x01)
#define ib_Flexray_SlotModeType_All        ((ib_Flexray_SlotModeType)0x02)

/*!
* \brief Indicates what error mode the POC is in.
* *AUTOSAR Name:* Fr_ErrorModeType
*/
typedef uint8_t ib_Flexray_ErrorModeType;
#define ib_Flexray_ErrorModeType_Active   ((ib_Flexray_ErrorModeType)0x00)
#define ib_Flexray_ErrorModeType_Passive  ((ib_Flexray_ErrorModeType)0x01)
#define ib_Flexray_ErrorModeType_CommHalt ((ib_Flexray_ErrorModeType)0x02)

/*!
* \brief Indicates the current substate in the startup procedure.
* *AUTOSAR Name:* Fr_StartupStateType
*/
typedef uint8_t ib_Flexray_StartupStateType;
#define ib_Flexray_StartupStateType_Undefined                    ((ib_Flexray_StartupStateType)0x00)
#define ib_Flexray_StartupStateType_ColdStartListen              ((ib_Flexray_StartupStateType)0x01)
#define ib_Flexray_StartupStateType_IntegrationColdstartCheck    ((ib_Flexray_StartupStateType)0x02)
#define ib_Flexray_StartupStateType_ColdStartJoin                ((ib_Flexray_StartupStateType)0x03)
#define ib_Flexray_StartupStateType_ColdStartCollisionResolution ((ib_Flexray_StartupStateType)0x04)
#define ib_Flexray_StartupStateType_ColdStartConsistencyCheck    ((ib_Flexray_StartupStateType)0x05)
#define ib_Flexray_StartupStateType_IntegrationListen            ((ib_Flexray_StartupStateType)0x06)
#define ib_Flexray_StartupStateType_InitializeSchedule           ((ib_Flexray_StartupStateType)0x07)
#define ib_Flexray_StartupStateType_IntegrationConsistencyCheck  ((ib_Flexray_StartupStateType)0x08)
#define ib_Flexray_StartupStateType_ColdStartGap                 ((ib_Flexray_StartupStateType)0x09)
#define ib_Flexray_StartupStateType_ExternalStartup              ((ib_Flexray_StartupStateType)0x0A)

/*!
* \brief Indicates the outcome of the wake-up mechanism.
* *AUTOSAR Name:* Fr_WakeupStateType
*/
typedef uint8_t ib_Flexray_WakeupStatusType;
#define ib_Flexray_WakeupStatusType_Undefined        ((ib_Flexray_WakeupStatusType)0x00)
#define ib_Flexray_WakeupStatusType_ReceivedHeader   ((ib_Flexray_WakeupStatusType)0x01)
#define ib_Flexray_WakeupStatusType_ReceivedWup      ((ib_Flexray_WakeupStatusType)0x02)
#define ib_Flexray_WakeupStatusType_CollisionHeader  ((ib_Flexray_WakeupStatusType)0x03)
#define ib_Flexray_WakeupStatusType_CollisionWup     ((ib_Flexray_WakeupStatusType)0x04)
#define ib_Flexray_WakeupStatusType_CollisionUnknown ((ib_Flexray_WakeupStatusType)0x05)
#define ib_Flexray_WakeupStatusType_Transmitted      ((ib_Flexray_WakeupStatusType)0x06)

/*!
 * \brief Protocol Operation Control status as available in the AUTOSAR
 *        FlexRay driver model.
 *
 * This enhances the deprecated struct ControllerStatus by adding  members
 * that are available through the Controller Host Interface.
 * *AUTOSAR Name:* Fr_POCStatusType
 * 
 */
struct ib_Flexray_PocStatusEvent
{
  ib_InterfaceIdentifier      interfaceId;     //!< The interface id specifying which version of this struct was obtained
  ib_NanosecondsTime          timestamp;       //!< IB timestamp
  ib_Flexray_PocState         state;           /* = ib_Flexray_PocState_DefaultConfig; */     //!< Status of the Protocol Operation Control (POC).
  ib_Bool                     chiHaltRequest;  /* = false; */                                 //!< indicates whether a halt request was received from the CHI
  ib_Bool                     coldstartNoise;  /* = false; */                                 //!< indicates noisy channel conditions during coldstart
  ib_Bool                     freeze;          /* = false; */                                 //!< indicates that the POC entered a halt state due to an error condition requiring immediate halt.
  ib_Bool                     chiReadyRequest; /* = false; */                                 //!< indicates that the CHI requested to enter ready state at the end of the communication cycle.
  ib_Flexray_ErrorModeType    errorMode;       /* = ib_Flexray_ErrorModeType_Active; */       //!< indicates the error mode of the POC
  ib_Flexray_SlotModeType     slotMode;        /* = ib_Flexray_SlotModeType_KeySlot; */       //!< indicates the slot mode of the POC
  ib_Flexray_StartupStateType startupState;    /* = ib_Flexray_StartupStateType_Undefined; */ //!< indicates states within the STARTUP mechanism
  ib_Flexray_WakeupStatusType wakeupStatus;    /* = ib_Flexray_WakeupStatusType_Undefined; */ //!< outcome of the execution of the WAKEUP mechanism
};
typedef struct ib_Flexray_PocStatusEvent ib_Flexray_PocStatusEvent;

/*! \brief abstract FlexRay Controller object to be used by vECUs
 */
typedef void ib_Flexray_Controller;

/*! Callback type to indicate that a FlexRay message has been received.
  *  Cf. \ref AddFrameHandler();
  */
typedef void (*ib_Flexray_FrameHandler_t)(void* context, ib_Flexray_Controller* controller, const ib_Flexray_FrameEvent* message);
/*! Callback type to indicate that a FlexrayFrameTransmitEvent has been received.
  *  Cf. \ref AddFrameTransmitHandler();
  */
typedef void (*ib_Flexray_FrameTransmitHandler_t)(void* context, ib_Flexray_Controller* controller, const ib_Flexray_FrameTransmitEvent* acknowledge);

/*! Callback type to indicate that a wakeup has been received.
  *   Should be answered by a call to Run(). Cf. \ref AddWakeupHandler();
  */
typedef void (*ib_Flexray_WakeupHandler_t)(void* context, ib_Flexray_Controller* controller, const ib_Flexray_WakeupEvent* symbol);

/*! Callback type to indicate that the POC status (including state variables, modes and error codes) has changed.
  *
  */
typedef void (*ib_Flexray_PocStatusHandler_t)(void* context, ib_Flexray_Controller* controller, const ib_Flexray_PocStatusEvent* status);

/*! Callback type to indicate that the controller has received a symbol.
  *  Cf. \ref AddSymbolHandler();
  */
typedef void (*ib_Flexray_SymbolHandler_t)(void* context, ib_Flexray_Controller* controller, const ib_Flexray_SymbolEvent* symbol);

/*! Callback type to indicate that the controller has sent a symbol.
  *  Cf. \ref AddSymbolTransmitHandler();
  */
typedef void (*ib_Flexray_SymbolTransmitHandler_t)(void* context, ib_Flexray_Controller* controller, const ib_Flexray_SymbolTransmitEvent* acknowledge);

/*! Callback type to indicate that a new FlexRay cycle did start.
  *  Cf. \ref AddCycleStartHandler();
  */
typedef void (*ib_Flexray_CycleStartHandler_t)(void* context, ib_Flexray_Controller* controller, const ib_Flexray_CycleStartEvent* cycleStart);

/* ! \brief Create a FlexRay controller with the given name.
 * ! \note The object returned must not be deallocated using free()!
 */
IntegrationBusAPI ib_ReturnCode ib_Flexray_Controller_Create(ib_Flexray_Controller** outController,
                                                             ib_Participant* participant, const char* name,
                                                             const char* network);

typedef ib_ReturnCode (*ib_Flexray_Controller_Create_t)(ib_Flexray_Controller** outController,
                                                        ib_Participant* participant, const char* name,
                                                        const char* network);

/*! \brief Apply the given controller configuration to the controller.*/
IntegrationBusAPI ib_ReturnCode ib_Flexray_Controller_Configure(ib_Flexray_Controller* controller, const ib_Flexray_ControllerConfig* config);

typedef ib_ReturnCode (*ib_Flexray_Controller_Configure_t)(ib_Flexray_Controller* controller, const ib_Flexray_ControllerConfig* config);

/*! \brief Reconfigure a TX Buffer that was previously setup with ib_Flexray_Controller_Configure()*/
IntegrationBusAPI ib_ReturnCode ib_Flexray_Controller_ReconfigureTxBuffer(ib_Flexray_Controller* controller, uint16_t txBufferIdx, const ib_Flexray_TxBufferConfig* config);

typedef ib_ReturnCode (*ib_Flexray_Controller_ReconfigureTxBuffer_t)(ib_Flexray_Controller* controller, uint16_t txBufferIdx, const ib_Flexray_TxBufferConfig* config);

/*! \brief Update the content of a previously configured TX buffer.
  *
  * A FlexRay message will be sent at the time matching the configured Slot ID. 
  * If the buffer was configured with FlexrayTransmissionMode::SingleShot,
  * the content is sent exactly once. If it is configured as FlexrayTransmissionMode::Continuous,
  * the content is sent repeatedly according to the offset and repetition configuration.
  *
  *  \see ib_Flexray_Controller_Configure(const FlexrayControllerConfig&)
  */
IntegrationBusAPI ib_ReturnCode ib_Flexray_Controller_UpdateTxBuffer(ib_Flexray_Controller* controller, const ib_Flexray_TxBufferUpdate* update);

typedef ib_ReturnCode (*ib_Flexray_Controller_UpdateTxBuffer_t)(ib_Flexray_Controller* controller, const ib_Flexray_TxBufferUpdate* update);

//! \brief Send the given FlexrayChiCommand.
IntegrationBusAPI ib_ReturnCode ib_Flexray_Controller_ExecuteCmd(ib_Flexray_Controller* controller, ib_Flexray_ChiCommand cmd);

typedef ib_ReturnCode (*ib_Flexray_Controller_ExecuteCmd_t)(ib_Flexray_Controller* controller, ib_Flexray_ChiCommand cmd);

/*! \brief Receive a FlexRay message from the given controller.
 *
 * \param controller The FlexRay controller for which the callback should be registered.
 * \param context The user provided context pointer, that is reobtained in the callback.
 * \param handler The handler to be called.
 * \param outHandlerId The handler identifier that can be used to remove the callback.
 */
IntegrationBusAPI ib_ReturnCode ib_Flexray_Controller_AddFrameHandler(ib_Flexray_Controller* controller, void* context,
                                                                      ib_Flexray_FrameHandler_t handler,
                                                                      ib_HandlerId* outHandlerId);

typedef ib_ReturnCode (*ib_Flexray_Controller_AddFrameHandler_t)(ib_Flexray_Controller* controller, void* context,
                                                                 ib_Flexray_FrameHandler_t handler,
                                                                 ib_HandlerId* outHandlerId);

/*! \brief  Remove a \ref ib_Flexray_FrameHandler_t by ib_HandlerId on this controller 
*
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
IntegrationBusAPI ib_ReturnCode ib_Flexray_Controller_RemoveFrameHandler(ib_Flexray_Controller* controller,
                                                                         ib_HandlerId handlerId);

typedef ib_ReturnCode (*ib_Flexray_Controller_RemoveFrameHandler_t)(ib_Flexray_Controller* controller,
                                                                    ib_HandlerId handlerId);

/*! \brief Notification that a FlexRay message has been successfully sent.
 *
 * \param controller The FlexRay controller for which the callback should be registered.
 * \param context The user provided context pointer, that is reobtained in the callback.
 * \param handler The handler to be called.
 * \param outHandlerId The handler identifier that can be used to remove the callback.
 */
IntegrationBusAPI ib_ReturnCode ib_Flexray_Controller_AddFrameTransmitHandler(ib_Flexray_Controller* controller,
                                                                              void* context,
                                                                              ib_Flexray_FrameTransmitHandler_t handler,
                                                                              ib_HandlerId* outHandlerId);

typedef ib_ReturnCode (*ib_Flexray_Controller_AddFrameTransmitHandler_t)(ib_Flexray_Controller* controller,
                                                                         void* context,
                                                                         ib_Flexray_FrameTransmitHandler_t handler,
                                                                         ib_HandlerId* outHandlerId);

/*! \brief  Remove a \ref ib_Flexray_FrameTransmitHandler_t by ib_HandlerId on this controller 
*
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
IntegrationBusAPI ib_ReturnCode ib_Flexray_Controller_RemoveFrameTransmitHandler(ib_Flexray_Controller* controller,
                                                                                 ib_HandlerId handlerId);

typedef ib_ReturnCode (*ib_Flexray_Controller_RemoveFrameTransmitHandler_t)(ib_Flexray_Controller* controller,
                                                                            ib_HandlerId handlerId);

/*! \brief Notification that a wakeup has been received.
 *
 * \param controller The FlexRay controller for which the callback should be registered.
 * \param context The user provided context pointer, that is reobtained in the callback.
 * \param handler The handler to be called.
 * \param outHandlerId The handler identifier that can be used to remove the callback.
 */
IntegrationBusAPI ib_ReturnCode ib_Flexray_Controller_AddWakeupHandler(ib_Flexray_Controller* controller, void* context,
                                                                       ib_Flexray_WakeupHandler_t handler,
                                                                       ib_HandlerId* outHandlerId);

typedef ib_ReturnCode (*ib_Flexray_Controller_AddWakeupHandler_t)(ib_Flexray_Controller* controller, void* context,
                                                                  ib_Flexray_WakeupHandler_t handler,
                                                                  ib_HandlerId* outHandlerId);

/*! \brief  Remove a \ref ib_Flexray_WakeupHandler_t by ib_HandlerId on this controller 
*
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
IntegrationBusAPI ib_ReturnCode ib_Flexray_Controller_RemoveWakeupHandler(ib_Flexray_Controller* controller,
                                                                          ib_HandlerId handlerId);

typedef ib_ReturnCode (*ib_Flexray_Controller_RemoveWakeupHandler_t)(ib_Flexray_Controller* controller,
                                                                     ib_HandlerId handlerId);

/*! \brief Notification that the POC status has changed.
 *
 * \param controller The FlexRay controller for which the callback should be registered.
 * \param context The user provided context pointer, that is reobtained in the callback.
 * \param handler The handler to be called.
 * \param outHandlerId The handler identifier that can be used to remove the callback.
 */
IntegrationBusAPI ib_ReturnCode ib_Flexray_Controller_AddPocStatusHandler(ib_Flexray_Controller* controller,
                                                                          void* context,
                                                                          ib_Flexray_PocStatusHandler_t handler,
                                                                          ib_HandlerId* outHandlerId);

typedef ib_ReturnCode (*ib_Flexray_Controller_AddPocStatusHandler_t)(ib_Flexray_Controller* controller, void* context,
                                                                     ib_Flexray_PocStatusHandler_t handler,
                                                                     ib_HandlerId* outHandlerId);

/*! \brief  Remove a \ref ib_Flexray_PocStatusHandler_t by ib_HandlerId on this controller 
*
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
IntegrationBusAPI ib_ReturnCode ib_Flexray_Controller_RemovePocStatusHandler(ib_Flexray_Controller* controller,
                                                                             ib_HandlerId handlerId);

typedef ib_ReturnCode (*ib_Flexray_Controller_RemovePocStatusHandler_t)(ib_Flexray_Controller* controller,
                                                                        ib_HandlerId handlerId);

/*! \brief Notification that the controller has received a symbol.
  *
  * This callback is primarily intended for tracing. There is no need to react on it.
  * The symbols relevant for interaction trigger also an additional callback,
  * e.g., \ref WakeupHandler.
  *
  * \param controller The FlexRay controller for which the callback should be registered.
  * \param context The user provided context pointer, that is reobtained in the callback.
  * \param handler The handler to be called.
  * \param outHandlerId The handler identifier that can be used to remove the callback.
  */
IntegrationBusAPI ib_ReturnCode ib_Flexray_Controller_AddSymbolHandler(ib_Flexray_Controller* controller, void* context,
                                                                       ib_Flexray_SymbolHandler_t handler,
                                                                       ib_HandlerId* outHandlerId);

typedef ib_ReturnCode (*ib_Flexray_Controller_AddSymbolHandler_t)(ib_Flexray_Controller* controller, void* context,
                                                                  ib_Flexray_SymbolHandler_t handler,
                                                                  ib_HandlerId* outHandlerId);

/*! \brief  Remove a \ref ib_Flexray_SymbolHandler_t by ib_HandlerId on this controller 
*
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
IntegrationBusAPI ib_ReturnCode ib_Flexray_Controller_RemoveSymbolHandler(ib_Flexray_Controller* controller,
                                                                          ib_HandlerId handlerId);

typedef ib_ReturnCode (*ib_Flexray_Controller_RemoveSymbolHandler_t)(ib_Flexray_Controller* controller,
                                                                     ib_HandlerId handlerId);

/*! \brief Notification that the controller has sent a symbol.
  *
  * This callback is primarily intended for tracing. There is no need to react on it.
  * Currently, the following SymbolPatterns can occur:
  *  - Wakeup() will cause sending the FlexraySymbolPattern::Wus, if the bus is idle.
  *  - Run() will cause the transmission of FlexraySymbolPattern::CasMts if configured to coldstart the bus.
  *
  * \param controller The FlexRay controller for which the callback should be registered.
  * \param context The user provided context pointer, that is reobtained in the callback.
  * \param handler The handler to be called.
  * \param outHandlerId The handler identifier that can be used to remove the callback.
  */
IntegrationBusAPI ib_ReturnCode
ib_Flexray_Controller_AddSymbolTransmitHandler(ib_Flexray_Controller* controller, void* context,
                                               ib_Flexray_SymbolTransmitHandler_t handler, ib_HandlerId* outHandlerId);

typedef ib_ReturnCode (*ib_Flexray_Controller_AddSymbolTransmitHandler_t)(ib_Flexray_Controller* controller,
                                                                          void* context,
                                                                          ib_Flexray_SymbolTransmitHandler_t handler,
                                                                          ib_HandlerId* outHandlerId);

/*! \brief  Remove a \ref ib_Flexray_SymbolTransmitHandler_t by ib_HandlerId on this controller 
*
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
IntegrationBusAPI ib_ReturnCode ib_Flexray_Controller_RemoveSymbolTransmitHandler(ib_Flexray_Controller* controller,
                                                                                  ib_HandlerId handlerId);

typedef ib_ReturnCode (*ib_Flexray_Controller_RemoveSymbolTransmitHandler_t)(ib_Flexray_Controller* controller,
                                                                             ib_HandlerId handlerId);

/*! \brief Notification that a new FlexRay cycle did start.
 *
 * \param controller The FlexRay controller for which the callback should be registered.
 * \param context The user provided context pointer, that is reobtained in the callback.
 * \param handler The handler to be called.
 * \param outHandlerId The handler identifier that can be used to remove the callback.
 */
IntegrationBusAPI ib_ReturnCode ib_Flexray_Controller_AddCycleStartHandler(ib_Flexray_Controller* controller,
                                                                           void* context,
                                                                           ib_Flexray_CycleStartHandler_t handler,
                                                                           ib_HandlerId* outHandlerId);

typedef ib_ReturnCode (*ib_Flexray_Controller_AddCycleStartHandler_t)(ib_Flexray_Controller* controller, void* context,
                                                                      ib_Flexray_CycleStartHandler_t handler,
                                                                      ib_HandlerId* outHandlerId);


/*! \brief  Remove a \ref ib_Flexray_CycleStartHandler_t by ib_HandlerId on this controller 
*
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
IntegrationBusAPI ib_ReturnCode ib_Flexray_Controller_RemoveCycleStartHandler(ib_Flexray_Controller* controller,
                                                                              ib_HandlerId handlerId);

typedef ib_ReturnCode (*ib_Flexray_Controller_RemoveCycleStartHandler_t)(ib_Flexray_Controller* controller,
                                                                         ib_HandlerId handlerId);

IB_END_DECLS

#pragma pack(pop)
