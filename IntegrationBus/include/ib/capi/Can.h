/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#pragma once
#include <stdint.h>
#include "ib/capi/IbMacros.h"
#include "ib/capi/Types.h"
#include "ib/capi/InterfaceIdentifiers.h"

#pragma pack(push)
#pragma pack(8)

IB_BEGIN_DECLS

/*! \brief A CAN frame
    */
struct ib_Can_Frame
{
    uint32_t id; //!< CAN Identifier
    uint32_t flags; //!< CAN Arbitration and Control Field Flags; see ib_Can_MessageFlag
    uint8_t dlc; //!< Data Length Code - determined by the Network Simulator

    ib_ByteVector data; //!< Data field containing the payload
};

typedef struct ib_Can_Frame ib_Can_Frame;

struct ib_Can_Message
{
    ib_InterfaceIdentifier interfaceId; //!< The interface id specifying which version of this struct was obtained
    ib_NanosecondsTime timestamp; //!< Reception time
    ib_Can_Frame* canFrame; //!< The CAN Frame that corresponds to the meta data
    ib_Direction direction; //!< The transmit direction of the CAN frame (TX/RX)
    void* userContext; //!< Optional pointer provided by user when sending the frame
};

typedef struct ib_Can_Message ib_Can_Message;

/*! The available flags within the flags member of a Can frame.
*/
typedef uint32_t ib_Can_FrameFlag;
#define ib_Can_FrameFlag_ide (((ib_Can_FrameFlag) 1) << 9) //!< Identifier Extension
#define ib_Can_FrameFlag_rtr (((ib_Can_FrameFlag) 1) << 4) //!< Remote Transmission Request
#define ib_Can_FrameFlag_fdf (((ib_Can_FrameFlag) 1) << 12) //!< FD Format Indicator
#define ib_Can_FrameFlag_brs (((ib_Can_FrameFlag) 1) << 13) //!< Bit Rate Switch  (for FD Format only)
#define ib_Can_FrameFlag_esi (((ib_Can_FrameFlag) 1) << 14) //!< Error State indicator (for FD Format only)


typedef int32_t ib_Can_TransmitStatus;
/*! The message was successfully transmitted on the CAN bus.
*/
#define ib_Can_TransmitStatus_Transmitted          ((ib_Can_TransmitStatus) 1)
/*! (currently not in use)
*
* The transmit queue was reset.
*/
#define ib_Can_TransmitStatus_Canceled             ((ib_Can_TransmitStatus) 2)
/*! The transmit request was rejected, because the transmit queue is full.
*/
#define ib_Can_TransmitStatus_TransmitQueueFull    ((ib_Can_TransmitStatus) 4)
/*! (currently not in use)
*
* The transmit request was rejected, because there is already another request with the same transmitId.
*/
#define ib_Can_TransmitStatus_DuplicatedTransmitId ((ib_Can_TransmitStatus) 8)

/*! \brief The acknowledgment of a CAN message, sent to the controller
*/
struct ib_Can_TransmitAcknowledge
{
    void* userContext; //!< Value that was provided by user in corresponding parameter on send of Can frame
    ib_NanosecondsTime timestamp; //!< Reception time
    ib_Can_TransmitStatus status; //!< Status of the CanTransmitRequest
};

typedef struct ib_Can_TransmitAcknowledge ib_Can_TransmitAcknowledge;

/*! \brief CAN Controller state according to AUTOSAR specification AUTOSAR_SWS_CANDriver 4.3.1
*/
typedef int32_t ib_Can_ControllerState;
/*! CAN controller is not initialized (initial state after reset).
*/
#define ib_Can_ControllerState_Uninit  ((ib_Can_ControllerState) 0)
/*! CAN controller is initialized but does not participate on the CAN bus.
*/
#define ib_Can_ControllerState_Stopped ((ib_Can_ControllerState) 1)
/*! CAN controller is in normal operation mode.
*/
#define ib_Can_ControllerState_Started ((ib_Can_ControllerState) 2)
/*! CAN controller is in sleep mode which is similar to the Stopped state.
*/
#define ib_Can_ControllerState_Sleep   ((ib_Can_ControllerState) 3)

/*! \brief Error state of a CAN node according to CAN specification.
*/
typedef int ib_Can_ErrorState;
/*! Error State is Not Available, because CAN controller is in state Uninit.
*
* *AUTOSAR Doc:* Successful transmission.
*/
#define ib_Can_ErrorState_NotAvailable ((ib_Can_ControllerState) 0)
/*! Error Active Mode, the CAN controller is allowed to send messages and active error flags.
*/
#define ib_Can_ErrorState_ErrorActive  ((ib_Can_ControllerState) 1)
/*! Error Passive Mode, the CAN controller is still allowed to send messages, but must not send active error flags.
*/
#define ib_Can_ErrorState_ErrorPassive ((ib_Can_ControllerState) 2)
/*! (currently not in use)
*
* *AUTOSAR Doc:* Bus Off Mode, the CAN controller does not take part in communication.
*/
#define ib_Can_ErrorState_BusOff       ((ib_Can_ControllerState) 3)

typedef struct ib_Can_Controller ib_Can_Controller;

/*! Callback type to indicate that a CanTransmitAcknowledge has been received.
* \param context The by the user provided context on registration.
* \param controller The Can controller that received the acknowledge.
* \param acknowledge The acknowledge and its data.
*/
typedef void (*ib_Can_TransmitStatusHandler_t)(void* context, ib_Can_Controller* controller, ib_Can_TransmitAcknowledge* acknowledge);

/*! Callback type to indicate that a CanMessage has been received.
* \param context The by the user provided context on registration.
* \param controller The Can controller that received the message.
* \param metaData The struct containing meta data and referencing the can frame itself.
*/
typedef void (*ib_Can_ReceiveMessageHandler_t)(void* context, ib_Can_Controller* controller, ib_Can_Message* metaData);

/*! Callback type to indicate that the State of the Can Controller has changed.
* \param context The by the user provided context on registration.
* \param controller The Can controller that changed its state.
* \param state The new state of the Can controller.
*/
typedef void (*ib_Can_StateChangedHandler_t)(void* context, ib_Can_Controller* controller, ib_Can_ControllerState state);

/*! Callback type to indicate that the controller Can error state has changed.
* \param context The by the user provided context on registration.
* \param controller The Can controller that received the message.
* \param errorState The new can error state.
*/
typedef void (*ib_Can_ErrorStateChangedHandler_t)(void* context, ib_Can_Controller* controller, ib_Can_ErrorState errorState);

/*! \brief Create a CAN controller at this IB simulation participant.
 * \param outCanController Pointer that refers to the resulting Can controller (out parameter).
 * \param participant The simulation participant at which the Can controller should be created.
 * \param name The name of the new Can controller.
 *
 * The lifetime of the resulting Can controller is directly bound to the lifetime of the simulation participant.
 * There is no futher cleanup necessary except for destroying the simulation participant at the end of the 
 * simulation.
 * The object returned must not be deallocated using free()!
 */
IntegrationBusAPI ib_ReturnCode ib_Can_Controller_Create(ib_Can_Controller** outCanController,
                                                         ib_Participant* participant, const char* name,
                                                         const char* network);

typedef ib_ReturnCode (*ib_Can_Controller_Create_t)(ib_Can_Controller** outCanController,
                                                    ib_Participant* participant, const char* name,
                                                    const char* network);

/*! \brief Start the CAN controller
*
* NB: Only supported in VIBE simulation, the command is ignored
* in simple simulation.
*
* \ref ib_Can_Controller_Reset(), \ref ib_Can_Controller_Stop(), \ref ib_Can_Controller_Sleep()
*/
IntegrationBusAPI ib_ReturnCode ib_Can_Controller_Start(ib_Can_Controller* controller);

typedef ib_ReturnCode (*ib_Can_Controller_Start_t)(ib_Can_Controller* controller);
/*! \brief Stop the CAN controller
*
* NB: Only supported in VIBE simulation, the command is ignored
* in simple simulation.
*
* \ref ib_Can_Controller_Reset(), \ref ib_Can_Controller_Start(), \ref ib_Can_Controller_Sleep()
*/
IntegrationBusAPI ib_ReturnCode ib_Can_Controller_Stop(ib_Can_Controller* controller);

typedef ib_ReturnCode (*ib_Can_Controller_Stop_t)(ib_Can_Controller* controller);

/*! \brief Reset the CAN controller
*
* Resets the controller's Transmit Error Count (TEC) and the
* Receive Error Count (REC). Furthermore, sets the
* CAN controller state to CanControllerState::Uninit and the
* controller's error state to CanErrorState::NotAvailable.
*
* NB: Only supported in VIBE simulation, the command is ignored
* in simple simulation.
*
* \ref ib_Can_Controller_Start(), \ref ib_Can_Controller_Stop(), \ref ib_Can_Controller_Sleep()
*/
IntegrationBusAPI ib_ReturnCode ib_Can_Controller_Reset(ib_Can_Controller* controller);

typedef ib_ReturnCode (*ib_Can_Controller_Reset_t)(ib_Can_Controller* controller);
/*! \brief Put the CAN controller in sleep mode
*
* NB: Only supported in VIBE simulation, the command is ignored
* in simple simulation.
*
* \ref ib_Can_Controller_Reset(), ib_Can_Controller_Start(), \ref ib_Can_Controller_Stop()
*/
IntegrationBusAPI ib_ReturnCode ib_Can_Controller_Sleep(ib_Can_Controller* controller);

typedef ib_ReturnCode(*ib_Can_Controller_Sleep_t)(ib_Can_Controller* controller);

/*! \brief Request the transmission of a CanFrame
*
* NB: In VIBE simulation, the CanFrame must provide a valid CAN
* ID and valid flags. The data length code is optional and is
* automatically derived by the VIBE CAN simulator based on the
* provided flags and the length of the dataField. The controller
* must be in the Started state to transmit and receive messages.
*
* NB: In simple simulation, the requirements for VIBE simulation
* are not enforced. I.e., CanMessages are distributed to
* connected controllers regardless of the content and controller
* states are not checked.
*
* \param controller The Can controller that should send the Can frame.
* \param frame The Can frame to transmit.
* \param userContext A user provided context pointer, that is
* reobtained in the ib_Can_Controller_RegisterTransmitStatusHandler
* handler.
*/
IntegrationBusAPI ib_ReturnCode ib_Can_Controller_SendFrame(ib_Can_Controller* controller, ib_Can_Frame* frame, 
    void* userContext);

typedef ib_ReturnCode (*ib_Can_Controller_SendFrame_t)(ib_Can_Controller* controller, ib_Can_Frame* frame, 
    void* userContext);

/*! \brief Configure the baudrate of the controller
*
* \param controller The Can controller for which the baudrate should be changed.
*
* \param rate Baud rate for regular (non FD) CAN messages given
* in bps; valid range: 0 to 2'000'000
*
* \param fdRate Baud rate for CAN FD messages given in bps; valid
* range: 0 to 16'000'000
*
* NB: In VIBE simulation, the baud rate is used to calculate
* transmission delays of CAN messages and to determine proper
* configuration and interoperation of the connected controllers.
*
* NB: The baud rate has no effect in simple simulation.
*/
IntegrationBusAPI ib_ReturnCode ib_Can_Controller_SetBaudRate(ib_Can_Controller* controller, uint32_t rate, 
    uint32_t fdRate);

typedef ib_ReturnCode(*ib_Can_Controller_SetBaudRate_t)(ib_Can_Controller* controller, uint32_t rate, uint32_t fdRate);
/*! \brief Register a callback for the TX status of sent CAN messages
*
* The registered handler is called when a CAN message was
* successfully transmitted on the bus or when an error occurred.
*
* NB: Full support in VIBE simulation. In simple simulation, all
* messages are automatically positively acknowledged.
*
* \param controller The Can controller for which the callback should be registered.
* \param context The user provided context pointer, that is reobtained in the callback.
* \param handler The handler to be called on transmit acknowledge.
*/
IntegrationBusAPI ib_ReturnCode ib_Can_Controller_RegisterTransmitStatusHandler(
    ib_Can_Controller* controller, 
    void* context, 
    ib_Can_TransmitStatusHandler_t handler,
    ib_Can_TransmitStatus statusMask);

typedef ib_ReturnCode (*ib_Can_Controller_RegisterTransmitStatusHandler_t)(
    ib_Can_Controller* controller, 
    void* context, 
    ib_Can_TransmitStatusHandler_t handler,
    ib_Can_TransmitStatus statusMask);

/*! \brief Register a callback for CAN message reception
*
* The registered handler is called when the controller receives a
* new Can frame.
*
* \param controller The Can controller for which the message callback should be registered.
* \param context The user provided context pointer, that is reobtained in the callback.
* \param handler The handler to be called on reception.
* \param directionMask A bit mask defining the transmit direction of the messages (rx/tx)
*/
IntegrationBusAPI ib_ReturnCode ib_Can_Controller_RegisterReceiveMessageHandler(
    ib_Can_Controller* controller, 
    void* context, 
    ib_Can_ReceiveMessageHandler_t handler,
    ib_Direction directionMask);
    
typedef ib_ReturnCode (*ib_Can_Controller_RegisterReceiveMessageHandler_t)(ib_Can_Controller* controller, void* context, 
    ib_Can_ReceiveMessageHandler_t handler);
/*! \brief Register a callback for controller state changes
*
* The registered handler is called when the CanControllerState of
* the controller changes. E.g., after starting the controller, the
* state changes from ib_Can_ControllerState_Uninit to
* ib_Can_ControllerState_Started.
*
* NB: Only supported in VIBE simulation. In simple simulation,
* the handler is never called.
*
* \param controller The Can controller for which the state change callback should be registered.
* \param context The user provided context pointer, that is reobtained in the callback.
* \param handler The handler to be called on state change.
*/
IntegrationBusAPI ib_ReturnCode ib_Can_Controller_RegisterStateChangedHandler(ib_Can_Controller* controller, 
    void* context, ib_Can_StateChangedHandler_t handler);

typedef ib_ReturnCode (*ib_Can_Controller_RegisterStateChangedHandler_t)(ib_Can_Controller* controller, void* context, 
    ib_Can_StateChangedHandler_t handler);
    
/*! \brief Register a callback for changes of the controller's error state
*
* The registered handler is called when the CanErrorState of the
* controller changes. During normal operation, the controller
* should be in state ib_Can_ErrorState_ErrorActive. The states correspond
* to the error state handling protocol of the CAN specification.
*
* NB: Only supported in VIBE simulation. In simple simulation,
* the handler is never called.
*
* \param controller The Can controller for which the error state callback should be registered.
* \param context The user provided context pointer, that is reobtained in the callback.
* \param handler The handler to be called on error state change.
*/
IntegrationBusAPI ib_ReturnCode ib_Can_Controller_RegisterErrorStateChangedHandler(ib_Can_Controller* controller, 
    void* context, ib_Can_ErrorStateChangedHandler_t handler);

typedef ib_ReturnCode (*ib_Can_Controller_RegisterErrorStateChangedHandler_t)(ib_Can_Controller* controller, 
    void* context, ib_Can_ErrorStateChangedHandler_t handler);

IB_END_DECLS

#pragma pack(pop)
