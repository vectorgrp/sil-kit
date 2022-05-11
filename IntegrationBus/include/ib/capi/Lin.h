/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#pragma once
#include <stdint.h>
#include "ib/capi/IbMacros.h"
#include "ib/capi/Types.h"
#include "ib/capi/InterfaceIdentifiers.h"

#pragma pack(push)
#pragma pack(8)

IB_BEGIN_DECLS

//! The operational state of the controller, i.e., operational or sleeping.
typedef uint32_t ib_Lin_ControllerStatus;
//! The controller state is not yet known.
#define ib_Lin_ControllerStatus_Unknown ((ib_Lin_ControllerStatus)0)
//! Normal operation
#define ib_Lin_ControllerStatus_Operational ((ib_Lin_ControllerStatus)1)
//! Sleep state operation; in this state wake-up detection from slave nodes
//  is enabled.
#define ib_Lin_ControllerStatus_Sleep ((ib_Lin_ControllerStatus)2)
//! Sleep Pending state is reached when a GoToSleep is issued.
//  This allows the network simulator to finish pending transmissions (e.g., sleep frames to slaves)
//  before entering state Sleep, cf. AUTOSAR SWS LINDriver [SWS_LIN_00266] and section 7.3.3.
//  This is only used when using detailed simulations with VIBE-NetworkSimulator.
#define ib_Lin_ControllerStatus_SleepPending ((ib_Lin_ControllerStatus)3)

/*! Used to configure a Lin controller as a master or slave.
 *
 *  Cf. ib_Lin_ControllerConfig, ib_Lin_Controller_Init()
 */
typedef uint8_t ib_Lin_ControllerMode;
/*! The Lin controller has not been configured yet and is
 *  inactive. This does not indicate sleep mode.
 */
#define ib_Lin_ControllerMode_Inactive ((ib_Lin_ControllerMode)0)
//! A Lin controller with active master task and slave task
#define ib_Lin_ControllerMode_Master ((ib_Lin_ControllerMode)1)
//! A Lin controller with only a slave task
#define ib_Lin_ControllerMode_Slave ((ib_Lin_ControllerMode)2)

/*! The operational baud rate of the controller.
 *
 * *Range:* 200...20'000 Bd
 */
typedef uint32_t ib_Lin_BaudRate;

//! \brief Controls the behavior of a Lin Slave task for a particular Lin ID
typedef uint8_t ib_Lin_FrameResponseMode;
//! The LinFrameResponse corresponding to the ID is neither received nor
//! transmitted by the Lin slave.
#define ib_Lin_FrameResponseMode_Unused ((ib_Lin_FrameResponseMode)0)
//! The LinFrameResponse corresponding to the ID is received by the Lin slave.
#define ib_Lin_FrameResponseMode_Rx ((ib_Lin_FrameResponseMode)1)
//! The LinFrameResponse corresponding to the ID is transmitted unconditionally
//! by the Lin slave.
#define ib_Lin_FrameResponseMode_TxUnconditional ((ib_Lin_FrameResponseMode)2)

/*! \brief The identifier of a Lin \ref ib_Lin_Frame
 *
 * This type represents all valid identifier used by \ref ib_Lin_Controller_SendFrame(), \ref
 * ib_Lin_Controller_SendFrameHeader().
 *
 * *Range:* 0...0x3F
 */
typedef uint8_t ib_Lin_Id;

/*! \brief The checksum model of a Lin \ref ib_Lin_Frame
 *
 * This type is used to specify the Checksum model to be used for the Lin \ref ib_Lin_Frame.
 */
typedef uint8_t ib_Lin_ChecksumModel;
#define ib_Lin_ChecksumModel_Undefined ((ib_Lin_ChecksumModel)0) //!< Undefined / unconfigured checksum model
#define ib_Lin_ChecksumModel_Enhanced ((ib_Lin_ChecksumModel)1) //!< Enhanced checksum model
#define ib_Lin_ChecksumModel_Classic ((ib_Lin_ChecksumModel)2) //!< Classic checksum model

/*! \brief Controls the behavior of \ref ib_Lin_Controller_SendFrame()
 *
 * Determines whether the master also provides a frame response or if the frame
 * response is expected to be provided from a slave.
 */
typedef uint8_t ib_Lin_FrameResponseType;
//! Response is generated from this (master) node
#define ib_Lin_FrameResponseType_MasterResponse ((ib_Lin_FrameResponseType)0)
//! Response is generated from a remote slave node
#define ib_Lin_FrameResponseType_SlaveResponse ((ib_Lin_FrameResponseType)1)
/*! Response is generated from one slave to and received by
 *  another slave, for the master the response will be anonymous,
 *  it does not have to receive the response.
 */
#define ib_Lin_FrameResponseType_SlaveToSlave ((ib_Lin_FrameResponseType)2)

/*! \brief The state of a Lin transmission
 *
 * Used to indicate the success or failure of a Lin transmission to a
 * registered \ref ib_Lin_FrameStatusHandler_t.
 *
 * *Note:* the enumeration values directly correspond to the AUTOSAR
 *  type Lin_StatusType. Not all values are used in the Integration
 *  Bus.
 *
 * *AUTOSAR Doc:* Lin operation states for a Lin channel or frame, as
 * returned by the API service Lin_GetStatus().
 *
 */
typedef uint8_t ib_Lin_FrameStatus;

/*! (currently not in use)
 */
#define ib_Lin_FrameStatus_NOT_OK              ((ib_Lin_FrameStatus)0)
/*! The controller successfully transmitted a frame response.
 */
#define ib_Lin_FrameStatus_LIN_TX_OK           ((ib_Lin_FrameStatus)1)
/*! (currently not in use)
 */
#define ib_Lin_FrameStatus_LIN_TX_BUSY         ((ib_Lin_FrameStatus)2)
/*! (currently not in use)
 */
#define ib_Lin_FrameStatus_LIN_TX_HEADER_ERROR ((ib_Lin_FrameStatus)3)
/*! (currently not in use)
 */
#define ib_Lin_FrameStatus_LIN_TX_ERROR        ((ib_Lin_FrameStatus)4)
/*! The controller received a correct frame response.
 */
#define ib_Lin_FrameStatus_LIN_RX_OK           ((ib_Lin_FrameStatus)5)
/*! (currently not in use)
 */
#define ib_Lin_FrameStatus_LIN_RX_BUSY         ((ib_Lin_FrameStatus)6)
/*! The reception of a response failed.
 *
 * Indicates a mismatch in expected and received data length or a checksum
 * error. Checksum errors occur when multiple slaves are configured to
 * transmit the same frame or when the sender and receiver use different
 * checksum models.
 */
#define ib_Lin_FrameStatus_LIN_RX_ERROR        ((ib_Lin_FrameStatus)7)
/*! No Lin controller did provide a response to the frame header.
 */
#define ib_Lin_FrameStatus_LIN_RX_NO_RESPONSE  ((ib_Lin_FrameStatus)8)

/*! \brief The data length of a Lin \ref ib_Lin_Frame in bytes
 *
 * This type is used to specify the number of data bytes to copy.
 *
 * *Range:* 1...8
 */
typedef uint8_t ib_Lin_DataLength;

/*! \brief A Lin ib_Lin_Frame
 *
 * This Type is used to provide Lin ID, checksum model, data length and data.
 *
 * *AUTOSAR Name:* Lin_PduType
 */
struct ib_Lin_Frame
{
    ib_InterfaceIdentifier interfaceId;   //!< The interface id specifying which version of this struct was obtained
    ib_Lin_Id               id;            //!< Lin Identifier
    ib_Lin_ChecksumModel    checksumModel; //!< Checksum Model
    ib_Lin_DataLength       dataLength;    //!< Data length
    uint8_t                data[8];       //!< The actual payload
};
typedef struct ib_Lin_Frame ib_Lin_Frame;

//! \brief A LIN frame status event delivered in the \ref ib_Lin_FrameStatusHandler_t.
//! 
struct ib_Lin_FrameStatusEvent
{
    ib_InterfaceIdentifier interfaceId; //!< The interface id specifying which version of this struct was obtained
    ib_NanosecondsTime timestamp; //!< Time of the event.
    ib_Lin_Frame* frame;
    ib_Lin_FrameStatus status;
};
typedef struct ib_Lin_FrameStatusEvent ib_Lin_FrameStatusEvent;

//! \brief A LIN wakeup event delivered in the \ref ib_Lin_WakeupHandler_t.
struct ib_Lin_WakeupEvent
{
    ib_InterfaceIdentifier interfaceId; //!< The interface id specifying which version of this struct was obtained
    ib_NanosecondsTime timestamp; //!< Time of the event.
    ib_Direction direction; //!< The direction of the event.
};
typedef struct ib_Lin_WakeupEvent ib_Lin_WakeupEvent;

//! \brief A LIN goToSleep event delivered in the \ref ib_Lin_GoToSleepHandler_t
struct ib_Lin_GoToSleepEvent
{
    ib_InterfaceIdentifier interfaceId; //!< The interface id specifying which version of this struct was obtained
    ib_NanosecondsTime timestamp; //!< Time of the event.
};
typedef struct ib_Lin_GoToSleepEvent ib_Lin_GoToSleepEvent;

/*! \brief Configuration data for a Lin Slave task for a particular Lin ID.
 */
struct ib_Lin_FrameResponse
{
    ib_InterfaceIdentifier interfaceId; //!< The interface id specifying which version of this struct was obtained
    /*! frame must provide the Lin \ref ib_Lin_Id for which the response is
     *  configured.
     *
     * If responseMode is ib_Lin_FrameResponseMode_TxUnconditional, the
     * frame data is used for the transaction.
     */
    ib_Lin_Frame* frame;
    //! Determines if the LinFrameResponse is used for transmission
    //! (TxUnconditional), reception (Rx) or ignored (Unused).
    ib_Lin_FrameResponseMode responseMode;
};
typedef struct ib_Lin_FrameResponse ib_Lin_FrameResponse;

/*! Configuration data to initialize the Lin Controller
 *  Cf.: \ref ib_Lin_Controller_Init();
 */
struct ib_Lin_ControllerConfig
{
    ib_InterfaceIdentifier interfaceId; //!< The interface id specifying which version of this struct was obtained
    //! Configure as Lin master or Lin slave
    ib_Lin_ControllerMode controllerMode;
    /*! The operational baud rate of the controller. Only relevant for VIBE
     * simulation.
     */
    ib_Lin_BaudRate baudRate;

    uint32_t numFrameResponses;
    /*! Optional LinFrameResponse configuration.
     *
     * FrameResponses can also be configured at a later point using
     * \ref ib_Lin_Controller_SetFrameResponse() and
     * \ref ib_Lin_Controller_SetFrameResponses().
     */
    ib_Lin_FrameResponse*  frameResponses;
};
typedef struct ib_Lin_ControllerConfig ib_Lin_ControllerConfig;

/*!
 * The LIN controller can assume the role of a LIN master or a LIN
 * slave. It provides two kinds of interfaces to perform data
 * transfers and provide frame responses:
 *
 * AUTOSAR-like LIN master interface:
 *
 * - \ref ib_Lin_Controller_SendFrame() transfers a frame from or to a LIN
 * master. Requires \ref ib_Lin_ControllerMode_Master.
 *
 *
 * non-AUTOSAR interface:
 *
 * - \ref ib_Lin_Controller_SetFrameResponses() configures
 * the response for a particular LIN identifier. Can be used with \ref
 * ib_Lin_ControllerMode_Master and \ref ib_Lin_ControllerMode_Slave.
 *
 * - \ref ib_Lin_Controller_SendFrameHeader() initiates the transmission of a
 * LIN frame for a particular LIN identifier. For a successful
 * transmission, exactly one LIN slave or master must have previously
 * set a corresponding frame response for unconditional
 * transmission. Requires \ref ib_Lin_ControllerMode_Master.
 *
 */
typedef struct ib_Lin_Controller ib_Lin_Controller;

/*! Callback type to indicate the end of a Lin Frame transmission.
 * \param context The context provided by the user on registration.
 * \param controller The Lin controller that received the acknowledge.
 * \param frameStatusEvent The event containing a timestamp, the corresponding frame and the new status.
 */
typedef void (*ib_Lin_FrameStatusHandler_t)(void* context, ib_Lin_Controller* controller,
                                            const ib_Lin_FrameStatusEvent* frameStatusEvent);

/*! Callback type to indicate that a go-to-sleep frame has been received.
 *  Cf., \ref ib_Lin_Controller_AddGoToSleepHandler();
 */
typedef void (*ib_Lin_GoToSleepHandler_t)(void* context, ib_Lin_Controller* controller,
                                          const ib_Lin_GoToSleepEvent* goToSleepEvent);

/*! Callback type to indicate that a wakeup pulse has been received.
 *  Cf., \ref AddWakeupHandler(WakeupHandler);
 */
typedef void (*ib_Lin_WakeupHandler_t)(void* context, ib_Lin_Controller* controller,
                                       const ib_Lin_WakeupEvent* wakeUpEvent);

/*! \brief Create a Lin controller at this IB simulation participant.
 *
 * The lifetime of the resulting Lin controller is directly bound to the lifetime of the simulation participant.
 * There is no further cleanup necessary except for destroying the simulation participant at the end of the
 * simulation.
 *
 * \param outLinController Pointer into which the resulting Lin controller will be written (out parameter).
 * \param participant The simulation participant at which the Lin controller should be created.
 * \param name The name of the new Lin controller.
 *
 * \return \ref ib_ReturnCode
 */
IntegrationBusAPI ib_ReturnCode ib_Lin_Controller_Create(
  ib_Lin_Controller** outLinController,
  ib_Participant *participant,
  const char* name,
  const char* network);

typedef ib_ReturnCode (*ib_Lin_Controller_Create_t)(
  ib_Lin_Controller** outLinController,
  ib_Participant* participant, 
  const char* name,
  const char* network);


/*! \brief Initialize the Lin controller
 *
 * \param controller The Lin controller to initialize
 * \param config The Controller configuration contains:
 *  - controllerMode, either sets Lin master or Lin slave mode
 *  - baudRate, determine transmission speeds (only used for VIBE simulation)
 *  - frameResponses, an optional set of initial FrameResponses
 * 
 * *AUTOSAR Name:* Lin_Init
 */
IntegrationBusAPI ib_ReturnCode ib_Lin_Controller_Init(ib_Lin_Controller* controller, const ib_Lin_ControllerConfig* config);

typedef ib_ReturnCode (*ib_Lin_Controller_Init_t)(ib_Lin_Controller* controller, const ib_Lin_ControllerConfig* config);

/*! \brief Get the current status of the Lin Controller, i.e., Operational or Sleep.
 *
 * \param controller The Lin controller to retrieve the status
 * \param outStatus Pointer into which the status will be written (out parameter).
 *
 */
IntegrationBusAPI ib_ReturnCode ib_Lin_Controller_Status(ib_Lin_Controller* controller, ib_Lin_ControllerStatus* outStatus);

typedef ib_ReturnCode (*ib_Lin_Controller_Status_t)(ib_Lin_Controller* controller, ib_Lin_ControllerStatus* outStatus);

/*! \brief AUTOSAR LIN master interface
 *
 * Perform a full Lin data transfer, i.e., frame header + frame
 * response. The responseType determines if frame.data is used for
 * the frame response or if a different node has to provide it:
 *
 * \li MasterResponse: \ref ib_Lin_Frame is sent from this controller to
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
IntegrationBusAPI ib_ReturnCode ib_Lin_Controller_SendFrame(ib_Lin_Controller* controller, const ib_Lin_Frame* frame,
                                                            ib_Lin_FrameResponseType responseType);

typedef ib_ReturnCode (*ib_Lin_Controller_SendFrame_t)(ib_Lin_Controller* controller, const ib_Lin_Frame* frame,
                                                      ib_Lin_FrameResponseType responseType);

//! Send Interface for a non-AUTOSAR Lin Master
IntegrationBusAPI ib_ReturnCode ib_Lin_Controller_SendFrameHeader(ib_Lin_Controller* controller, ib_Lin_Id linId);

typedef ib_ReturnCode(*ib_Lin_Controller_SendFrameHeader_t)(ib_Lin_Controller* controller, ib_Lin_Id linId);

/*! LinFrameResponse configuration for Slaves or non-AUTOSAR Lin
 *  Masters The corresponding Lin ID does not need to be
 *  previously configured. */
IntegrationBusAPI ib_ReturnCode ib_Lin_Controller_SetFrameResponse(ib_Lin_Controller* controller,
                                                                   const ib_Lin_FrameResponse* frameResponse);

typedef ib_ReturnCode (*ib_Lin_Controller_SetFrameResponse_t)(ib_Lin_Controller* controller,
                                                              const ib_Lin_FrameResponse* frameResponse);

/*! LinFrameResponse configuration for Slaves or non-AUTOSAR Lin Masters.
 *
 * Configures multiple responses at once. Corresponding IDs do not
 * need to be previously configured.
 *
 * NB: only configures responses for the provided Lin IDs. I.e.,
 * an empty vector does not clear or reset the currently
 * configured FrameResponses.
 */
IntegrationBusAPI ib_ReturnCode ib_Lin_Controller_SetFrameResponses(ib_Lin_Controller*    controller,
                                                                    const ib_Lin_FrameResponse* frameResponses,
                                                                    uint32_t                   numFrameResponses);
typedef ib_ReturnCode (*ib_Lin_Controller_SetFrameResponses_t)(ib_Lin_Controller*    controller,
                                                              const ib_Lin_FrameResponse* frameResponses,
                                                              uint32_t                   numFrameResponses);

/*! \brief Transmit a go-to-sleep-command and set ib_Lin_ControllerStatus_Sleep and enable wake-up
 *
 * *AUTOSAR Name:* Lin_GoToSleep
 * \return ib_ReturnCode_SUCCESS or ib_ReturnCode_WRONGSTATE if issued with wrong \ref ib_Lin_ControllerMode
 */
IntegrationBusAPI ib_ReturnCode ib_Lin_Controller_GoToSleep(ib_Lin_Controller* controller);

typedef ib_ReturnCode(*ib_Lin_Controller_GoToSleep_t)(ib_Lin_Controller* controller);
/*! \brief Set ib_Lin_ControllerStatus_Sleep without sending a go-to-sleep command.
 *
 * *AUTOSAR Name:* Lin_GoToSleepInternal
 * \return ib_ReturnCode_SUCCESS or ib_ReturnCode_WRONGSTATE if issued with wrong \ref ib_Lin_ControllerMode
 */
IntegrationBusAPI ib_ReturnCode ib_Lin_Controller_GoToSleepInternal(ib_Lin_Controller* controller);

typedef ib_ReturnCode(*ib_Lin_Controller_GoToSleepInternal_t)(ib_Lin_Controller* controller);
/*! \brief Generate a wake up pulse and set ib_Lin_ControllerStatus_Operational.
 *
 * *AUTOSAR Name:* Lin_Wakeup
 * \return ib_ReturnCode_SUCCESS or ib_ReturnCode_WRONGSTATE if issued with wrong \ref ib_Lin_ControllerMode
 */
IntegrationBusAPI ib_ReturnCode ib_Lin_Controller_Wakeup(ib_Lin_Controller* controller);

typedef ib_ReturnCode(*ib_Lin_Controller_Wakeup_t)(ib_Lin_Controller* controller);
/*! Set ib_Lin_ControllerStatus_Operational without generating a wake up pulse.
 *
 * *AUTOSAR Name:* Lin_WakeupInternal
 * \return ib_ReturnCode_SUCCESS or ib_ReturnCode_WRONGSTATE if issued with wrong \ref ib_Lin_ControllerMode
 */
IntegrationBusAPI ib_ReturnCode ib_Lin_Controller_WakeupInternal(ib_Lin_Controller* controller);

typedef ib_ReturnCode(*ib_Lin_Controller_WakeupInternal_t)(ib_Lin_Controller* controller);

/*! \brief Reports the \ref ib_Lin_FrameStatus of a Lin \ref ib_Lin_Frame
 * transmission and provides the transmitted frame.
 *
 * The \ref ib_Lin_FrameStatusHandler_t is called once per call to
 * \ref ib_Lin_Controller_SendFrame() or call to
 * \ref ib_Lin_Controller_SendFrameHeader(). The handler is called independently
 * of the transmission's success or failure.
 *
 * The ib_Lin_FrameStatusHandler_t is called for all participating Lin
 * controllers. I.e., for Lin masters, it is always called, and
 * for Lin slaves, it is called if the corresponding \ref ib_Lin_Id is
 * configured ib_Lin_FrameResponseMode_Rx or
 * ib_Lin_FrameResponseModeTxUnconditional.
 *
 * <em>Note: this is one of the major changes to the previous version.
 * Previously, frame transmission was indicated using different
 * means. For Masters, a TX was confirmed using the
 * TxCompleteHandler while an RX was handled using
 * ReceiveMessageHandler. For Lin slaves the confirmation varied
 * for simple simulation and VIBE simulation.</em>
 */
IntegrationBusAPI ib_ReturnCode ib_Lin_Controller_AddFrameStatusHandler(ib_Lin_Controller* controller, void* context,
                                                                             ib_Lin_FrameStatusHandler_t handler);

typedef ib_ReturnCode(*ib_Lin_Controller_AddFrameStatusHandler_t)(ib_Lin_Controller* controller, void* context,
                                                                             ib_Lin_FrameStatusHandler_t handler);

/*! \brief The GoToSleepHandler is called whenever a go-to-sleep frame
 * was received.
 *
 * Note: The Lin controller does not automatically enter sleep
 * mode up reception of a go-to-sleep frame. I.e.,
 * ib_Lin_Controller_GoToSleepInternal() must be called manually
 *
 * NB: This handler will always be called, independently of the
 * \ref ib_Lin_FrameResponseMode configuration for Lin ID 0x3C. However,
 * regarding the ib_Lin_FrameStatusHandler, the go-to-sleep frame is
 * treated like every other frame, i.e. the ib_Lin_FrameStatusHandler is
 * only called for Lin ID 0x3C if configured as
 * ib_Lin_FrameResponseMode_Rx.
 */
IntegrationBusAPI ib_ReturnCode ib_Lin_Controller_AddGoToSleepHandler(ib_Lin_Controller* controller, void* context,
                                                                           ib_Lin_GoToSleepHandler_t handler);

typedef ib_ReturnCode(*ib_Lin_Controller_AddGoToSleepHandler_t)(ib_Lin_Controller* controller, void* context,
                                                                           ib_Lin_GoToSleepHandler_t handler);

/*! \brief The WakeupHandler is called whenever a wake up pulse is received
 *
 * Note: The Lin controller does not automatically enter
 * operational mode on wake up pulse detection. I.e.,
 * WakeInternal() must be called manually.
 */
IntegrationBusAPI ib_ReturnCode ib_Lin_Controller_AddWakeupHandler(ib_Lin_Controller* controller, void* context,
                                                                        ib_Lin_WakeupHandler_t handler);

typedef ib_ReturnCode(*ib_Lin_Controller_AddWakeupHandler_t)(ib_Lin_Controller* controller, void* context,
                                                                        ib_Lin_WakeupHandler_t handler);

IB_END_DECLS

#pragma pack(pop)
