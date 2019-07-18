// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <array>
#include <chrono>
#include <vector>

#include "ib/sim/datatypes.hpp"

namespace ib {
namespace sim {
//! The LIN namespace
namespace lin {

/*! \brief The LIN identifier of a \ref Frame
 *
 * This type represents all valid identifier used by ILinController::SendFrame(), ILinController::SendFrameHeader().
 *
 * *Range:* 0...0x3F
 */
using LinIdT = uint8_t;

/*! \brief The checksum model of a LIN \ref Frame
 *
 * This type is used to specify the Checksum model to be used for the LIN \ref Frame.
 *
 * *AUTOSAR Name:* Lin_FrameCsModelType 
 */
enum class ChecksumModel : uint8_t
{
    Undefined = 0,//!< Undefined / unconfigured checksum model
    Enhanced = 1, //!< Enhanced checksum model
    Classic = 2   //!< Classic checksum model 
};

/*! \brief The data length of a LIN \ref Frame in bytes
 *
 * This type is used to specify the number of data bytes to copy.
 *
 * *AUTOSAR Name:* Lin_FrameDlType
 *
 * *Range:* 1...8
 */
using DataLengthT = uint8_t;

struct Frame
{
    LinIdT                 id{0}; //!< Lin Identifier
    ChecksumModel          checksumModel{ChecksumModel::Undefined}; //!< Checksum Model
    DataLengthT            dataLength{0}; //!< Data length
    std::array<uint8_t, 8> data{}; //!< The actual payload
};

//! \brief Create a Frame that corresponds to a Go-To-Sleep command
inline auto GoToSleepFrame() -> Frame;


/*! \brief Controls the behavior of \ref ILinController::SendFrame()
 *
 * Determines whether the master also provides a frame response or if the frame
 * response is expected to be provided from a slave.
 *
 * *AUTOSAR Name:* Lin_FrameResponseType
 */ 
enum class FrameResponseType : uint8_t
{
    //! Response is generated from this (master) node 
    MasterResponse = 0,
    
    //! Response is generated from a remote slave node
    SlaveResponse = 1,
    
    /*! Response is generated from one slave to and received by
     *  another slave, for the master the response will be anonymous,
     *  it does not have to receive the response.
     */
    SlaveToSlave = 2
};

enum class FrameResponseMode : uint8_t
{
    //! The FrameResponse corresponding to the ID is neither received nor
    //! transmitted by the LIN slave.
    Unused = 0,

    //! The FrameResponse corresponding to the ID is received by the LIN slave.
    Rx = 1,

    //! The FrameResponse corresponding to the ID is transmitted unconditionally
    //! by the LIN slave.
    TxUnconditional = 2
};

struct FrameResponse
{
    /*! frame must provide the LIN \ref LinIdT for which the response is
     *  configured.
     *
     * If responseMode is FrameResponseMode::TxUnconditional, the
     * frame data is used for the transaction.
     */
    Frame frame;
    //! Determines if the FrameResponse is used for transmission
    //! (TxUnconditional), reception (Rx) or ignored (Unused).
    FrameResponseMode responseMode{FrameResponseMode::Unused};
};

/*! \brief The state of a LIN transmission
 *
 * Used to indicate the success or failure of a LIN transmission to a
 * registered \ref ILinController::FrameStatusHandler.
 *
 * *Note:* the enumeration values directly correspond to the AUTOSAR
 *  type Lin_StatusType. Not all values are used in the Integration
 *  Bus.
 *
 * *AUTOSAR Doc:* LIN operation states for a LIN channel or frame, as
 * returned by the API service Lin_GetStatus().
 *
 */
enum class FrameStatus : uint8_t
{
    /*! (currently not in use)
     *
     * *AUROSAR Doc:* Development or production error occurred
     */
    NOT_OK = 0,

    /*! The controller successfully transmitted a frame response.
     *
     * *AUTOSAR Doc:* Successful transmission.
     */
    LIN_TX_OK,

    /*! (currently not in use)
     *
     * *AUTOSAR Doc:* Ongoing transmission (Header or Response). 
     */
    LIN_TX_BUSY,

    /*! (currently not in use)
     *
     * *AUTOSAR Doc:*
     * Erroneous header transmission such as: 
     *  Mismatch between sent and read back data, 
     *  Identifier parity error, or 
     *  Physical bus error 
     */
    LIN_TX_HEADER_ERROR,

    /*! (currently not in use)
     *
     * *AUTOSAR Doc:*
     * Erroneous response transmission such as: 
     *  Mismatch between sent and read back data,
     *  Physical bus error 
     */
    LIN_TX_ERROR,

    /*! The controller received a correct frame response.
     *
     * *AUTOSAR Doc:* Reception of correct response.
     */
    LIN_RX_OK,

    /*! (currently not in use)
     * 
     * *AUTOSAR Doc:* Ongoing reception: at least one response byte has been
     *  received, but the checksum byte has not been received.
     */
    LIN_RX_BUSY,

    /*! The reception of a response failed.
     *
     * Indicates a mismatch in expected and received data length or a checksum
     * error. Checksum errors occur when multiple slaves are configured to
     * transmit the same frame or when the sender and receiver use different
     * checksum models.
     *
     * *AUTOSAR Doc:*
     * Erroneous response reception such as: 
     *  Framing error,
     *  Overrun error,
     *  Checksum error, or
     *  Short response 
     */
    LIN_RX_ERROR,

    /*! No LIN controller did provide a response to the frame header.
     *
     * *AUTOSAR Doc:* No response byte has been received so far. 
     */
    LIN_RX_NO_RESPONSE
};

/*! Used to configure a LIN controller as a master or slave.
 *
 *  Cf. \ref ControllerConfig, \ref ILinController::Init()
 */
enum class ControllerMode : uint8_t
{
    /*! The LIN controller has not been configured yet and is
     *  inactive. This does not indicate sleep mode.
     */
    Inactive = 0,
    //! A LIN controller with active master task and slave task
    Master = 1,
    //! A LIN controller with only a slave task
    Slave = 2
};

/*! The operational baud rate of the controller.
 *
 * *Range:* 200...20'000 Bd
 */
using BaudRateT = uint32_t;


/*! Configuration data to initialize the LIN Controller
 *  Cf.: \ref ILinController::Init(ControllerConfig config);
 */
struct ControllerConfig
{
    //! Configure as LIN master or LIN slave
    ControllerMode controllerMode{ControllerMode::Inactive};
    /*! The operational baud rate of the controller. Only relevant for VIBE
     * simulation.
     */
    BaudRateT baudRate{0};
    /*! Optional FrameResponse configuration.
     *  
     * FrameResponses can also be configured at a later point using
     * ILinController::SetFrameResponse() and
     * ILinController::SetFrameResponses().
     */
    std::vector<FrameResponse> frameResponses; 
};

/*! The operational state of the controller, i.e., operational or
 *  sleeping.
 */
enum class ControllerStatus
{
    //! The controller state is not yet known.
    Unknown = 0,
    
    //! Normal operation
    Operational = 1,

    //! Sleep state operation; in this state wake-up detection from slave nodes
    //  is enabled.
    Sleep = 2
};


// ================================================================================
//  Messages used at the ComAdapter Interface
// ================================================================================
struct Transmission
{
    std::chrono::nanoseconds timestamp;
    Frame frame;
    FrameStatus status;
};

struct SendFrameRequest
{
    Frame frame;
    FrameResponseType responseType;
};

struct SendFrameHeaderRequest
{
    LinIdT id;
};

struct FrameResponseUpdate
{
    std::vector<FrameResponse> frameResponses;
};

struct ControllerStatusUpdate
{
    std::chrono::nanoseconds timestamp;
    ControllerStatus status;
};

struct WakeupPulse
{
    std::chrono::nanoseconds timestamp;
};


// ================================================================================
//  Inline Implementations
// ================================================================================
inline auto GoToSleepFrame() -> Frame
{
    Frame frame;
    frame.id = 0x3c;
    frame.checksumModel = ChecksumModel::Classic;
    frame.dataLength = 8;
    frame.data = {0x0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    return frame;
}

inline bool operator==(const Frame& lhs, const Frame& rhs)
{
    return lhs.id == rhs.id
        && lhs.checksumModel == rhs.checksumModel
        && lhs.dataLength == rhs.dataLength
        && lhs.data == rhs.data;
}

inline bool operator==(const SendFrameRequest& lhs, const SendFrameRequest& rhs)
{
    return lhs.frame == rhs.frame
        && lhs.responseType == rhs.responseType;
}
inline bool operator==(const SendFrameHeaderRequest& lhs, const SendFrameHeaderRequest& rhs)
{
    return lhs.id == rhs.id;
}

inline bool operator==(const Transmission& lhs, const Transmission& rhs)
{
    return lhs.timestamp == rhs.timestamp
        && lhs.frame == rhs.frame
        && lhs.status == rhs.status;
}
inline bool operator==(const WakeupPulse& lhs, const WakeupPulse& rhs)
{
    return lhs.timestamp == rhs.timestamp;
}
inline bool operator==(const FrameResponse& lhs, const FrameResponse& rhs)
{
    return lhs.frame == rhs.frame
        && lhs.responseMode == rhs.responseMode;
}
inline bool operator==(const ControllerConfig& lhs, const ControllerConfig& rhs)
{
    return lhs.controllerMode == rhs.controllerMode
        && lhs.baudRate == rhs.baudRate
        && lhs.frameResponses == rhs.frameResponses;
}
inline bool operator==(const ControllerStatusUpdate& lhs, const ControllerStatusUpdate& rhs)
{
    return lhs.timestamp == rhs.timestamp
        && lhs.status == rhs.status;
}
inline bool operator==(const FrameResponseUpdate& lhs, const FrameResponseUpdate& rhs)
{
    return lhs.frameResponses == rhs.frameResponses;
}

} // namespace lin
} // namespace sim
} // namespace ib
