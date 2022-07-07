// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <array>
#include <chrono>
#include <vector>

#include "silkit/services/datatypes.hpp"

namespace SilKit {
namespace Services {
namespace Lin {

/*! \brief The identifier of a LIN \ref LinFrame
 *
 * This type represents all valid identifier used by ILinController::SendFrame(), ILinController::SendFrameHeader().
 *
 * *Range:* 0...0x3F
 */
using LinIdT = uint8_t;

/*! \brief The checksum model of a LIN \ref LinFrame
 *
 * This type is used to specify the Checksum model to be used for the LIN \ref LinFrame.
 *
 * *AUTOSAR Name:* Lin_FrameCsModelType 
 */
enum class LinChecksumModel : uint8_t
{
    Undefined = 0,//!< Undefined / unconfigured checksum model
    Enhanced = 1, //!< Enhanced checksum model
    Classic = 2   //!< Classic checksum model 
};

/*! \brief The data length of a LIN \ref LinFrame in bytes
 *
 * This type is used to specify the number of data bytes to copy.
 *
 * *AUTOSAR Name:* Lin_FrameDlType
 *
 * *Range:* 1...8
 */
using LinDataLengthT = uint8_t;

/*! \brief A LIN LinFrame
*
* This Type is used to provide LIN ID, checksum model, data length and data.
*
* *AUTOSAR Name:* Lin_PduType
*/
struct LinFrame
{
    LinIdT                 id{0}; //!< Lin Identifier
    LinChecksumModel          checksumModel{LinChecksumModel::Undefined}; //!< Checksum Model
    LinDataLengthT            dataLength{0}; //!< Data length
    std::array<uint8_t, 8> data{}; //!< The actual payload
};

//! \brief Create a LinFrame that corresponds to a Go-To-Sleep command
inline auto GoToSleepFrame() -> LinFrame;


/*! \brief Controls the behavior of ILinController::SendFrame()
 *
 * Determines whether the master also provides a frame response or if the frame
 * response is expected to be provided from a slave.
 *
 * *AUTOSAR Name:* Lin_FrameResponseType
 */ 
enum class LinFrameResponseType : uint8_t
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

//! \brief Controls the behavior of a LIN Slave task for a particular LIN ID
enum class LinFrameResponseMode : uint8_t
{
    //! The LinFrameResponse corresponding to the ID is neither received nor
    //! transmitted by the LIN slave.
    Unused = 0,

    //! The LinFrameResponse corresponding to the ID is received by the LIN slave.
    Rx = 1,

    //! The LinFrameResponse corresponding to the ID is transmitted unconditionally
    //! by the LIN slave.
    TxUnconditional = 2
};

/*! \brief Configuration data for a LIN Slave task for a particular LIN ID.
 */
struct LinFrameResponse
{
    /*! frame must provide the LIN \ref LinIdT for which the response is
     *  configured.
     *
     * If responseMode is LinFrameResponseMode::TxUnconditional, the
     * frame data is used for the transaction.
     */
    LinFrame frame;
    //! Determines if the LinFrameResponse is used for transmission
    //! (TxUnconditional), reception (Rx) or ignored (Unused).
    LinFrameResponseMode responseMode{LinFrameResponseMode::Unused};
};

/*! \brief The state of a LIN transmission
 *
 * Used to indicate the success or failure of a LIN transmission to a
 * registered \ref ILinController::FrameStatusHandler.
 *
 * *Note:* the enumeration values directly correspond to the AUTOSAR
 *  type Lin_StatusType. Not all values are used in the SIL Kit.
 *
 * *AUTOSAR Doc:* LIN operation states for a LIN channel or frame, as
 * returned by the API service Lin_GetStatus().
 *
 */
enum class LinFrameStatus : uint8_t
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
 *  Cf. \ref LinControllerConfig, \ref ILinController::Init()
 */
enum class LinControllerMode : uint8_t
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
using LinBaudRateT = uint32_t;


/*! Configuration data to initialize the LIN Controller
 *  Cf.: \ref ILinController::Init(LinControllerConfig config);
 */
struct LinControllerConfig
{
    //! Configure as LIN master or LIN slave
    LinControllerMode controllerMode{LinControllerMode::Inactive};
    /*! The operational baud rate of the controller. Used in a detailed simulation.
     */
    LinBaudRateT baudRate{0};
    /*! Optional LinFrameResponse configuration.
     *  
     * FrameResponses can also be configured at a later point using
     * ILinController::SetFrameResponse() and
     * ILinController::SetFrameResponses().
     */
    std::vector<LinFrameResponse> frameResponses; 
};

/*! The operational state of the controller, i.e., operational or
 *  sleeping.
 */
enum class LinControllerStatus
{
    //! The controller state is not yet known.
    Unknown = 0,

    //! Normal operation
    Operational = 1,

    //! Sleep state operation; in this state wake-up detection from slave nodes
    //  is enabled.
    Sleep = 2,

    //! Sleep Pending state is reached when a GoToSleep is issued.
    //  This allows the network simulator to finish pending transmissions (e.g., sleep frames to slaves)
    //  before entering state Sleep, cf. AUTOSAR SWS LINDriver [SWS_LIN_00266] and section 7.3.3.
    //  This is only used when using detailed simulations with a network simulator.
    SleepPending = 3,
};


//! \brief A LIN frame status event delivered in the \ref ILinController::FrameStatusHandler.
struct LinFrameStatusEvent
{
    std::chrono::nanoseconds timestamp; //!< Time of the event.
    LinFrame frame;
    LinFrameStatus status;
};

//! \brief A LIN wakeup event delivered in the \ref ILinController::WakeupHandler.
struct LinWakeupEvent
{
    std::chrono::nanoseconds timestamp; //!< Time of the event.
    TransmitDirection direction; //!< The direction of the wakeup pulse.
};

//! \brief A LIN wakeup event delivered in the \ref ILinController::GoToSleepHandler.
struct LinGoToSleepEvent
{
    std::chrono::nanoseconds timestamp; //!< Time of the event.
};

/*! \brief A LIN frame response update event delivered in the \ref ILinController::FrameResponseUpdateHandler
*
* The event is received for every LinFrameResponse whenever a LinControllerConfig is received or a controller calls
* \ref ILinController::SetFrameResponses. This event is mainly for diagnostic purposes and contains no timestamp.
* 
*/
struct LinFrameResponseUpdateEvent
{
    std::string senderID; //!< String identifier of the controller providing the update.
    LinFrameResponse frameResponse; //!< The frameResponse of the update.
};

// ================================================================================
//  Messages used at the Participant Interface
// ================================================================================
//! \brief Data type representing a finished LIN transmission, independent of success or error.
struct LinTransmission
{
    std::chrono::nanoseconds timestamp; //!< Time at the end of the transmission. Only valid in a detailed simulation.
    LinFrame frame;                        //!< The transmitted frame
    LinFrameStatus status;                 //!< The status of the transmitted frame
};

/*! \brief Data type representing a request to perform an AUTOSAR SendFrame operation.
 *
 * Used internally.
 */
struct LinSendFrameRequest
{
    LinFrame frame;                    //!< Provide the LIN ID, checksum model, expected data length and optional data.
    LinFrameResponseType responseType; //!< Determines whether to provide a frame response or not.
};

/*! \brief Data type representing a request to perform an non-AUTOSAR send operation.
*
* Used internally.
*/
struct LinSendFrameHeaderRequest
{
    LinIdT id; //!< The LinIdT of the header to be transmitted
};

//! \brief Data type used to inform other LIN participants about changed LinFrameResponse data.
struct LinFrameResponseUpdate
{
    std::vector<LinFrameResponse> frameResponses; //!< Vector of new FrameResponses.
};

//! \brief Data type used to inform other LIN participants about changed LinControllerStatus.
struct LinControllerStatusUpdate
{
    std::chrono::nanoseconds timestamp; //!< Time of the controller status change.
    LinControllerStatus status;            //!< The new controller status
};

//! \brief Data type representing a LIN WakeUp pulse.
struct LinWakeupPulse
{
    std::chrono::nanoseconds timestamp; //!< Time of the WakeUp pulse. Only valid in a detailed simulation.
    TransmitDirection direction; //!< The direction of the wakeup pulse.
};


// ================================================================================
//  Inline Implementations
// ================================================================================
//! \brief Factory method for a LinFrame representing a Go-To-Sleep signal
inline auto GoToSleepFrame() -> LinFrame
{
    LinFrame frame;
    frame.id = 0x3c;
    frame.checksumModel = LinChecksumModel::Classic;
    frame.dataLength = 8;
    frame.data = {0x0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    return frame;
}
//! \brief operator== for LinFrame
inline bool operator==(const LinFrame& lhs, const LinFrame& rhs)
{
    return lhs.id == rhs.id
        && lhs.checksumModel == rhs.checksumModel
        && lhs.dataLength == rhs.dataLength
        && lhs.data == rhs.data;
}
//! \brief operator== for LinSendFrameRequest
inline bool operator==(const LinSendFrameRequest& lhs, const LinSendFrameRequest& rhs)
{
    return lhs.frame == rhs.frame
        && lhs.responseType == rhs.responseType;
}
//! \brief operator== for LinSendFrameHeaderRequest
inline bool operator==(const LinSendFrameHeaderRequest& lhs, const LinSendFrameHeaderRequest& rhs)
{
    return lhs.id == rhs.id;
}
//! \brief operator== for LinTransmission
inline bool operator==(const LinTransmission& lhs, const LinTransmission& rhs)
{
    return lhs.timestamp == rhs.timestamp
        && lhs.frame == rhs.frame
        && lhs.status == rhs.status;
}
//! \brief operator== for LinWakeupPulse
inline bool operator==(const LinWakeupPulse& lhs, const LinWakeupPulse& rhs)
{
    return lhs.timestamp == rhs.timestamp;
}
//! \brief operator== for LinFrameResponse
inline bool operator==(const LinFrameResponse& lhs, const LinFrameResponse& rhs)
{
    return lhs.frame == rhs.frame
        && lhs.responseMode == rhs.responseMode;
}
//! \brief operator== for LinControllerConfig
inline bool operator==(const LinControllerConfig& lhs, const LinControllerConfig& rhs)
{
    return lhs.controllerMode == rhs.controllerMode
        && lhs.baudRate == rhs.baudRate
        && lhs.frameResponses == rhs.frameResponses;
}
//! \brief operator== for LinControllerStatusUpdate
inline bool operator==(const LinControllerStatusUpdate& lhs, const LinControllerStatusUpdate& rhs)
{
    return lhs.timestamp == rhs.timestamp
        && lhs.status == rhs.status;
}
//! \brief operator== for LinFrameResponseUpdate
inline bool operator==(const LinFrameResponseUpdate& lhs, const LinFrameResponseUpdate& rhs)
{
    return lhs.frameResponses == rhs.frameResponses;
}

} // namespace Lin
} // namespace Services
} // namespace SilKit
