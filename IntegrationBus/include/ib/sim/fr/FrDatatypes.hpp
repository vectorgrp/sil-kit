// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <chrono>
#include <vector>

#include "ib/sim/datatypes.hpp"

namespace ib {
namespace sim {
namespace fr {

using FrMickroTick     = int32_t;
using FrMacroTick      = int32_t;

//! Type and constants for the FlexRay channel parameter A, B, or AB
enum class Channel : uint8_t
{
    None = 0,
    A    = 1,
    B    = 2,
    AB   = 3
};

//! Period of the clock (used for micro tick period and sample clock period)
enum class ClockPeriod : uint8_t
{
    T12_5NS = 1, //!< 12.5ns / 80MHz
    T25NS = 2, //!< 25ns   / 40MHz
    T50NS = 3, //!< 50ns   / 20MHz
};

/*!
 * \brief Protocol relevant global cluster parameters
 *
 *  Cf. 'FlexRay Protocol Specification Version 3.0.1' Appendix B.3.1.1 Parameters
 */
struct ClusterParameters
{
    //! Number of attempts for a cold start before giving up (range 2 -31)
    uint8_t gColdstartAttempts;

    //! Max number for cycle count (number of cycles - 1)
    uint8_t gCycleCountMax;

    //! time offset for a static slot in MacroTicks (MT) (range 1-63)
    uint16_t gdActionPointOffset;

    ////! Not used by Network Simulator
    //gdCASRxLowMax   

    //! Duration of the idle phase within a dynamic slot in gdMiniSlots (range 0 - 2)
    uint16_t gdDynamicSlotIdlePhase;

    ////! Not used by Network Simulator
    //gdIgnoreAfterTx

    //! Duration of a mini slot in MacroTicks (MT) (2-63)
    uint16_t gdMiniSlot;

    //! Time offset for a mini slot in MacroTicks (MT) (range 1- 31)
    uint16_t gdMiniSlotActionPointOffset;

    //! Duration of a static slot in MacroTicks (MT) (3-664)
    uint16_t gdStaticSlot;

    //! Duration of the symbol window in MacroTicks (MT) (range1 - 139)
    uint16_t gdSymbolWindow;

    //! Time offset for a static symbol windows in MacroTicks (MT) (range 1-63)
    uint16_t gdSymbolWindowActionPointOffset;

    //! Duration of TSS (Transmission Start Sequence) in gdBits (range 1-15)
    uint16_t gdTSSTransmitter;

    ////! Not used by Network Simulator
    //gdWakeupRxIdle

    ////! Not used by Network Simulator
    //gdWakeupRxLow

    ////! Not used by Network Simulator
    //gdWakeupRxWindow

    //! Duration of LOW Phase of a wakeup symbol in gdBit (range 15 - 60)
    uint16_t gdWakeupTxActive;

    //! Duration of the idle of a wakeup symbol in gdBit (45 - 180)
    uint16_t gdWakeupTxIdle;

    /*!
     * Upper limit for the startup listen timeout and wakeup listen timeout in the
     * presence of noise. Used as a multiplier of pdListenTimeout. (range 2 - 16)
     */
    uint8_t gListenNoise;

    //! Number of MacroTicks (MT) per cycle, (range 8 - 16000)
    uint16_t gMacroPerCycle;

    //! Threshold used for testing the vClockCorrectionFailed counter (range 1-15)
    uint8_t gMaxWithoutClockCorrectionFatal;

    //! Threshold used for testing the vClockCorrectionFailed counter (range 1-15)
    uint8_t gMaxWithoutClockCorrectionPassive;

    //! Number of mini slots (range 0-7988)
    uint16_t gNumberOfMiniSlots;

    //! Number of static slots in a cycle (range 2-1023)
    uint16_t gNumberOfStaticSlots;

    //! Length of the payload of a static frame in 16-Bits words (range 0 - 127)
    uint16_t gPayloadLengthStatic;

    //! Max number of distinct sync frame identifiers present in a given cluster. (range 2 - 15)
    uint8_t gSyncFrameIDCountMax;
};

/*!
 * \brief Protocol relevant global node parameters
 *
 *  Cf. 'FlexRay Protocol Specification Version 3.0.1' Appendix B.3.2 Parameters
 */
struct NodeParameters
{
    // ----------------------------------------------------------------------
    // Parameters according to B.3.2.1

    //! Controls the transition to halt state due to clock synchronization errors. (0,1)
    uint8_t pAllowHaltDueToClock;

    //! Required number of consecutive even / odd cycle pairs for normal passive to normal active (range 0-31)
    uint8_t pAllowPassiveToActive;

    //! Channel(s) to which the controller is connected (values Channel::A, Channel::B, Channel::AB)
    Channel pChannels;

    //! Cluster drift damping factor for rate correction in Microticks (range 0 - 10)
    uint8_t pClusterDriftDamping;

    //! Allowed deviation for startup frames during integration in MicroTicks (range 29 - 2743 �T)
    FrMickroTick pdAcceptedStartupRange;

    ////! Not used by Network Simulator
    //pDecodingCorrection

    ////! Not used by Network Simulator
    //pDelayCompensationA

    ////! Not used by Network Simulator
    //pDelayCompensationB

    //! Duration of listen phase in MicroTicks (range 1926 - 2567692)
    FrMickroTick pdListenTimeout;

    ////! Not used by Network Simulator
    //pExternalSync

    ////! Not used by Network Simulator
    //pExternOffsetCorrection

    ////! Not used by Network Simulator
    //pExternRateCorrection

    ////! Not used by Network Simulator
    //pFallBackInternal

    //! Slot ID of the key slot (range 0-1023, value 0 means that there is no key slot)
    uint16_t pKeySlotId;

    //! Shall the node enter key slot only mode after startup. (values 0, 1) (AUTOSAR pSingleSlotEnabled)
    uint8_t pKeySlotOnlyEnabled;

    //! Key slot is used for startup (range 0, 1)
    uint8_t pKeySlotUsedForStartup;

    //! Key slot is used for sync (range 0, 1)
    uint8_t pKeySlotUsedForSync;

    //! Last mini slot which can be transmitted (range 0 -7981)
    uint16_t pLatestTx;

    //! Initial startup offset for frame reference point on channel A (rang 2-68 MacroTicks (MT))
    uint8_t pMacroInitialOffsetA;

    //! Initial startup offset for frame reference point on channel B (rang 2-68 MacroTicks (MT))
    uint8_t pMacroInitialOffsetB;

    //! Offset between secondary time reference and MT boundary (range 0 - 239 Microticks)
    FrMickroTick pMicroInitialOffsetA;

    //! Offset between secondary time reference and MT boundary (range 0 - 239 Microticks)
    FrMickroTick pMicroInitialOffsetB;

    //! Nominal number of microticks in the communication cycle (range 960 - 1280000)
    FrMickroTick pMicroPerCycle;

    //! Maximum permissible offset correction value (range 15 - 16082 Microticks)
    FrMickroTick pOffsetCorrectionOut;

    //! Start of the offset correction phase within the NIT, (7 - 15999 MT)
    uint16_t pOffsetCorrectionStart;

    //! Maximum permissible rate correction value (range 3 - 3846 Microticks)
    FrMickroTick pRateCorrectionOut;

    ////! Not used by Network Simulator
    //pSecondKeySlotID

    ////! Not used by Network Simulator
    //pTwoKeySlotMode

    //! Channel used by the node to send a wakeup pattern (values Channel::A, Channel::B)
    Channel pWakeupChannel;

    //! Number of repetitions of the wakeup symbol (range 0-63, value 0 prevents sending of WUP)
    uint8_t pWakeupPattern; 

    // ----------------------------------------------------------------------
    // Parameters according to B.3.2.2

    //! Duration of a FlexRay microtick (12.5ns, 25ns or 50ns)
    ClockPeriod pdMicrotick;

    ////! Not used by Network Simulator
    //pNMVectorEarlyUpdate

    ////! Not used by Network Simulator
    //pPayloadLengthDynMax

    //! Number of samples per microtick (values 1 or 2)
    uint8_t pSamplesPerMicrotick;
};

//! Transmission mode for FlexRay Tx-Buffer
enum class TransmissionMode : uint8_t
{
    SingleShot = 0,
    Continuous = 1
};

//! Configuration of Tx-Buffer, used in struct ControllerConfig
struct TxBufferConfig
{
    //! (values Channel::A, Channel::B, Channel::AB)
    Channel channels;

    //! The slot Id of frame
    uint16_t slotId;

    //! Base offset for cycle multiplexing (values 0-63)
    uint8_t offset;

    //! Repetition for cycle multiplexing (values 1,2,4,8,16,32,64)
    uint8_t repetition;

    //! Set the PPindicator
    bool hasPayloadPreambleIndicator;

    //! Header CRC, 11 bits
    uint16_t headerCrc;

    //! TransmissionMode::SingleShot or TransmissionMode::Continuous
    TransmissionMode transmissionMode;
};

//! Configure the communication parameters of the FlexRay controller.
struct ControllerConfig
{
    ClusterParameters clusterParams;
    NodeParameters nodeParams;

    std::vector<TxBufferConfig> bufferConfigs;
};

//! Update the content of a FlexRay TX-Buffer
struct TxBufferUpdate
{
    //! Index of the TX Buffers according to the configured buffers (cf. ControllerConfig)
    uint16_t txBufferIndex;

    //! Payload data valid flag
    bool payloadDataValid;

    //! Raw payload containing 0 to 254 bytes.
    std::vector<uint8_t> payload;
};

enum class ChiCommand : uint8_t
{
    RUN,
    DEFERRED_HALT,
    FREEZE,
    ALLOW_COLDSTART,
    ALL_SLOTS,
    WAKEUP
};

struct HostCommand
{
    ChiCommand command;
};

struct Header
{
    /*!
     * \brief Flags bit map according to FlagMask
     * 
     * Description:
     * [7-5]: unused
     * [4]: Reserved bit
	 * [3]: PPIndicator: 0, regular payload; 1, NM vector or message ID
	 * [2]: NFIndicator: 0, no valid payload data and PPIndicator = 0; 1, valid payload data
	 * [1]: SyFIndicator: 0, frame not used for synchronization; 1, frame shall be used for sync
	 * [0]: SuFIndicator: 0, not a startup frame; 1, a startup frame
	 */
    uint8_t flags = 0;
    uint16_t frameId = 0; //!< Slot ID in which the frame was sent: 1 - 2047
    uint8_t payloadLength = 0; //!< Payload length, 7 bits
    uint16_t headerCrc = 0; //!< Header CRC, 11 bits
    uint8_t cycleCount = 0; //!< Cycle in which the frame was sent: 0 - 63


    // --------------------------------------------------------------------------------
    //  Convenience Accessors to Header Flags
    // --------------------------------------------------------------------------------

    //! Flag BitMask definition for helper methods
    enum class Flag : uint8_t
    {
        SuFIndicator = 1 << 0,
        SyFIndicator = 1 << 1,
        NFIndicator = 1 << 2,
        PPIndicator = 1 << 3
    };

    //! Convenience helper to check if a Flag is set
    inline bool IsSet(Flag flag) const;

    //! Convenience helper to set a Flag
    inline void Set(Flag flag);

    //! Convenience helper to clear a Flag
    inline void Clear(Flag flag);

    //! Convenience helper to set or clear a Flag according to a condition
    inline void Set(Flag flag, bool condition);
};

struct Frame
{
    Header header; //!< Header flags, slot, crc, and cycle indidcators
    std::vector<uint8_t> payload; //!< Raw payload containing 0 to 254 bytes
};

// Receive a frame from the Bus.
// Direction: from Network Simulator to simulated ecu
struct FrMessage
{
    // Time at end of frame transmission
    std::chrono::nanoseconds timestamp;

    // FlexRay channel A or B. (Valid values: Channel::A, Channel::B)
    Channel channel;

    Frame frame;
};

/*!
 * \brief Acknowledge for the transmit on the FlexRay bus
 * 
 * Directions:
 * - From: Network Simulator  To: Simulated ECU
 */
struct FrMessageAck
{
    std::chrono::nanoseconds timestamp; //!< Time at end of frame transmission
    uint16_t txBufferIndex; //!< Tx buffer, that was used for the transmission
    Channel channel; //!< FlexRay channel A or B. (Valid values: Channel::A, Channel::B)
    Frame frame; //!< Copy of the FlexRay frame that was successfully transmitted
};

/*!
 * \brief FlexRay symbols patterns.
 */
enum class SymbolPattern : uint8_t
{
    CasMts, //!< Collision avoidance symbol (CAS) OR media access test symbol (MTS)
    Wus,    //!< Wakeup symbol (WUS)
    Wudop   //!< Wakeup During Operation Pattern (WUDOP)
};

/*!
 * \brief A FlexRay Symbol as received on the FlexRay bus.
 */
struct FrSymbol
{
    std::chrono::nanoseconds timestamp; //!< End time of symbol reception.
    Channel channel; //!< FlexRay channel A or B (values: Channel::A, Channel::B)
    SymbolPattern pattern; //!< The received symbol, e.g. wakeup pattern
};

struct FrSymbolAck : FrSymbol
{
};

/*!
 * \brief Protocol Operation Control (POC) state of the FlexRay communication controller
 */
enum class PocState : uint8_t
{
    DefaultConfig = 0, //!< CC expects configuration. Initial state after reset. 
    Config        = 1, //!< CC is in configuration Mode for setting communication parameters
    Ready         = 2, //!< intermediate state for initialization process (after Config)
    Startup       = 3, //!< FlexRay startup phase
    Wakeup        = 4, //!< FlexRay wakeup phase
    NormalActive  = 5, //!< Normal operating mode
    NormalPassive = 6, //!< Operating mode with transient or tolerable errors
    Halt          = 7  //!< CC is halted, (caused by the application (DeferredHalt) or by a fatal error)
};

/*!
 * \brief Status of the simulated FlexRay controller
 */
struct ControllerStatus
{
    std::chrono::nanoseconds timestamp;
    PocState pocState; //!< Status of the Protocol Operation Control (POC)
};


// ================================================================================
//  Inline Implementations
// ================================================================================
bool Header::IsSet(Flag flag) const
{
    return flags & static_cast<uint8_t>(flag);
}

void Header::Set(Flag flag)
{
    flags |= static_cast<uint8_t>(flag);
}

void Header::Clear(Flag flag)
{
    flags &= ~static_cast<uint8_t>(flag);
}

void Header::Set(Flag flag, bool condition)
{
    if (condition)
        Set(flag);
    else
        Clear(flag);
}

} // namespace fr
} // namespace sim
} // namespace ib
