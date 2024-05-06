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

#include <array>
#include <chrono>
#include <vector>

#include "silkit/services/datatypes.hpp"
#include "silkit/util/Span.hpp"

#include "silkit/capi/Lin.h"

namespace SilKit {
namespace Services {
namespace Lin {

/*! \brief The identifier of a LIN \ref LinFrame
 *
 * This type represents all valid identifier used by ILinController::SendFrame(), ILinController::SendFrameHeader().
 *
 * *Range:* 0...0x3F
 */
using LinId = SilKit_LinId;

/*! \brief The checksum model of a LIN \ref SilKit::Services::Lin::LinFrame
 *
 * This type is used to specify the Checksum model to be used for the LIN \ref LinFrame.
 *
 * *AUTOSAR Name:* Lin_FrameCsModelType 
 */
enum class LinChecksumModel : SilKit_LinChecksumModel
{
    //! Unknown checksum model. If configured with this value, the checksum model of the first reception will be used.
    Unknown = SilKit_LinChecksumModel_Unknown,
    //! Enhanced checksum model
    Enhanced = SilKit_LinChecksumModel_Enhanced,
    //! Classic checksum model
    Classic = SilKit_LinChecksumModel_Classic,
};

/*! \brief The data length of a LIN \ref LinFrame in bytes
 *
 * This type is used to specify the number of data bytes to copy.
 *
 * *AUTOSAR Name:* Lin_FrameDlType
 *
 * *Range:* 0...8
 */
using LinDataLength = SilKit_LinDataLength;

//! \brief If configured with this value, the data length of the first reception will be used.
const LinDataLength LinDataLengthUnknown = SilKit_LinDataLengthUnknown;

/*! \brief A LIN LinFrame
 *
 * This Type is used to provide LIN ID, checksum model, data length and data.
 *
 * *AUTOSAR Name:* Lin_PduType
 */
struct LinFrame
{
    LinId id{0}; //!< Lin Identifier
    LinChecksumModel checksumModel{LinChecksumModel::Unknown}; //!< Checksum Model
    LinDataLength dataLength{0}; //!< Data length
    std::array<uint8_t, 8> data{}; //!< The actual payload
};

//! \brief Create a LinFrame that corresponds to a Go-To-Sleep command
inline auto GoToSleepFrame() -> LinFrame;


/*! \brief Controls the behavior of ILinController::SendFrame()
 *
 * Determines whether the master also provides a frame response or if the frame response is expected to be provided 
 * from a slave.
 *
 * *AUTOSAR Name:* Lin_FrameResponseType
 */ 
enum class LinFrameResponseType : SilKit_LinFrameResponseType
{
    //! Response is generated from this master node 
    MasterResponse = SilKit_LinFrameResponseType_MasterResponse,
    
    //! Response is generated from a remote slave node
    SlaveResponse = SilKit_LinFrameResponseType_SlaveResponse,
    
    /*! Response is generated from one slave to and received by
     *  another slave, for the master the response will be anonymous,
     *  it does not have to receive the response.
     */
    SlaveToSlave = SilKit_LinFrameResponseType_SlaveToSlave
};

//! \brief Controls the behavior of a LIN Slave task for a particular LIN ID
enum class LinFrameResponseMode : SilKit_LinFrameResponseMode
{
    //! The LinFrameResponse corresponding to the ID is neither received nor
    //! transmitted by the LIN slave.
    Unused = SilKit_LinFrameResponseMode_Unused,

    //! The LinFrameResponse corresponding to the ID is received by the LIN slave.
    Rx = SilKit_LinFrameResponseMode_Rx,

    //! The LinFrameResponse corresponding to the ID is transmitted unconditionally
    //! by the LIN slave.
    TxUnconditional = SilKit_LinFrameResponseMode_TxUnconditional,
};

/*! \brief Configuration data for a LIN Slave task for a particular LIN ID.
 */
struct LinFrameResponse
{
    /*! frame must provide the LIN \ref LinId for which the response is configured.
     *
     * If responseMode is LinFrameResponseMode::TxUnconditional, the frame data is used for the transaction.
     */
    LinFrame frame;
    //! Determines if the LinFrameResponse is used for transmission (TxUnconditional), reception (Rx) or 
    //! ignored (Unused).
    LinFrameResponseMode responseMode{LinFrameResponseMode::Unused};
};

/*! \brief The state of a LIN transmission
 *
 * Used to indicate the success or failure of a LIN transmission to a registered \ref ILinController::FrameStatusHandler.
 *
 * *Note:* the enumeration values directly correspond to the AUTOSAR type Lin_StatusType. Not all values are used in 
 * the SIL Kit.
 *
 * *AUTOSAR Doc:* LIN operation states for a LIN channel or frame, as returned by the API service Lin_GetStatus().
 *
 */
enum class LinFrameStatus : SilKit_LinFrameStatus
{
    /*! (currently not in use)
     *
     * *AUROSAR Doc:* Development or production error occurred
     */
    NOT_OK = SilKit_LinFrameStatus_NOT_OK,

    /*! The controller successfully transmitted a frame response.
     *
     * *AUTOSAR Doc:* Successful transmission.
     */
    LIN_TX_OK = SilKit_LinFrameStatus_LIN_TX_OK,

    /*! (currently not in use)
     *
     * *AUTOSAR Doc:* Ongoing transmission (Header or Response). 
     */
    LIN_TX_BUSY = SilKit_LinFrameStatus_LIN_TX_BUSY,

    /*! (currently not in use)
     *
     * *AUTOSAR Doc:*
     * Erroneous header transmission such as: 
     *  Mismatch between sent and read back data, 
     *  Identifier parity error, or 
     *  Physical bus error 
     */
    LIN_TX_HEADER_ERROR = SilKit_LinFrameStatus_LIN_TX_HEADER_ERROR,

    /*! (currently not in use)
     *
     * *AUTOSAR Doc:*
     * Erroneous response transmission such as: 
     *  Mismatch between sent and read back data,
     *  Physical bus error 
     */
    LIN_TX_ERROR = SilKit_LinFrameStatus_LIN_TX_ERROR,

    /*! The controller received a correct frame response.
     *
     * *AUTOSAR Doc:* Reception of correct response.
     */
    LIN_RX_OK = SilKit_LinFrameStatus_LIN_RX_OK,

    /*! (currently not in use)
     * 
     * *AUTOSAR Doc:* Ongoing reception: at least one response byte has been
     *  received, but the checksum byte has not been received.
     */
    LIN_RX_BUSY = SilKit_LinFrameStatus_LIN_RX_BUSY,

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
    LIN_RX_ERROR = SilKit_LinFrameStatus_LIN_RX_ERROR,

    /*! No LIN controller did provide a response to the frame header.
     *
     * *AUTOSAR Doc:* No response byte has been received so far. 
     */
    LIN_RX_NO_RESPONSE = SilKit_LinFrameStatus_LIN_RX_NO_RESPONSE,
};

/*! Used to configure a LIN controller as a master or slave.
 *
 *  Cf. \ref LinControllerConfig, \ref ILinController::Init()
 */
enum class LinControllerMode : SilKit_LinControllerMode
{
    /*! The LIN controller has not been configured yet and is
     *  inactive. This does not indicate sleep mode.
     */
    Inactive = SilKit_LinControllerMode_Inactive,
    //! A LIN controller with active master task and slave task
    Master = SilKit_LinControllerMode_Master,
    //! A LIN controller with only a slave task
    Slave = SilKit_LinControllerMode_Slave,
};

/*! The operational baud rate of the controller.
 *
 * *Range:* 200...20'000 Bd
 */
using LinBaudRate = SilKit_LinBaudRate;

/*! Configuration data to initialize the LIN Controller
 *  Cf.: \ref ILinController::Init(LinControllerConfig config);
 */
struct LinControllerConfig
{
    //! Configure as LIN master or LIN slave
    LinControllerMode controllerMode{LinControllerMode::Inactive};
    /*! The operational baud rate of the controller. Used in a detailed simulation.
     */
    LinBaudRate baudRate{0};
    /*! Optional LinFrameResponse configuration.
     *
     * FrameResponses can also be configured at a later point using
     * ILinController::UpdateTxBuffer() and
     * ILinController::SetFrameResponses().
     */
    std::vector<LinFrameResponse> frameResponses;

};

/*! The operational state of the controller, i.e., operational or
 *  sleeping.
 */
enum class LinControllerStatus : SilKit_LinControllerStatus
{
    //! The controller state is not yet known.
    Unknown = SilKit_LinControllerStatus_Unknown,

    //! Normal operation
    Operational = SilKit_LinControllerStatus_Operational,

    //! Sleep state operation; in this state wake-up detection from slave nodes
    //  is enabled.
    Sleep = SilKit_LinControllerStatus_Sleep,

    //! Sleep pending state is reached when a GoToSleep is issued.
    //  This allows the network simulator to finish pending transmissions (e.g., sleep frames to slaves)
    //  before entering state Sleep, cf. AUTOSAR SWS LINDriver [SWS_LIN_00266] and section 7.3.3.
    //  This is only used when using detailed simulations with a network simulator.
    SleepPending = SilKit_LinControllerStatus_SleepPending,
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

/*! \brief Data type representing a request to perform an non-AUTOSAR send operation.
*/
struct LinSendFrameHeaderRequest
{
    std::chrono::nanoseconds timestamp; //!< Time of the header request.
    LinId id; //!< The LinId of the header to be transmitted
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
//! \brief Comparison operator for LinFrame
inline bool operator==(const LinFrame& lhs, const LinFrame& rhs)
{
    return lhs.id == rhs.id
        && lhs.checksumModel == rhs.checksumModel
        && lhs.dataLength == rhs.dataLength
        && lhs.data == rhs.data;
}
//! \brief Comparison operator for LinFrameResponse
inline bool operator==(const LinFrameResponse& lhs, const LinFrameResponse& rhs)
{
    return lhs.frame == rhs.frame
        && lhs.responseMode == rhs.responseMode;
}
//! \brief Comparison operator for LinControllerConfig
inline bool operator==(const LinControllerConfig& lhs, const LinControllerConfig& rhs)
{
    return lhs.controllerMode == rhs.controllerMode
        && lhs.baudRate == rhs.baudRate
        && lhs.frameResponses == rhs.frameResponses;
}

} // namespace Lin
} // namespace Services
} // namespace SilKit
