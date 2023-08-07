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

/*! The operational state of the controller, i.e., operational or sleeping. */
typedef uint32_t SilKit_LinControllerStatus;
/*! The controller state is not yet known. */
#define SilKit_LinControllerStatus_Unknown ((SilKit_LinControllerStatus)0)
/*! Normal operation */
#define SilKit_LinControllerStatus_Operational ((SilKit_LinControllerStatus)1)
/*! Sleep state operation; in this state wake-up detection from slave nodes is enabled. */
#define SilKit_LinControllerStatus_Sleep ((SilKit_LinControllerStatus)2)
/*! \brief Sleep Pending state is reached when a GoToSleep is issued.
 *
 * This allows the network simulator to finish pending transmissions (e.g., sleep frames to slaves)
 * before entering state Sleep, cf. AUTOSAR SWS LINDriver [SWS_LIN_00266] and section 7.3.3.
 * This is only used when using detailed simulations.
 */
#define SilKit_LinControllerStatus_SleepPending ((SilKit_LinControllerStatus)3)

/*! Used to configure a LIN controller as a master or slave.
 *
 *  Cf. SilKit_LinControllerConfig, SilKit_LinController_Init()
 */
typedef uint8_t SilKit_LinControllerMode;
/*! The LIN controller has not been configured yet and is
 *  inactive. This does not indicate sleep mode.
 */
#define SilKit_LinControllerMode_Inactive ((SilKit_LinControllerMode)0)
/*! A LIN controller with active master task and slave task */
#define SilKit_LinControllerMode_Master ((SilKit_LinControllerMode)1)
/*! A LIN controller with only a slave task */
#define SilKit_LinControllerMode_Slave ((SilKit_LinControllerMode)2)

/*! The operational baud rate of the controller.
 * Used in detailed simulation. 
 *
 * *Range:* 200...20'000 Bd
 */
typedef uint32_t SilKit_LinBaudRate;

/*! \brief Controls the behavior of a LIN Slave task for a particular LIN ID */
typedef uint8_t SilKit_LinFrameResponseMode;
/*! The LinFrameResponse corresponding to the ID is neither received nor transmitted by the LIN slave. */
#define SilKit_LinFrameResponseMode_Unused ((SilKit_LinFrameResponseMode)0)
/*! The LinFrameResponse corresponding to the ID is received by the LIN slave. */
#define SilKit_LinFrameResponseMode_Rx ((SilKit_LinFrameResponseMode)1)
/*! The LinFrameResponse corresponding to the ID is transmitted unconditionally by the LIN slave. */
#define SilKit_LinFrameResponseMode_TxUnconditional ((SilKit_LinFrameResponseMode)2)

/*! \brief The identifier of a LIN \ref SilKit_LinFrame
 *
 * This type represents all valid identifier used by \ref SilKit_LinController_SendFrame(), \ref
 * SilKit_LinController_SendFrameHeader().
 *
 * *Range:* 0...0x3F
 */
typedef uint8_t SilKit_LinId;

/*! \brief The checksum model of a LIN \ref SilKit_LinFrame
 *
 * This type is used to specify the Checksum model to be used for the LIN \ref SilKit_LinFrame.
 */
typedef uint8_t SilKit_LinChecksumModel;
#define SilKit_LinChecksumModel_Unknown ((SilKit_LinChecksumModel)0) //!< Unknown checksum model. If configured with this value, the checksum model of the first reception will be used.
#define SilKit_LinChecksumModel_Enhanced ((SilKit_LinChecksumModel)1) //!< Enhanced checksum model
#define SilKit_LinChecksumModel_Classic ((SilKit_LinChecksumModel)2) //!< Classic checksum model

/*! \brief Controls the behavior of \ref SilKit_LinController_SendFrame()
 *
 * Determines whether the master also provides a frame response or if the frame
 * response is expected to be provided from a slave.
 */
typedef uint8_t SilKit_LinFrameResponseType;
/*! Response is generated from this (master) node */
#define SilKit_LinFrameResponseType_MasterResponse ((SilKit_LinFrameResponseType)0)
/*! Response is generated from a remote slave node */
#define SilKit_LinFrameResponseType_SlaveResponse ((SilKit_LinFrameResponseType)1)
/*! Response is generated from one slave to and received by
 *  another slave, for the master the response will be anonymous,
 *  it does not have to receive the response.
 */
#define SilKit_LinFrameResponseType_SlaveToSlave ((SilKit_LinFrameResponseType)2)

/*! \brief The state of a LIN transmission
 *
 * Used to indicate the success or failure of a LIN transmission to a
 * registered \ref SilKit_LinFrameStatusHandler_t.
 *
 * *Note:* the enumeration values directly correspond to the AUTOSAR
 *  type Lin_StatusType. Not all values are used in the SIL Kit.
 *
 * *AUTOSAR Doc:* LIN operation states for a LIN channel or frame, as
 * returned by the API service Lin_GetStatus().
 *
 */
typedef uint8_t SilKit_LinFrameStatus;

/*! (currently not in use) */
#define SilKit_LinFrameStatus_NOT_OK              ((SilKit_LinFrameStatus)0)
/*! The controller successfully transmitted a frame response. */
#define SilKit_LinFrameStatus_LIN_TX_OK           ((SilKit_LinFrameStatus)1)
/*! (currently not in use) */
#define SilKit_LinFrameStatus_LIN_TX_BUSY         ((SilKit_LinFrameStatus)2)
/*! (currently not in use) */
#define SilKit_LinFrameStatus_LIN_TX_HEADER_ERROR ((SilKit_LinFrameStatus)3)
/*! (currently not in use) */
#define SilKit_LinFrameStatus_LIN_TX_ERROR        ((SilKit_LinFrameStatus)4)
/*! The controller received a correct frame response. */
#define SilKit_LinFrameStatus_LIN_RX_OK           ((SilKit_LinFrameStatus)5)
/*! (currently not in use) */
#define SilKit_LinFrameStatus_LIN_RX_BUSY         ((SilKit_LinFrameStatus)6)
/*! The reception of a response failed.
 *
 * Indicates a mismatch in expected and received data length or a checksum
 * error. Checksum errors occur when multiple slaves are configured to
 * transmit the same frame or when the sender and receiver use different
 * checksum models.
 */
#define SilKit_LinFrameStatus_LIN_RX_ERROR        ((SilKit_LinFrameStatus)7)
/*! No LIN controller did provide a response to the frame header. */
#define SilKit_LinFrameStatus_LIN_RX_NO_RESPONSE  ((SilKit_LinFrameStatus)8)

/*! \brief The data length of a LIN \ref SilKit_LinFrame in bytes
 *
 * This type is used to specify the number of data bytes to copy.
 *
 * *Range:* 1...8
 */
typedef uint8_t SilKit_LinDataLength;

//! \brief If configured with this value, the data length of the first reception will be used.
const SilKit_LinDataLength SilKit_LinDataLengthUnknown = 255u;

/*! \brief A LIN SilKit_LinFrame
 *
 * This Type is used to provide LIN ID, checksum model, data length and data.
 *
 * *AUTOSAR Name:* Lin_PduType
 */
struct SilKit_LinFrame
{
    SilKit_StructHeader structHeader;   //!< The interface id specifying which version of this struct was obtained
    SilKit_LinId               id;            //!< LIN Identifier
    SilKit_LinChecksumModel    checksumModel; //!< Checksum Model
    SilKit_LinDataLength       dataLength;    //!< Data length
    uint8_t                data[8];       //!< The actual payload
};
typedef struct SilKit_LinFrame SilKit_LinFrame;

/*! \brief A LIN frame status event delivered in the \ref SilKit_LinFrameStatusHandler_t. */
struct SilKit_LinFrameStatusEvent
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    SilKit_NanosecondsTime timestamp; //!< Time of the event.
    SilKit_LinFrame* frame;
    SilKit_LinFrameStatus status;
};
typedef struct SilKit_LinFrameStatusEvent SilKit_LinFrameStatusEvent;

/*! \brief A LIN wakeup event delivered in the \ref SilKit_LinWakeupHandler_t. */
struct SilKit_LinWakeupEvent
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    SilKit_NanosecondsTime timestamp; //!< Time of the event.
    SilKit_Direction direction; //!< The direction of the event.
};
typedef struct SilKit_LinWakeupEvent SilKit_LinWakeupEvent;

/*! \brief A LIN goToSleep event delivered in the \ref SilKit_LinGoToSleepHandler_t */
struct SilKit_LinGoToSleepEvent
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    SilKit_NanosecondsTime timestamp; //!< Time of the event.
};
typedef struct SilKit_LinGoToSleepEvent SilKit_LinGoToSleepEvent;

/*! \brief A LIN wakeup event delivered in the \ref SilKit_LinWakeupHandler_t. */
struct SilKit_Experimental_LinSlaveConfigurationEvent
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    SilKit_NanosecondsTime timestamp; //!< Time of the event.
};
typedef struct SilKit_Experimental_LinSlaveConfigurationEvent SilKit_Experimental_LinSlaveConfigurationEvent;

/*! \brief Configuration data for a LIN Slave task for a particular LIN ID. */
struct SilKit_LinFrameResponse
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    /*! frame must provide the LIN \ref SilKit_LinId for which the response is
     *  configured.
     *
     * If responseMode is SilKit_LinFrameResponseMode_TxUnconditional, the
     * frame data is used for the transaction.
     */
    SilKit_LinFrame* frame;
    //! Determines if the LinFrameResponse is used for transmission
    //! (TxUnconditional), reception (Rx) or ignored (Unused).
    SilKit_LinFrameResponseMode responseMode;
};
typedef struct SilKit_LinFrameResponse SilKit_LinFrameResponse;

/*! Configuration data to initialize the LIN Controller
 *  Cf.: \ref SilKit_LinController_Init();
 */
struct SilKit_LinControllerConfig
{
    /*! The interface id specifying which version of this struct was obtained */
    SilKit_StructHeader structHeader;
    /*! Configure as LIN master or LIN slave */
    SilKit_LinControllerMode controllerMode;
    /*! The operational baud rate of the controller. */
    SilKit_LinBaudRate baudRate;

    size_t numFrameResponses;
    /*! LinFrameResponse configuration. */
    SilKit_LinFrameResponse*  frameResponses;
};
typedef struct SilKit_LinControllerConfig SilKit_LinControllerConfig;

/*! \brief Configuration data to initialize the LIN controller in dynamic response mode.
 * Cf.: \ref SilKit_Experimental_LinController_InitDynamic()
 */
struct SilKit_Experimental_LinControllerDynamicConfig
{
    /*! The interface id specifying which version of this struct was obtained */
    SilKit_StructHeader structHeader;
    /*! Configure as LIN master or LIN slave */
    SilKit_LinControllerMode controllerMode;
    /*! The operational baud rate of the controller. */
    SilKit_LinBaudRate baudRate;
};
typedef struct SilKit_Experimental_LinControllerDynamicConfig SilKit_Experimental_LinControllerDynamicConfig;

/*! \brief The aggregated configuration of all LIN slaves in the network.
 * \param numRespondingLinIds The number of entries in respondingLinIds.
 * \param respondingLinIds An array of SilKit_LinId on which any LIN Slave has configured SilKit_LinFrameResponseMode_TxUnconditional
 */
struct SilKit_Experimental_LinSlaveConfiguration
{
    /*!< The interface id specifying which version of this struct was obtained */
    SilKit_StructHeader structHeader;
    SilKit_Bool isLinIdResponding[64];
};
typedef struct SilKit_Experimental_LinSlaveConfiguration SilKit_Experimental_LinSlaveConfiguration;

/*! \brief A LIN frame header event delivered in the \ref SilKit_Experimental_LinFrameHeaderHandler_t. */
struct SilKit_Experimental_LinFrameHeaderEvent
{
    SilKit_StructHeader    structHeader; //!< The interface id specifying which version of this struct was obtained
    SilKit_NanosecondsTime timestamp;    //!< Time of the event.
    SilKit_LinId           id;           //!< LIN Identifier
};
typedef struct SilKit_Experimental_LinFrameHeaderEvent SilKit_Experimental_LinFrameHeaderEvent;

/*!
 * The LIN controller can assume the role of a LIN master or a LIN
 * slave. It provides two kinds of interfaces to perform data
 * transfers and provide frame responses:
 *
 * AUTOSAR-like LIN master interface:
 *
 * - \ref SilKit_LinController_SendFrame() transfers a frame from or to a LIN
 * master. Requires \ref SilKit_LinControllerMode_Master.
 *
 *
 * non-AUTOSAR interface:
 *
 * - \ref SilKit_LinController_SendFrameHeader() initiates the transmission of a
 * LIN frame for a particular LIN identifier. For a successful
 * transmission, exactly one LIN slave or master must have previously
 * set a corresponding frame response for unconditional
 * transmission. Requires \ref SilKit_LinControllerMode_Master.
 *
 */
typedef struct SilKit_LinController SilKit_LinController;

/*! Callback type to indicate the end of a LIN Frame transmission.
 * \param context The context provided by the user on registration.
 * \param controller The LIN controller that received the acknowledge.
 * \param frameStatusEvent The event containing a timestamp, the corresponding frame and the new status.
 */
typedef void (SilKitFPTR *SilKit_LinFrameStatusHandler_t)(void* context, SilKit_LinController* controller,
                                            const SilKit_LinFrameStatusEvent* frameStatusEvent);

/*! Callback type to indicate that a go-to-sleep frame has been received.
 *  Cf., \ref SilKit_LinController_AddGoToSleepHandler();
 */
typedef void (SilKitFPTR *SilKit_LinGoToSleepHandler_t)(void* context, SilKit_LinController* controller,
                                          const SilKit_LinGoToSleepEvent* goToSleepEvent);

/*! Callback type to indicate that a wakeup pulse has been received.
 *  Cf., \ref SilKit_LinController_AddWakeupHandler;
 */
typedef void (SilKitFPTR *SilKit_LinWakeupHandler_t)(void* context, SilKit_LinController* controller,
                                       const SilKit_LinWakeupEvent* wakeUpEvent);

/*! Callback type to indicate that a LIN Slave configuration has been received.
 *  Cf., \ref SilKit_Experimental_LinController_AddLinSlaveConfigurationHandler;
 */
typedef void (SilKitFPTR *SilKit_Experimental_LinSlaveConfigurationHandler_t)(void* context, SilKit_LinController* controller,
                                          const SilKit_Experimental_LinSlaveConfigurationEvent* slaveConfigurationEvent);

/*! Callback type to indicate the reception of a LIN frame header.
 * \param context The context provided by the user on registration.
 * \param controller The LIN controller that received the acknowledge.
 * \param frameHeaderEvent The event containing information about the frame header.
 */
typedef void (SilKitFPTR* SilKit_Experimental_LinFrameHeaderHandler_t)(
    void* context, SilKit_LinController* controller, const SilKit_Experimental_LinFrameHeaderEvent* frameHeaderEvent);


/*! \brief Create a LIN controller at this SIL Kit simulation participant.
 *
 * The lifetime of the resulting LIN controller is directly bound to the lifetime of the simulation participant.
 * There is no further cleanup necessary except for destroying the simulation participant at the end of the
 * simulation.
 *
 * \param outLinController Pointer into which the resulting LIN controller will be written (out parameter).
 * \param participant The simulation participant at which the LIN controller should be created.
 * \param name The name of the new LIN controller.
 * \param network The network of the LIN controller to operate in.
 *
 * \return \ref SilKit_ReturnCode
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LinController_Create(
  SilKit_LinController** outLinController,
  SilKit_Participant *participant,
  const char* name,
  const char* network);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LinController_Create_t)(
  SilKit_LinController** outLinController,
  SilKit_Participant* participant, 
  const char* name,
  const char* network);

/*! \brief Initialize the LIN controller defining its role and RX/TX configuration. 
 * 
 * All controllers must be initialized exactly once to take part in LIN communication.
 * 
 * \param controller The LIN controller to initialize
 * \param config The controller configuration contains:
 *  - controllerMode, either sets LIN master or LIN slave mode.
 *  - baudRate, determine transmission speeds (only used for detailed simulation).
 *  - frameResponses, a vector of LinFrameResponse. This must contain the final configuration
 * on which LIN Ids the controller will receive (\ref SilKit_LinFrameResponseMode_Rx) or respond to
 * (\ref SilKit_LinFrameResponseMode_TxUnconditional) frames. An exception is the use of \ref SilKit_LinController_SendFrame for 
 * LinFrameResponseType::MasterResponse, which allows to extend the configuration during operation. 
 *
 * *AUTOSAR Name:* Lin_Init
 * 
 * \return \ref SilKit_ReturnCode
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LinController_Init(SilKit_LinController* controller, const SilKit_LinControllerConfig* config);
typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LinController_Init_t)(SilKit_LinController* controller, const SilKit_LinControllerConfig* config);


/*! \brief Initialize the LIN controller in dynamic response mode.
 * 
 * All controllers must be initialized exactly once to take part in LIN communication.
 *
 * \param controller The LIN controller to initialize
 * \param config The controller configuration contains:
 *  - controllerMode, either sets LIN master or LIN slave mode.
 *  - baudRate, determine transmission speeds (only used for detailed simulation).
 *
 * \return \ref SilKit_ReturnCode
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_LinController_InitDynamic(
    SilKit_LinController* controller, const SilKit_Experimental_LinControllerDynamicConfig* config);
typedef SilKit_ReturnCode (SilKitFPTR *SilKit_Experimental_LinController_InitDynamic_t)(
    SilKit_LinController* controller, const SilKit_Experimental_LinControllerDynamicConfig* config);

/*! \brief Set a RX/TX configuration during operation.
 *
 * \param controller The LIN controller to set the configuration.
 * \param response The frame and response mode to be configured.
 *
 * \throws SilKit::StateError if the LIN Controller is not initialized.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LinController_SetFrameResponse(SilKit_LinController* controller,
                                                                             const SilKit_LinFrameResponse* response);
typedef SilKit_ReturnCode(SilKitFPTR* SilKit_LinController_SetFrameResponse_t)(SilKit_LinController* controller,
                                                                               const SilKit_LinFrameResponse* response);

/*! \brief When in LinSimulationMode_Default, send a response for the previously received LIN header.
 *
 * \param controller The LIN controller to set the configuration.
 * \param frame The frame to send immediately.
 *
 * \throws SilKit::StateError if the LIN Controller is not initialized.
 * \throws SilKit::StateError if the LIN controller was not initialized into dynamic response mode using \ref SilKit_Experimental_LinController_InitDynamic().
 * 
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_LinController_SendDynamicResponse(
    SilKit_LinController* controller, const SilKit_LinFrame* frame);
typedef SilKit_ReturnCode (SilKitFPTR *SilKit_Experimental_LinController_SendDynamicResponse_t)(
    SilKit_LinController* controller, const SilKit_LinFrame* frame);

/*! \brief Get the current status of the LIN Controller, i.e., Operational or Sleep.
 *
 * \param controller The LIN controller to retrieve the status
 * \param outStatus Pointer into which the status will be written (out parameter).
 *
 * \return \ref SilKit_ReturnCode
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LinController_Status(SilKit_LinController* controller, SilKit_LinControllerStatus* outStatus);
typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LinController_Status_t)(SilKit_LinController* controller, SilKit_LinControllerStatus* outStatus);

/*! \brief Initiate a LIN data transfer of a given LinFrameResponseType (AUTOSAR LIN master interface)
 *
 * The responseType determines if frame.data is used for the frame response or if a different node has to provide 
 * it:
 * - MasterResponse: LinFrame is sent from this controller to all connected slaves using frame.data. The LIN
 * Master doesn't have to be configured with SilKit_LinFrameResponseMode_TxUnconditional on this LIN ID.
 * - SlaveResponse: the frame response must be provided by a connected slave and is received by this controller.
 * - SlaveToSlave: the frame response must be provided by a connected slave and is ignored by this controller.
 *
 * *AUTOSAR Name:* Lin_SendFrame
 *
 * \param controller The LIN controller to operate on
 * \param frame Provides the LIN identifier, checksum model, and optional data.
 * \param responseType Determines which LIN Node will provide the frame response.
 * 
 * \return \ref SilKit_ReturnCode
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LinController_SendFrame(SilKit_LinController* controller, const SilKit_LinFrame* frame,
                                                            SilKit_LinFrameResponseType responseType);
typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LinController_SendFrame_t)(SilKit_LinController* controller, const SilKit_LinFrame* frame,
                                                      SilKit_LinFrameResponseType responseType);


/*! \brief Initiate a LIN data transfer by sending a LIN header (AUTOSAR LIN master interface)
 * 
 * \param controller The LIN controller to operate on
 * \param linId Provides the LIN header identifier. The node that is configured to respond on this ID will complete
 * the transmission and provide the response data.
 *
 * \throws SilKit::StateError if the LIN Controller is not initialized or not a master node.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LinController_SendFrameHeader(SilKit_LinController* controller, SilKit_LinId linId);
typedef SilKit_ReturnCode(SilKitFPTR *SilKit_LinController_SendFrameHeader_t)(SilKit_LinController* controller, SilKit_LinId linId);

/*! Update the response data. The LIN controller needs to be configured with TxUnconditional on this ID. 
 * 
 * \param frame Provides the LIN ID and data used for the update.
 * 
 * \param controller The LIN controller to operate on
 * 
 * \return \ref SilKit_ReturnCode
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LinController_UpdateTxBuffer(SilKit_LinController* controller,
                                                                   const SilKit_LinFrame* frame);
typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LinController_UpdateTxBuffer_t)(SilKit_LinController* controller,
                                                                     const SilKit_LinFrame* frame);

/*! \brief Transmit a go-to-sleep-command and set SilKit_LinControllerStatus_Sleep and enable wake-up
 *
 * *AUTOSAR Name:* Lin_GoToSleep
 * 
 * \param controller The LIN controller to operate on
 * 
 * \return SilKit_ReturnCode_SUCCESS or SilKit_ReturnCode_WRONGSTATE if issued with wrong \ref SilKit_LinControllerMode
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LinController_GoToSleep(SilKit_LinController* controller);
typedef SilKit_ReturnCode(SilKitFPTR *SilKit_LinController_GoToSleep_t)(SilKit_LinController* controller);

/*! \brief Set SilKit_LinControllerStatus_Sleep without sending a go-to-sleep command.
 *
 * *AUTOSAR Name:* Lin_GoToSleepInternal
 * 
 * \param controller The LIN controller to operate on
 * 
 * \return SilKit_ReturnCode_SUCCESS or SilKit_ReturnCode_WRONGSTATE if issued with wrong \ref SilKit_LinControllerMode
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LinController_GoToSleepInternal(SilKit_LinController* controller);
typedef SilKit_ReturnCode(SilKitFPTR *SilKit_LinController_GoToSleepInternal_t)(SilKit_LinController* controller);

/*! \brief Generate a wake up pulse and set SilKit_LinControllerStatus_Operational.
 *
 * *AUTOSAR Name:* Lin_Wakeup
 * 
 * \param controller The LIN controller to operate on
 * 
 * \return SilKit_ReturnCode_SUCCESS or SilKit_ReturnCode_WRONGSTATE if issued with wrong \ref SilKit_LinControllerMode
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LinController_Wakeup(SilKit_LinController* controller);
typedef SilKit_ReturnCode(SilKitFPTR *SilKit_LinController_Wakeup_t)(SilKit_LinController* controller);

/*! Set SilKit_LinControllerStatus_Operational without generating a wake up pulse.
 *
 * *AUTOSAR Name:* Lin_WakeupInternal
 * 
 * \param controller The LIN controller to operate on
 * 
 * \return SilKit_ReturnCode_SUCCESS or SilKit_ReturnCode_WRONGSTATE if issued with wrong \ref SilKit_LinControllerMode
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LinController_WakeupInternal(SilKit_LinController* controller);
typedef SilKit_ReturnCode(SilKitFPTR *SilKit_LinController_WakeupInternal_t)(SilKit_LinController* controller);

/*! \brief Get the aggregated configuration of all LIN slaves in the network.
 *
 * Requires \ref SilKit_LinControllerMode_Master
 * 
 * \param outLinSlaveConfiguration Pointer into which the resulting LinSlaveConfiguration will be written (out parameter)
 * \param controller The LIN controller to operate on
 * 
 * \return SilKit_ReturnCode_SUCCESS or SilKit_ReturnCode_WRONGSTATE if issued with wrong \ref SilKit_LinControllerMode
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_LinController_GetSlaveConfiguration(
    SilKit_LinController* controller, SilKit_Experimental_LinSlaveConfiguration* outLinSlaveConfiguration);
typedef SilKit_ReturnCode (SilKitFPTR *SilKit_Experimental_LinController_GetSlaveConfiguration_t)(
    SilKit_LinController* controller, SilKit_Experimental_LinSlaveConfiguration* outLinSlaveConfiguration);

/*! \brief Reports the SilKit_LinFrameStatus of a SilKit_LinFrame and provides the transmitted frame.
 *
 * The handler is used for reception and acknowledgement of LIN frames. The direction (prefixed with 
 * LIN_TX_ or LIN_RX_) and error state of the tranmission is encoded in the \ref SilKit_LinFrameStatus. 
 * 
 *
 * \param controller The LIN controller for which the callback should be registered.
 * \param context The user provided context pointer that is reobtained in the callback.
 * \param handler The handler to be called.
 * \param outHandlerId The handler identifier that can be used to remove the callback.
 *
 * \return \ref SilKit_ReturnCode
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LinController_AddFrameStatusHandler(SilKit_LinController* controller, void* context,
                                                                        SilKit_LinFrameStatusHandler_t handler,
                                                                        SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LinController_AddFrameStatusHandler_t)(SilKit_LinController* controller, void* context,
                                                                   SilKit_LinFrameStatusHandler_t handler,
                                                                   SilKit_HandlerId* outHandlerId);

/*! \brief  Remove a \ref SilKit_LinFrameStatusHandler_t by SilKit_HandlerId on this controller 
 *
 * \param controller The LIN controller for which the callback should be removed.
 * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
 *
 * \return \ref SilKit_ReturnCode
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LinController_RemoveFrameStatusHandler(SilKit_LinController* controller,
                                                                           SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LinController_RemoveFrameStatusHandler_t)(SilKit_LinController* controller,
                                                                      SilKit_HandlerId handlerId);

/*! \brief The GoToSleepHandler is called whenever a go-to-sleep frame
 * was received.
 *
 * Note: The LIN controller does not automatically enter sleep
 * mode up reception of a go-to-sleep frame. I.e.,
 * SilKit_LinController_GoToSleepInternal() must be called manually
 *
 * Note: This handler will always be called, independently of the
 * \ref SilKit_LinFrameResponseMode configuration for LIN ID 0x3C. However,
 * regarding the SilKit_LinFrameStatusHandler, the go-to-sleep frame is
 * treated like every other frame, i.e. the SilKit_LinFrameStatusHandler is
 * only called for LIN ID 0x3C if configured as
 * SilKit_LinFrameResponseMode_Rx.
 * 
 * \param controller The LIN controller for which the callback should be registered.
 * \param context The user provided context pointer that is reobtained in the callback.
 * \param handler The handler to be called.
 * \param outHandlerId The handler identifier that can be used to remove the callback.
 * 
 * \return \ref SilKit_ReturnCode
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LinController_AddGoToSleepHandler(SilKit_LinController* controller, void* context,
                                                                      SilKit_LinGoToSleepHandler_t handler,
                                                                      SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LinController_AddGoToSleepHandler_t)(SilKit_LinController* controller, void* context,
                                                                 SilKit_LinGoToSleepHandler_t handler,
                                                                 SilKit_HandlerId* outHandlerId);

/*! \brief  Remove a \ref SilKit_LinGoToSleepHandler_t by SilKit_HandlerId on this controller 
 *
 * \param controller The LIN controller for which the callback should be removed.
 * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
 *
 * \return \ref SilKit_ReturnCode
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LinController_RemoveGoToSleepHandler(SilKit_LinController* controller,
                                                                         SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LinController_RemoveGoToSleepHandler_t)(SilKit_LinController* controller,
                                                                    SilKit_HandlerId handlerId);

/*! \brief The WakeupHandler is called whenever a wake up pulse is received
 *
 * Note: The LIN controller does not automatically enter
 * operational mode on wake up pulse detection. I.e.,
 * WakeInternal() must be called manually.
 * 
 * \param controller The LIN controller for which the callback should be registered.
 * \param context The user provided context pointer that is reobtained in the callback.
 * \param handler The handler to be called.
 * \param outHandlerId The handler identifier that can be used to remove the callback.
 *
 * \return \ref SilKit_ReturnCode
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LinController_AddWakeupHandler(SilKit_LinController* controller, void* context,
                                                                   SilKit_LinWakeupHandler_t handler,
                                                                   SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode(SilKitFPTR *SilKit_LinController_AddWakeupHandler_t)(SilKit_LinController* controller, void* context,
                                                             SilKit_LinWakeupHandler_t handler,
                                                             SilKit_HandlerId* outHandlerId);

/*! \brief  Remove a \ref SilKit_LinWakeupHandler_t by SilKit_HandlerId on this controller 
*
 * \param controller The LIN controller for which the callback should be removed.
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_LinController_RemoveWakeupHandler(SilKit_LinController* controller,
                                                                      SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_LinController_RemoveWakeupHandler_t)(SilKit_LinController* controller, SilKit_HandlerId handlerId);


/*! \brief The LinSlaveConfigurationHandler is called whenever a remote LIN Slave is configured via SilKit_LinController_Init
 *
 * Note: This callback is mainly for diagnostic purposes and is NOT needed for regular LIN controller operation. 
 * It can be used to call \ref SilKit_Experimental_LinController_GetSlaveConfiguration to keep track of LIN Ids, where
 * a response of a LIN Slave is to be expected.
 * 
 * Requires \ref SilKit_LinControllerMode_Master
 * 
 * \param controller The LIN controller for which the callback should be registered.
 * \param context The user provided context pointer that is reobtained in the callback.
 * \param handler The handler to be called.
 * \param outHandlerId The handler identifier that can be used to remove the callback.
 *
 * \return \ref SilKit_ReturnCode
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_LinController_AddLinSlaveConfigurationHandler(SilKit_LinController* controller, void* context,
                                                                  SilKit_Experimental_LinSlaveConfigurationHandler_t handler,
                                                                  SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_Experimental_LinController_AddLinSlaveConfigurationHandler_t)(SilKit_LinController* controller, void* context,
                                                                     SilKit_Experimental_LinSlaveConfigurationHandler_t handler,
                                                                     SilKit_HandlerId* outHandlerId);

/*! \brief  Remove a \ref SilKit_Experimental_LinSlaveConfigurationHandler_t by SilKit_HandlerId on this controller
 *
 * \param controller The LIN controller for which the callback should be removed.
 * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
 *
 * \return \ref SilKit_ReturnCode
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_LinController_RemoveLinSlaveConfigurationHandler(SilKit_LinController* controller,
                                                                     SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_Experimental_LinController_RemoveLinSlaveConfigurationHandler_t)(SilKit_LinController* controller,
                                                                        SilKit_HandlerId handlerId);


/*! \brief The LinFrameHeaderHandler is called whenever the master sends a LIN header.
 *
 * \param controller The LIN controller for which the callback should be registered.
 * \param context The user provided context pointer that is obtained in the callback.
 * \param handler The handler to be called.
 * \param outHandlerId The handler identifier that can be used to remove the callback.
 *
 * \throws SilKit::StateError if the LIN controller was not initialized into dynamic response mode using \ref SilKit_Experimental_LinController_InitDynamic().
 *
 * \return \ref SilKit_ReturnCode
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_LinController_AddFrameHeaderHandler(
    SilKit_LinController* controller, void* context, SilKit_Experimental_LinFrameHeaderHandler_t handler,
    SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_Experimental_LinController_AddFrameHeaderHandler_t)(
    SilKit_LinController* controller, void* context, SilKit_Experimental_LinFrameHeaderHandler_t handler,
    SilKit_HandlerId* outHandlerId);

SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_Experimental_LinController_RemoveFrameHeaderHandler(
    SilKit_LinController* controller, SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_Experimental_LinController_RemoveFrameHeaderHandler_t)(
    SilKit_LinController* controller, SilKit_HandlerId handlerId);


SILKIT_END_DECLS

#pragma pack(pop)
