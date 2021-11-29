/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#pragma once
#include "ib/capi/InterfaceIdentifiers.h"
#include "ib/capi/Utils.h"
#include <stdint.h>

#pragma pack(push)
#pragma pack(8)

__IB_BEGIN_DECLS

//! The operational state of the controller, i.e., operational or sleeping.
typedef uint32_t ib_LinControllerStatus;
//! The controller state is not yet known.
#define ib_LinControllerStatus_Unknown ((ib_LinControllerStatus)0)
//! Normal operation
#define ib_LinControllerStatus_Operational ((ib_LinControllerStatus)1)
//! Sleep state operation; in this state wake-up detection from slave nodes
//  is enabled.
#define ib_LinControllerStatus_Sleep ((ib_LinControllerStatus)2)
//! Sleep Pending state is reached when a GoToSleep is issued.
//  This allows the network simulator to finish pending transmissions (e.g., sleep frames to slaves)
//  before entering state Sleep, cf. AUTOSAR SWS LINDriver [SWS_LIN_00266] and section 7.3.3.
//  This is only used when using detailed simulations with VIBE-NetworkSimulator.
#define ib_LinControllerStatus_SleepPending ((ib_LinControllerStatus)3)

/*! Used to configure a Lin controller as a master or slave.
 *
 *  Cf. ib_LinControllerConfig, ib_LinController_Init()
 */
typedef uint8_t ib_LinControllerMode;
/*! The Lin controller has not been configured yet and is
 *  inactive. This does not indicate sleep mode.
 */
#define ib_LinControllerMode_Inactive ((ib_LinControllerMode)0)
//! A Lin controller with active master task and slave task
#define ib_LinControllerMode_Master ((ib_LinControllerMode)1)
//! A Lin controller with only a slave task
#define ib_LinControllerMode_Slave ((ib_LinControllerMode)2)

/*! The operational baud rate of the controller.
 *
 * *Range:* 200...20'000 Bd
 */
typedef uint32_t ib_LinBaudRate;

//! \brief Controls the behavior of a Lin Slave task for a particular Lin ID
typedef uint8_t ib_LinFrameResponseMode;
//! The FrameResponse corresponding to the ID is neither received nor
//! transmitted by the Lin slave.
#define ib_LinFrameResponseMode_Unused ((ib_LinFrameResponseMode)0)
//! The FrameResponse corresponding to the ID is received by the Lin slave.
#define ib_LinFrameResponseMode_Rx ((ib_LinFrameResponseMode)1)
//! The FrameResponse corresponding to the ID is transmitted unconditionally
//! by the Lin slave.
#define ib_LinFrameResponseMode_TxUnconditional ((ib_LinFrameResponseMode)2)

/*! \brief The identifier of a Lin \ref ib_LinFrame
 *
 * This type represents all valid identifier used by \ref ib_LinController_SendFrame(), \ref
 * ib_LinController_SendFrameHeader().
 *
 * *Range:* 0...0x3F
 */
typedef uint8_t ib_LinId;

/*! \brief The checksum model of a Lin \ref ib_LinFrame
 *
 * This type is used to specify the Checksum model to be used for the Lin \ref ib_LinFrame.
 */
typedef uint8_t ib_LinChecksumModel;
#define ib_LinChecksumModel_Undefined ((ib_LinChecksumModel)0) //!< Undefined / unconfigured checksum model
#define ib_LinChecksumModel_Enhanced ((ib_LinChecksumModel)1) //!< Enhanced checksum model
#define ib_LinChecksumModel_Classic ((ib_LinChecksumModel)2) //!< Classic checksum model

/*! \brief Controls the behavior of \ref ib_LinController_SendFrame()
 *
 * Determines whether the master also provides a frame response or if the frame
 * response is expected to be provided from a slave.
 */
typedef uint8_t ib_LinFrameResponseType;
//! Response is generated from this (master) node
#define ib_LinFrameResponseType_MasterResponse ((ib_LinFrameResponseType)0)
//! Response is generated from a remote slave node
#define ib_LinFrameResponseType_SlaveResponse ((ib_LinFrameResponseType)1)
/*! Response is generated from one slave to and received by
 *  another slave, for the master the response will be anonymous,
 *  it does not have to receive the response.
 */
#define ib_LinFrameResponseType_SlaveToSlave ((ib_LinFrameResponseType)2)

/*! \brief The state of a Lin transmission
 *
 * Used to indicate the success or failure of a Lin transmission to a
 * registered \ref ib_LinFrameStatusHandler_t.
 *
 * *Note:* the enumeration values directly correspond to the AUTOSAR
 *  type Lin_StatusType. Not all values are used in the Integration
 *  Bus.
 *
 * *AUTOSAR Doc:* Lin operation states for a Lin channel or frame, as
 * returned by the API service Lin_GetStatus().
 *
 */
typedef uint8_t ib_LinFrameStatus;

/*! (currently not in use)
 */
#define ib_LinFrameStatus_NOT_OK ((ib_LinFrameStatus)0)
/*! The controller successfully transmitted a frame response.
 */
#define ib_LinFrameStatus_LIN_TX_OK ((ib_LinFrameStatus)1)
/*! (currently not in use)
 */
#define ib_LinFrameStatus_LIN_TX_BUSY ((ib_LinFrameStatus)2)
/*! (currently not in use)
 */
#define ib_LinFrameStatus_LIN_TX_HEADER_ERROR ((ib_LinFrameStatus)3)
/*! (currently not in use)
 */
#define ib_LinFrameStatus_LIN_TX_ERROR ((ib_LinFrameStatus)4)
/*! The controller received a correct frame response.
 */
#define ib_LinFrameStatus_LIN_RX_OK ((ib_LinFrameStatus)5)
/*! (currently not in use)
 */
#define ib_LinFrameStatus_LIN_RX_BUSY ((ib_LinFrameStatus)6)
/*! The reception of a response failed.
 *
 * Indicates a mismatch in expected and received data length or a checksum
 * error. Checksum errors occur when multiple slaves are configured to
 * transmit the same frame or when the sender and receiver use different
 * checksum models.
 */
#define ib_LinFrameStatus_LIN_RX_ERROR ((ib_LinFrameStatus)7)
/*! No Lin controller did provide a response to the frame header.
 */
#define ib_LinFrameStatus_LIN_RX_NO_RESPONSE ((ib_LinFrameStatus)8)

/*! \brief The data length of a Lin \ref ib_LinFrame in bytes
 *
 * This type is used to specify the number of data bytes to copy.
 *
 * *Range:* 1...8
 */
typedef uint8_t ib_LinDataLength;

/*! \brief A Lin ib_LinFrame
 *
 * This Type is used to provide Lin ID, checksum model, data length and data.
 *
 * *AUTOSAR Name:* Lin_PduType
 */
struct ib_LinFrame
{
    ib_InterfaceIdentifier interfaceId;   //!< The interface id specifying which version of this struct was obtained
    ib_LinId               id;            //!< Lin Identifier
    ib_LinChecksumModel    checksumModel; //!< Checksum Model
    ib_LinDataLength      dataLength;    //!< Data length
    uint8_t                data[8];       //!< The actual payload
};
typedef struct ib_LinFrame ib_LinFrame;

/*! \brief Configuration data for a Lin Slave task for a particular Lin ID.
 */
struct ib_LinFrameResponse
{
    ib_InterfaceIdentifier interfaceId; //!< The interface id specifying which version of this struct was obtained
    /*! frame must provide the Lin \ref ib_LinId for which the response is
     *  configured.
     *
     * If responseMode is ib_LinFrameResponseMode_TxUnconditional, the
     * frame data is used for the transaction.
     */
    ib_LinFrame frame;
    //! Determines if the FrameResponse is used for transmission
    //! (TxUnconditional), reception (Rx) or ignored (Unused).
    ib_LinFrameResponseMode responseMode;
};
typedef struct ib_LinFrameResponse ib_LinFrameResponse;

/*! Configuration data to initialize the Lin Controller
 *  Cf.: \ref ib_LinController_Init();
 */
struct ib_LinControllerConfig
{
    ib_InterfaceIdentifier interfaceId; //!< The interface id specifying which version of this struct was obtained
    //! Configure as Lin master or Lin slave
    ib_LinControllerMode controllerMode;
    /*! The operational baud rate of the controller. Only relevant for VIBE
     * simulation.
     */
    ib_LinBaudRate baudRate;

    uint32_t numFrameResponses;
    /*! Optional FrameResponse configuration.
     *
     * FrameResponses can also be configured at a later point using
     * \ref ib_LinController_SetFrameResponse() and
     * \ref ib_LinController_SetFrameResponses().
     */
    ib_LinFrameResponse  frameResponses[1];
};
typedef struct ib_LinControllerConfig ib_LinControllerConfig;

/*!
 * The LIN controller can assume the role of a LIN master or a LIN
 * slave. It provides two kinds of interfaces to perform data
 * transfers and provide frame responses:
 *
 * AUTOSAR-like LIN master interface:
 *
 * - \ref ib_LinController_SendFrame() transfers a frame from or to a LIN
 * master. Requires \ref ib_LinControllerMode_Master.
 *
 *
 * non-AUTOSAR interface:
 *
 * - \ref ib_LinController_SetFrameResponses() configures
 * the response for a particular LIN identifier. Can be used with \ref
 * ib_LinControllerMode_Master and \ref ib_LinControllerMode_Slave.
 *
 * - \ref ib_LinController_SendFrameHeader() initiates the transmission of a
 * LIN frame for a particular LIN identifier. For a successful
 * transmission, exactly one LIN slave or master must have previously
 * set a corresponding frame response for unconditional
 * transmission. Requires \ref ib_LinControllerMode_Master.
 *
 */
typedef struct ib_LinController ib_LinController;

typedef ib_ReturnCode (*ib_LinController_create_t)(ib_LinController**        outLinController,
                                                   ib_SimulationParticipant* participant, const char* name);


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
CIntegrationBusAPI ib_ReturnCode ib_LinController_create(ib_LinController**        outLinController,
                                                         ib_SimulationParticipant* participant, const char* name);

typedef ib_ReturnCode (*ib_LinController_Init_t)(ib_LinController* controller, const ib_LinControllerConfig* config);

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
CIntegrationBusAPI ib_ReturnCode ib_LinController_Init(ib_LinController* controller, const ib_LinControllerConfig* config);

typedef ib_ReturnCode (*ib_LinController_Status_t)(ib_LinController* controller, ib_LinControllerStatus* outStatus);

/*! \brief Get the current status of the Lin Controller, i.e., Operational or Sleep.
 *
 * \param controller The Lin controller to retrieve the status
 * \param outStatus Pointer into which the status will be written (out parameter).
 *
 */
CIntegrationBusAPI ib_ReturnCode ib_LinController_Status(ib_LinController* controller, ib_LinControllerStatus* outStatus);

typedef ib_ReturnCode (*ib_LinController_SendFrame_t)(ib_LinController* controller, const ib_LinFrame* frame,
                                                      ib_LinFrameResponseType responseType);

/*! \brief AUTOSAR LIN master interface
 *
 * Perform a full Lin data transfer, i.e., frame header + frame
 * response. The responseType determines if frame.data is used for
 * the frame response or if a different node has to provide it:
 *
 * \li MasterResponse: \ref ib_LinFrame is sent from this controller to
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
CIntegrationBusAPI ib_ReturnCode ib_LinController_SendFrame(ib_LinController* controller, const ib_LinFrame* frame,
                                                            ib_LinFrameResponseType responseType);

typedef ib_ReturnCode (*ib_LinController_SendFrameWithTimestamp_t)(ib_LinController* controller, const ib_LinFrame* frame,
                                                                   ib_LinFrameResponseType responseType,
                                                                   ib_NanosecondsTime timestamp);

/*! \brief AUTOSAR LIN master interface
 *
 * Overload to \ref ib_LinController_SendFrame(ib_LinController* controller, ib_LinFrame frame, ib_LinFrameResponseType
 * responseType) with additional timestamp for the reception time of a Lin transmission.
 *
 * \param timestamp This time stamp value is passed to the configured
 * ib_LinFrameStatusHandler_t unless a VIBE NetworkSimulator is used. When using
 * VIBE NetworkSimulator, precise timestamps are always generated by the
 * NetworkSimulator.
 */
CIntegrationBusAPI ib_ReturnCode ib_LinController_SendFrameWithTimestamp(ib_LinController* controller, const ib_LinFrame* frame,
                                                                         ib_LinFrameResponseType responseType,
                                                                         ib_NanosecondsTime      timestamp);

typedef ib_ReturnCode(*ib_LinController_SendFrameHeader_t)(ib_LinController* controller, ib_LinId linId);

//! Send Interface for a non-AUTOSAR Lin Master
CIntegrationBusAPI ib_ReturnCode ib_LinController_SendFrameHeader(ib_LinController* controller, ib_LinId linId);

typedef ib_ReturnCode (*ib_LinController_SendFrameHeaderWithTimestamp_t)(ib_LinController* controller, ib_LinId linId,
                                                            ib_NanosecondsTime timestamp);
/*! Send Interface for a non-AUTOSAR Lin Master
 *
 * Overload to \ref ib_LinController_SendFrameHeader(ib_LinController* controller, ib_LinId linId) with additional timestamp
 * for the reception time of a Lin transmission.
 *
 * \param timestamp This time stamp value is passed to the configured
 * ib_LinFrameStatusHandler_t unless a VIBE NetworkSimulator is used. When using
 * VIBE NetworkSimulator, precise timestamps are always generated by the
 * NetworkSimulator.
 */
CIntegrationBusAPI ib_ReturnCode ib_LinController_SendFrameHeaderWithTimestamp(ib_LinController* controller, ib_LinId linId,
                                                                  ib_NanosecondsTime timestamp);

typedef ib_ReturnCode(*ib_LinController_SetFrameResponse_t)(ib_LinController* controller, const ib_LinFrame* frame,
                                                                   ib_LinFrameResponseMode mode);
/*! FrameResponse configuration for Slaves or non-AUTOSAR Lin
 *  Masters The corresponding Lin ID does not need to be
 *  previously configured. */
CIntegrationBusAPI ib_ReturnCode ib_LinController_SetFrameResponse(ib_LinController* controller, const ib_LinFrame* frame,
                                                                   ib_LinFrameResponseMode mode);

typedef ib_ReturnCode (*ib_LinController_SetFrameResponses_t)(ib_LinController*    controller,
                                                              const ib_LinFrameResponse* frameResponses,
                                                              uint32_t                   numFrameResponses);
/*! FrameResponse configuration for Slaves or non-AUTOSAR Lin Masters.
 *
 * Configures multiple responses at once. Corresponding IDs do not
 * need to be previously configured.
 *
 * NB: only configures responses for the provided Lin IDs. I.e.,
 * an empty vector does not clear or reset the currently
 * configured FrameResponses.
 */
CIntegrationBusAPI ib_ReturnCode ib_LinController_SetFrameResponses(ib_LinController*    controller,
                                                                    const ib_LinFrameResponse* frameResponses,
                                                                    uint32_t                   numFrameResponses);

typedef ib_ReturnCode(*ib_LinController_GoToSleep_t)(ib_LinController* controller);
/*! \brief Transmit a go-to-sleep-command and set ib_LinControllerStatus_Sleep and enable wake-up
 *
 * *AUTOSAR Name:* Lin_GoToSleep
 */
CIntegrationBusAPI ib_ReturnCode ib_LinController_GoToSleep(ib_LinController* controller);

typedef ib_ReturnCode(*ib_LinController_GoToSleepInternal_t)(ib_LinController* controller);
/*! \brief Set ib_LinControllerStatus_Sleep without sending a go-to-sleep command.
 *
 * *AUTOSAR Name:* Lin_GoToSleepInternal
 */
CIntegrationBusAPI ib_ReturnCode ib_LinController_GoToSleepInternal(ib_LinController* controller);

typedef ib_ReturnCode(*ib_LinController_Wakeup_t)(ib_LinController* controller);
/*! \brief Generate a wake up pulse and set ib_LinControllerStatus_Operational.
 *
 * *AUTOSAR Name:* Lin_Wakeup
 */
CIntegrationBusAPI ib_ReturnCode ib_LinController_Wakeup(ib_LinController* controller);

typedef ib_ReturnCode(*ib_LinController_WakeupInternal_t)(ib_LinController* controller);
/*! Set ib_LinControllerStatus_Operational without generating a wake up pulse.
 *
 * *AUTOSAR Name:* Lin_WakeupInternal
 */
CIntegrationBusAPI ib_ReturnCode ib_LinController_WakeupInternal(ib_LinController* controller);

/*! Callback type to indicate the end of a Lin Frame transmission.
 * \param context The context provided by the user on registration.
 * \param controller The Lin controller that received the acknowledge.
 * \param frame A reference to the corresponding frame.
 * \param status The state of a Lin transmission.
 * \param timestamp The timestamp of the Lin transmission
 */
typedef void (*ib_LinFrameStatusHandler_t)(void* context, ib_LinController* controller, const ib_LinFrame* frame,
                                        ib_LinFrameStatus status, ib_NanosecondsTime timestamp);

typedef ib_ReturnCode(*ib_LinController_RegisterFrameStatusHandler_t)(ib_LinController* controller, void* context,
                                                                             ib_LinFrameStatusHandler_t handler);

/*! \brief Reports the \ref ib_LinFrameStatus of a Lin \ref ib_LinFrame
 * transmission and provides the transmitted frame.
 *
 * The \ref ib_LinFrameStatusHandler_t is called once per call to
 * \ref ib_LinController_SendFrame() or call to
 * \ref ib_LinController_SendFrameHeader(). The handler is called independently
 * of the transmission's success or failure.
 *
 * The ib_LinFrameStatusHandler_t is called for all participating Lin
 * controllers. I.e., for Lin masters, it is always called, and
 * for Lin slaves, it is called if the corresponding \ref ib_LinId is
 * configured ib_LinFrameResponseMode_Rx or
 * ib_LinFrameResponseModeTxUnconditional.
 *
 * <em>Note: this is one of the major changes to the previous version.
 * Previously, frame transmission was indicated using different
 * means. For Masters, a TX was confirmed using the
 * TxCompleteHandler while an RX was handled using
 * ReceiveMessageHandler. For Lin slaves the confirmation varied
 * for simple simulation and VIBE simulation.</em>
 */
CIntegrationBusAPI ib_ReturnCode ib_LinController_RegisterFrameStatusHandler(ib_LinController* controller, void* context,
                                                                             ib_LinFrameStatusHandler_t handler);

/*! Callback type to indicate that a go-to-sleep frame has been received.
 *  Cf., \ref ib_LinController_RegisterGoToSleepHandler();
 */
typedef void (*ib_LinGoToSleepHandler_t)(void* context, ib_LinController* controller);

typedef ib_ReturnCode(*ib_LinController_RegisterGoToSleepHandler_t)(ib_LinController* controller, void* context,
                                                                           ib_LinGoToSleepHandler_t handler);

/*! \brief The GoToSleepHandler is called whenever a go-to-sleep frame
 * was received.
 *
 * Note: The Lin controller does not automatically enter sleep
 * mode up reception of a go-to-sleep frame. I.e.,
 * ib_LinController_GoToSleepInternal() must be called manually
 *
 * NB: This handler will always be called, independently of the
 * \ref ib_LinFrameResponseMode configuration for Lin ID 0x3C. However,
 * regarding the ib_LinFrameStatusHandler, the go-to-sleep frame is
 * treated like every other frame, i.e. the ib_LinFrameStatusHandler is
 * only called for Lin ID 0x3C if configured as
 * ib_LinFrameResponseMode_Rx.
 */
CIntegrationBusAPI ib_ReturnCode ib_LinController_RegisterGoToSleepHandler(ib_LinController* controller, void* context,
                                                                           ib_LinGoToSleepHandler_t handler);

/*! Callback type to indicate that a wakeup pulse has been received.
 *  Cf., \ref RegisterWakeupHandler(WakeupHandler);
 */
typedef void (*ib_LinWakeupHandler_t)(void* context, ib_LinController* controller);

typedef ib_ReturnCode(*ib_LinController_RegisterWakeupHandler_t)(ib_LinController* controller, void* context,
                                                                        ib_LinWakeupHandler_t handler);

/*! \brief The WakeupHandler is called whenever a wake up pulse is received
 *
 * Note: The Lin controller does not automatically enter
 * operational mode on wake up pulse detection. I.e.,
 * WakeInternal() must be called manually.
 */
CIntegrationBusAPI ib_ReturnCode ib_LinController_RegisterWakeupHandler(ib_LinController* controller, void* context,
                                                                        ib_LinWakeupHandler_t handler);

__IB_END_DECLS

#pragma pack(pop)
