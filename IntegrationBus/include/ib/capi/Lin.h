/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#pragma once
#include <stdint.h>
#include "silkit/capi/SilKitMacros.h"
#include "silkit/capi/Types.h"
#include "silkit/capi/InterfaceIdentifiers.h"

#pragma pack(push)
#pragma pack(8)

SILKIT_BEGIN_DECLS

//! The operational state of the controller, i.e., operational or sleeping.
typedef uint32_t SilKit_LinControllerStatus;
//! The controller state is not yet known.
#define SilKit_LinControllerStatus_Unknown ((SilKit_LinControllerStatus)0)
//! Normal operation
#define SilKit_LinControllerStatus_Operational ((SilKit_LinControllerStatus)1)
//! Sleep state operation; in this state wake-up detection from slave nodes
//  is enabled.
#define SilKit_LinControllerStatus_Sleep ((SilKit_LinControllerStatus)2)
//! Sleep Pending state is reached when a GoToSleep is issued.
//  This allows the network simulator to finish pending transmissions (e.g., sleep frames to slaves)
//  before entering state Sleep, cf. AUTOSAR SWS LINDriver [SWS_LIN_00266] and section 7.3.3.
//  This is only used when using detailed simulations.
#define SilKit_LinControllerStatus_SleepPending ((SilKit_LinControllerStatus)3)

/*! Used to configure a Lin controller as a master or slave.
 *
 *  Cf. SilKit_LinControllerConfig, SilKit_LinController_Init()
 */
typedef uint8_t SilKit_LinControllerMode;
/*! The Lin controller has not been configured yet and is
 *  inactive. This does not indicate sleep mode.
 */
#define SilKit_LinControllerMode_Inactive ((SilKit_LinControllerMode)0)
//! A Lin controller with active master task and slave task
#define SilKit_LinControllerMode_Master ((SilKit_LinControllerMode)1)
//! A Lin controller with only a slave task
#define SilKit_LinControllerMode_Slave ((SilKit_LinControllerMode)2)

/*! The operational baud rate of the controller.
 * Used in detailed simulation. 
 *
 * *Range:* 200...20'000 Bd
 */
typedef uint32_t SilKit_LinBaudRate;

//! \brief Controls the behavior of a Lin Slave task for a particular Lin ID
typedef uint8_t SilKit_LinFrameResponseMode;
//! The LinFrameResponse corresponding to the ID is neither received nor
//! transmitted by the Lin slave.
#define SilKit_LinFrameResponseMode_Unused ((SilKit_LinFrameResponseMode)0)
//! The LinFrameResponse corresponding to the ID is received by the Lin slave.
#define SilKit_LinFrameResponseMode_Rx ((SilKit_LinFrameResponseMode)1)
//! The LinFrameResponse corresponding to the ID is transmitted unconditionally
//! by the Lin slave.
#define SilKit_LinFrameResponseMode_TxUnconditional ((SilKit_LinFrameResponseMode)2)

/*! \brief The identifier of a Lin \ref SilKit_LinFrame
 *
 * This type represents all valid identifier used by \ref SilKit_LinController_SendFrame(), \ref
 * SilKit_LinController_SendFrameHeader().
 *
 * *Range:* 0...0x3F
 */
typedef uint8_t SilKit_LinId;

/*! \brief The checksum model of a Lin \ref SilKit_LinFrame
 *
 * This type is used to specify the Checksum model to be used for the Lin \ref SilKit_LinFrame.
 */
typedef uint8_t SilKit_LinChecksumModel;
#define SilKit_LinChecksumModel_Undefined ((SilKit_LinChecksumModel)0) //!< Undefined / unconfigured checksum model
#define SilKit_LinChecksumModel_Enhanced ((SilKit_LinChecksumModel)1) //!< Enhanced checksum model
#define SilKit_LinChecksumModel_Classic ((SilKit_LinChecksumModel)2) //!< Classic checksum model

/*! \brief Controls the behavior of \ref SilKit_LinController_SendFrame()
 *
 * Determines whether the master also provides a frame response or if the frame
 * response is expected to be provided from a slave.
 */
typedef uint8_t SilKit_LinFrameResponseType;
//! Response is generated from this (master) node
#define SilKit_LinFrameResponseType_MasterResponse ((SilKit_LinFrameResponseType)0)
//! Response is generated from a remote slave node
#define SilKit_LinFrameResponseType_SlaveResponse ((SilKit_LinFrameResponseType)1)
/*! Response is generated from one slave to and received by
 *  another slave, for the master the response will be anonymous,
 *  it does not have to receive the response.
 */
#define SilKit_LinFrameResponseType_SlaveToSlave ((SilKit_LinFrameResponseType)2)

/*! \brief The state of a Lin transmission
 *
 * Used to indicate the success or failure of a Lin transmission to a
 * registered \ref SilKit_LinFrameStatusHandler_t.
 *
 * *Note:* the enumeration values directly correspond to the AUTOSAR
 *  type Lin_StatusType. Not all values are used in the Integration
 *  Bus.
 *
 * *AUTOSAR Doc:* Lin operation states for a Lin channel or frame, as
 * returned by the API service Lin_GetStatus().
 *
 */
typedef uint8_t SilKit_LinFrameStatus;

/*! (currently not in use)
 */
#define SilKit_LinFrameStatus_NOT_OK              ((SilKit_LinFrameStatus)0)
/*! The controller successfully transmitted a frame response.
 */
#define SilKit_LinFrameStatus_LIN_TX_OK           ((SilKit_LinFrameStatus)1)
/*! (currently not in use)
 */
#define SilKit_LinFrameStatus_LIN_TX_BUSY         ((SilKit_LinFrameStatus)2)
/*! (currently not in use)
 */
#define SilKit_LinFrameStatus_LIN_TX_HEADER_ERROR ((SilKit_LinFrameStatus)3)
/*! (currently not in use)
 */
#define SilKit_LinFrameStatus_LIN_TX_ERROR        ((SilKit_LinFrameStatus)4)
/*! The controller received a correct frame response.
 */
#define SilKit_LinFrameStatus_LIN_RX_OK           ((SilKit_LinFrameStatus)5)
/*! (currently not in use)
 */
#define SilKit_LinFrameStatus_LIN_RX_BUSY         ((SilKit_LinFrameStatus)6)
/*! The reception of a response failed.
 *
 * Indicates a mismatch in expected and received data length or a checksum
 * error. Checksum errors occur when multiple slaves are configured to
 * transmit the same frame or when the sender and receiver use different
 * checksum models.
 */
#define SilKit_LinFrameStatus_LIN_RX_ERROR        ((SilKit_LinFrameStatus)7)
/*! No Lin controller did provide a response to the frame header.
 */
#define SilKit_LinFrameStatus_LIN_RX_NO_RESPONSE  ((SilKit_LinFrameStatus)8)

/*! \brief The data length of a Lin \ref SilKit_LinFrame in bytes
 *
 * This type is used to specify the number of data bytes to copy.
 *
 * *Range:* 1...8
 */
typedef uint8_t SilKit_LinDataLength;

/*! \brief A Lin SilKit_LinFrame
 *
 * This Type is used to provide Lin ID, checksum model, data length and data.
 *
 * *AUTOSAR Name:* Lin_PduType
 */
struct SilKit_LinFrame
{
    SilKit_InterfaceIdentifier interfaceId;   //!< The interface id specifying which version of this struct was obtained
    SilKit_LinId               id;            //!< Lin Identifier
    SilKit_LinChecksumModel    checksumModel; //!< Checksum Model
    SilKit_LinDataLength       dataLength;    //!< Data length
    uint8_t                data[8];       //!< The actual payload
};
typedef struct SilKit_LinFrame SilKit_LinFrame;

//! \brief A LIN frame status event delivered in the \ref SilKit_LinFrameStatusHandler_t.
//! 
struct SilKit_LinFrameStatusEvent
{
    SilKit_InterfaceIdentifier interfaceId; //!< The interface id specifying which version of this struct was obtained
    SilKit_NanosecondsTime timestamp; //!< Time of the event.
    SilKit_LinFrame* frame;
    SilKit_LinFrameStatus status;
};
typedef struct SilKit_LinFrameStatusEvent SilKit_LinFrameStatusEvent;

//! \brief A LIN wakeup event delivered in the \ref SilKit_LinWakeupHandler_t.
struct SilKit_LinWakeupEvent
{
    SilKit_InterfaceIdentifier interfaceId; //!< The interface id specifying which version of this struct was obtained
    SilKit_NanosecondsTime timestamp; //!< Time of the event.
    SilKit_Direction direction; //!< The direction of the event.
};
typedef struct SilKit_LinWakeupEvent SilKit_LinWakeupEvent;

//! \brief A LIN goToSleep event delivered in the \ref SilKit_LinGoToSleepHandler_t
struct SilKit_LinGoToSleepEvent
{
    SilKit_InterfaceIdentifier interfaceId; //!< The interface id specifying which version of this struct was obtained
    SilKit_NanosecondsTime timestamp; //!< Time of the event.
};
typedef struct SilKit_LinGoToSleepEvent SilKit_LinGoToSleepEvent;

/*! \brief Configuration data for a Lin Slave task for a particular Lin ID.
 */
struct SilKit_LinFrameResponse
{
    SilKit_InterfaceIdentifier interfaceId; //!< The interface id specifying which version of this struct was obtained
    /*! frame must provide the Lin \ref SilKit_LinId for which the response is
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

/*! Configuration data to initialize the Lin Controller
 *  Cf.: \ref SilKit_LinController_Init();
 */
struct SilKit_LinControllerConfig
{
    SilKit_InterfaceIdentifier interfaceId; //!< The interface id specifying which version of this struct was obtained
    //! Configure as Lin master or Lin slave
    SilKit_LinControllerMode controllerMode;
    /*! The operational baud rate of the controller.
     */
    SilKit_LinBaudRate baudRate;

    uint32_t numFrameResponses;
    /*! Optional LinFrameResponse configuration.
     *
     * FrameResponses can also be configured at a later point using
     * \ref SilKit_LinController_SetFrameResponse() and
     * \ref SilKit_LinController_SetFrameResponses().
     */
    SilKit_LinFrameResponse*  frameResponses;
};
typedef struct SilKit_LinControllerConfig SilKit_LinControllerConfig;

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
 * - \ref SilKit_LinController_SetFrameResponses() configures
 * the response for a particular LIN identifier. Can be used with \ref
 * SilKit_LinControllerMode_Master and \ref SilKit_LinControllerMode_Slave.
 *
 * - \ref SilKit_LinController_SendFrameHeader() initiates the transmission of a
 * LIN frame for a particular LIN identifier. For a successful
 * transmission, exactly one LIN slave or master must have previously
 * set a corresponding frame response for unconditional
 * transmission. Requires \ref SilKit_LinControllerMode_Master.
 *
 */
typedef struct SilKit_LinController SilKit_LinController;

/*! Callback type to indicate the end of a Lin Frame transmission.
 * \param context The context provided by the user on registration.
 * \param controller The Lin controller that received the acknowledge.
 * \param frameStatusEvent The event containing a timestamp, the corresponding frame and the new status.
 */
typedef void (*SilKit_LinFrameStatusHandler_t)(void* context, SilKit_LinController* controller,
                                            const SilKit_LinFrameStatusEvent* frameStatusEvent);

/*! Callback type to indicate that a go-to-sleep frame has been received.
 *  Cf., \ref SilKit_LinController_AddGoToSleepHandler();
 */
typedef void (*SilKit_LinGoToSleepHandler_t)(void* context, SilKit_LinController* controller,
                                          const SilKit_LinGoToSleepEvent* goToSleepEvent);

/*! Callback type to indicate that a wakeup pulse has been received.
 *  Cf., \ref AddWakeupHandler(WakeupHandler);
 */
typedef void (*SilKit_LinWakeupHandler_t)(void* context, SilKit_LinController* controller,
                                       const SilKit_LinWakeupEvent* wakeUpEvent);

/*! \brief Create a Lin controller at this SilKit simulation participant.
 *
 * The lifetime of the resulting Lin controller is directly bound to the lifetime of the simulation participant.
 * There is no further cleanup necessary except for destroying the simulation participant at the end of the
 * simulation.
 *
 * \param outLinController Pointer into which the resulting Lin controller will be written (out parameter).
 * \param participant The simulation participant at which the Lin controller should be created.
 * \param name The name of the new Lin controller.
 *
 * \return \ref SilKit_ReturnCode
 */
SilKitAPI SilKit_ReturnCode SilKit_LinController_Create(
  SilKit_LinController** outLinController,
  SilKit_Participant *participant,
  const char* name,
  const char* network);

typedef SilKit_ReturnCode (*SilKit_LinController_Create_t)(
  SilKit_LinController** outLinController,
  SilKit_Participant* participant, 
  const char* name,
  const char* network);


/*! \brief Initialize the Lin controller
 *
 * \param controller The Lin controller to initialize
 * \param config The Controller configuration contains:
 *  - controllerMode: either sets Lin master or Lin slave mode
 *  - baudRate: determine transmission speeds
 *  - frameResponses: an optional set of initial FrameResponses
 * 
 * *AUTOSAR Name:* Lin_Init
 */
SilKitAPI SilKit_ReturnCode SilKit_LinController_Init(SilKit_LinController* controller, const SilKit_LinControllerConfig* config);

typedef SilKit_ReturnCode (*SilKit_LinController_Init_t)(SilKit_LinController* controller, const SilKit_LinControllerConfig* config);

/*! \brief Get the current status of the Lin Controller, i.e., Operational or Sleep.
 *
 * \param controller The Lin controller to retrieve the status
 * \param outStatus Pointer into which the status will be written (out parameter).
 *
 */
SilKitAPI SilKit_ReturnCode SilKit_LinController_Status(SilKit_LinController* controller, SilKit_LinControllerStatus* outStatus);

typedef SilKit_ReturnCode (*SilKit_LinController_Status_t)(SilKit_LinController* controller, SilKit_LinControllerStatus* outStatus);

/*! \brief AUTOSAR LIN master interface
 *
 * Perform a full Lin data transfer, i.e., frame header + frame
 * response. The responseType determines if frame.data is used for
 * the frame response or if a different node has to provide it:
 *
 * \li MasterResponse: \ref SilKit_LinFrame is sent from this controller to
 *     all connected slaves.
 * \li SlaveResponse: the frame response must be provided by a
 *     connected slave and is received by this controller.
 * \li SlaveToSlave: the frame response must be provided by a
 *     connected slave and is ignored by this controller.
 *
 * *AUTOSAR Name:* Lin_SendFrame
 * 
 * \param controller The Lin controller to operate on
 * \param frame provides the Lin identifier, checksum model, and optional data
 * \param responseType determines if *frame.data* is used for the frame response.
 */
SilKitAPI SilKit_ReturnCode SilKit_LinController_SendFrame(SilKit_LinController* controller, const SilKit_LinFrame* frame,
                                                            SilKit_LinFrameResponseType responseType);

typedef SilKit_ReturnCode (*SilKit_LinController_SendFrame_t)(SilKit_LinController* controller, const SilKit_LinFrame* frame,
                                                      SilKit_LinFrameResponseType responseType);

//! Send Interface for a non-AUTOSAR Lin Master
SilKitAPI SilKit_ReturnCode SilKit_LinController_SendFrameHeader(SilKit_LinController* controller, SilKit_LinId linId);

typedef SilKit_ReturnCode(*SilKit_LinController_SendFrameHeader_t)(SilKit_LinController* controller, SilKit_LinId linId);

/*! LinFrameResponse configuration for Slaves or non-AUTOSAR Lin
 *  Masters The corresponding Lin ID does not need to be
 *  previously configured. */
SilKitAPI SilKit_ReturnCode SilKit_LinController_SetFrameResponse(SilKit_LinController* controller,
                                                                   const SilKit_LinFrameResponse* frameResponse);

typedef SilKit_ReturnCode (*SilKit_LinController_SetFrameResponse_t)(SilKit_LinController* controller,
                                                              const SilKit_LinFrameResponse* frameResponse);

/*! LinFrameResponse configuration for Slaves or non-AUTOSAR Lin Masters.
 *
 * Configures multiple responses at once. Corresponding IDs do not
 * need to be previously configured.
 *
 * NB: only configures responses for the provided Lin IDs. I.e.,
 * an empty vector does not clear or reset the currently
 * configured FrameResponses.
 */
SilKitAPI SilKit_ReturnCode SilKit_LinController_SetFrameResponses(SilKit_LinController*    controller,
                                                                    const SilKit_LinFrameResponse* frameResponses,
                                                                    uint32_t                   numFrameResponses);
typedef SilKit_ReturnCode (*SilKit_LinController_SetFrameResponses_t)(SilKit_LinController*    controller,
                                                              const SilKit_LinFrameResponse* frameResponses,
                                                              uint32_t                   numFrameResponses);

/*! \brief Transmit a go-to-sleep-command and set SilKit_LinControllerStatus_Sleep and enable wake-up
 *
 * *AUTOSAR Name:* Lin_GoToSleep
 * \return SilKit_ReturnCode_SUCCESS or SilKit_ReturnCode_WRONGSTATE if issued with wrong \ref SilKit_LinControllerMode
 */
SilKitAPI SilKit_ReturnCode SilKit_LinController_GoToSleep(SilKit_LinController* controller);

typedef SilKit_ReturnCode(*SilKit_LinController_GoToSleep_t)(SilKit_LinController* controller);
/*! \brief Set SilKit_LinControllerStatus_Sleep without sending a go-to-sleep command.
 *
 * *AUTOSAR Name:* Lin_GoToSleepInternal
 * \return SilKit_ReturnCode_SUCCESS or SilKit_ReturnCode_WRONGSTATE if issued with wrong \ref SilKit_LinControllerMode
 */
SilKitAPI SilKit_ReturnCode SilKit_LinController_GoToSleepInternal(SilKit_LinController* controller);

typedef SilKit_ReturnCode(*SilKit_LinController_GoToSleepInternal_t)(SilKit_LinController* controller);
/*! \brief Generate a wake up pulse and set SilKit_LinControllerStatus_Operational.
 *
 * *AUTOSAR Name:* Lin_Wakeup
 * \return SilKit_ReturnCode_SUCCESS or SilKit_ReturnCode_WRONGSTATE if issued with wrong \ref SilKit_LinControllerMode
 */
SilKitAPI SilKit_ReturnCode SilKit_LinController_Wakeup(SilKit_LinController* controller);

typedef SilKit_ReturnCode(*SilKit_LinController_Wakeup_t)(SilKit_LinController* controller);
/*! Set SilKit_LinControllerStatus_Operational without generating a wake up pulse.
 *
 * *AUTOSAR Name:* Lin_WakeupInternal
 * \return SilKit_ReturnCode_SUCCESS or SilKit_ReturnCode_WRONGSTATE if issued with wrong \ref SilKit_LinControllerMode
 */
SilKitAPI SilKit_ReturnCode SilKit_LinController_WakeupInternal(SilKit_LinController* controller);

typedef SilKit_ReturnCode(*SilKit_LinController_WakeupInternal_t)(SilKit_LinController* controller);

/*! \brief Reports the \ref SilKit_LinFrameStatus of a Lin \ref SilKit_LinFrame
 * transmission and provides the transmitted frame.
 *
 * The \ref SilKit_LinFrameStatusHandler_t is called once per call to
 * \ref SilKit_LinController_SendFrame() or call to
 * \ref SilKit_LinController_SendFrameHeader(). The handler is called independently
 * of the transmission's success or failure.
 *
 * The SilKit_LinFrameStatusHandler_t is called for all participating Lin
 * controllers. I.e., for Lin masters, it is always called, and
 * for Lin slaves, it is called if the corresponding \ref SilKit_LinId is
 * configured SilKit_LinFrameResponseMode_Rx or
 * SilKit_LinFrameResponseModeTxUnconditional.
 *
 *
 * \param controller The LIN controller for which the callback should be registered.
 * \param context The user provided context pointer that is reobtained in the callback.
 * \param handler The handler to be called.
 * \param outHandlerId The handler identifier that can be used to remove the callback.
 */
SilKitAPI SilKit_ReturnCode SilKit_LinController_AddFrameStatusHandler(SilKit_LinController* controller, void* context,
                                                                        SilKit_LinFrameStatusHandler_t handler,
                                                                        SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode (*SilKit_LinController_AddFrameStatusHandler_t)(SilKit_LinController* controller, void* context,
                                                                   SilKit_LinFrameStatusHandler_t handler,
                                                                   SilKit_HandlerId* outHandlerId);

/*! \brief  Remove a \ref SilKit_LinFrameStatusHandler_t by SilKit_HandlerId on this controller 
*
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
SilKitAPI SilKit_ReturnCode SilKit_LinController_RemoveFrameStatusHandler(SilKit_LinController* controller,
                                                                           SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode (*SilKit_LinController_RemoveFrameStatusHandler_t)(SilKit_LinController* controller,
                                                                      SilKit_HandlerId handlerId);

/*! \brief The GoToSleepHandler is called whenever a go-to-sleep frame
 * was received.
 *
 * Note: The Lin controller does not automatically enter sleep
 * mode up reception of a go-to-sleep frame. I.e.,
 * SilKit_LinController_GoToSleepInternal() must be called manually
 *
 * NB: This handler will always be called, independently of the
 * \ref SilKit_LinFrameResponseMode configuration for Lin ID 0x3C. However,
 * regarding the SilKit_LinFrameStatusHandler, the go-to-sleep frame is
 * treated like every other frame, i.e. the SilKit_LinFrameStatusHandler is
 * only called for Lin ID 0x3C if configured as
 * SilKit_LinFrameResponseMode_Rx.
 * 
 * \param controller The LIN controller for which the callback should be registered.
 * \param context The user provided context pointer that is reobtained in the callback.
 * \param handler The handler to be called.
 * \param outHandlerId The handler identifier that can be used to remove the callback.
 */
SilKitAPI SilKit_ReturnCode SilKit_LinController_AddGoToSleepHandler(SilKit_LinController* controller, void* context,
                                                                      SilKit_LinGoToSleepHandler_t handler,
                                                                      SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode (*SilKit_LinController_AddGoToSleepHandler_t)(SilKit_LinController* controller, void* context,
                                                                 SilKit_LinGoToSleepHandler_t handler,
                                                                 SilKit_HandlerId* outHandlerId);

/*! \brief  Remove a \ref SilKit_LinGoToSleepHandler_t by SilKit_HandlerId on this controller 
*
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
SilKitAPI SilKit_ReturnCode SilKit_LinController_RemoveGoToSleepHandler(SilKit_LinController* controller,
                                                                         SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode (*SilKit_LinController_RemoveGoToSleepHandler_t)(SilKit_LinController* controller,
                                                                    SilKit_HandlerId handlerId);

/*! \brief The WakeupHandler is called whenever a wake up pulse is received
 *
 * Note: The Lin controller does not automatically enter
 * operational mode on wake up pulse detection. I.e.,
 * WakeInternal() must be called manually.
 * 
 * \param controller The LIN controller for which the callback should be registered.
 * \param context The user provided context pointer that is reobtained in the callback.
 * \param handler The handler to be called.
 * \param outHandlerId The handler identifier that can be used to remove the callback.
 */
SilKitAPI SilKit_ReturnCode SilKit_LinController_AddWakeupHandler(SilKit_LinController* controller, void* context,
                                                                   SilKit_LinWakeupHandler_t handler,
                                                                   SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode(*SilKit_LinController_AddWakeupHandler_t)(SilKit_LinController* controller, void* context,
                                                             SilKit_LinWakeupHandler_t handler,
                                                             SilKit_HandlerId* outHandlerId);

/*! \brief  Remove a \ref SilKit_LinWakeupHandler_t by SilKit_HandlerId on this controller 
*
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
SilKitAPI SilKit_ReturnCode SilKit_LinController_RemoveWakeupHandler(SilKit_LinController* controller,
                                                                      SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode (*SilKit_LinController_RemoveWakeupHandler_t)(SilKit_LinController* controller, SilKit_HandlerId handlerId);

SILKIT_END_DECLS

#pragma pack(pop)
