// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once
#include <stdint.h>
#include "ib/capi/IbMacros.h"
#include "ib/capi/Types.h"
#include "ib/capi/InterfaceIdentifiers.h"

#pragma pack(push)
#pragma pack(8)

IB_BEGIN_DECLS


typedef int32_t ib_Ethernet_TransmitStatus;

/*! The message was successfully transmitted on the CAN bus. */
#define ib_Ethernet_TransmitStatus_Transmitted ((ib_Ethernet_TransmitStatus) 0)

/*! The transmit request was rejected, because the Ethernet controller is not active. */
#define ib_Ethernet_TransmitStatus_ControllerInactive ((ib_Ethernet_TransmitStatus) 1)

/*! The transmit request was rejected, because the Ethernet link is down. */
#define ib_Ethernet_TransmitStatus_LinkDown ((ib_Ethernet_TransmitStatus) 2)

/*! The transmit request was dropped, because the transmit queue is full. */
#define ib_Ethernet_TransmitStatus_Dropped ((ib_Ethernet_TransmitStatus) 3)

/*! The given raw Ethernet frame is ill formated (e.g. frame length is too small or too large, etc.). */
#define ib_Ethernet_TransmitStatus_InvalidFrameFormat ((ib_Ethernet_TransmitStatus) 4)


typedef int32_t ib_Ethernet_State;

//!< The Ethernet controller is switched off (default after reset).
#define ib_Ethernet_State_Inactive ((ib_Ethernet_State) 0)

//!< The Ethernet controller is active, but a link to another Ethernet controller in not yet established.
#define ib_Ethernet_State_LinkDown ((ib_Ethernet_State) 1)

//!< The Ethernet controller is active and the link to another Ethernet controller is established.
#define ib_Ethernet_State_LinkUp ((ib_Ethernet_State) 2)

typedef ib_ByteVector ib_Ethernet_Frame;

struct ib_Ethernet_Message
{
    ib_InterfaceIdentifier interfaceId; //!< The interface id that specifies which version of this struct was obtained
    ib_NanosecondsTime timestamp;
    ib_Ethernet_Frame* ethernetFrame;
};

typedef struct ib_Ethernet_Message ib_Ethernet_Message;

struct ib_Ethernet_TransmitAcknowledge
{
    ib_InterfaceIdentifier interfaceId; //!< The interface id that specifies which version of this struct was obtained
    void* userContext; //!< Value that was provided by user in corresponding parameter on send of Ethernet frame
    ib_NanosecondsTime timestamp; //!< Reception time
    ib_Ethernet_TransmitStatus status; //!< Status of the EthernetTransmitRequest
};

typedef struct ib_Ethernet_TransmitAcknowledge ib_Ethernet_TransmitAcknowledge;

typedef struct ib_Ethernet_Controller ib_Ethernet_Controller;

/*! Callback type to indicate that a EthernetMessage has been received.
* \param context The context provided by the user upon registration.
* \param controller The Ethernet controller that received the message.
* \param metaData The struct containing meta data and referencing the Ethernet frame itself.
*/
typedef void (*ib_Ethernet_ReceiveMessageHandler_t)(void* context, ib_Ethernet_Controller* controller, 
  ib_Ethernet_Message* message);
    
/*! Callback type to indicate that a EthernetFrame has been sent.
* \param context The by the user provided context on registration.
* \param controller The Ethernet controller that received the acknowledge.
* \param acknowledge The acknowledge and its data.
*/
typedef void (*ib_Ethernet_FrameAckHandler_t)(void* context, ib_Ethernet_Controller* controller, 
  ib_Ethernet_TransmitAcknowledge* acknowledge);
    
/*! Callback type to indicate that the Ethernet controller state has changed.
* \param context The by the user provided context on registration.
* \param controller The Ethernet controller whose state did change.
* \param state The new state.
*/
typedef void (*ib_Ethernet_StateChangedHandler_t)(void* context, ib_Ethernet_Controller* controller,
  ib_Ethernet_State state);

/*! Callback type to indicate that the link bit rate has changed.
* \param context The by the user provided context on registration.
* \param controller The Ethernet controller that is affected.
* \param bitrate The new bitrate in kbits per second of the Ethernet link.
*/
typedef void (*ib_Ethernet_BitRateChangedHandler_t)(void* context, ib_Ethernet_Controller* controller,
  uint32_t bitrate);

/*! \brief Create an Ethernet controller at this IB simulation participant.
* 
* \param outController A pointer to a pointer in which the ethernet controller will be stored (out parameter).
* \param participant The simulation participant for whicht the ethernet controller should be created.
* \param name The utf8 encoded name of the new Ethernet controller.
* \result A return code identifying the success/failure of the call.
* ! \note The object returned must not be deallocated using free()!
* 
* \see ib::mw::IParticipant::CreateEthController
*/
IntegrationBusAPI ib_ReturnCode ib_Ethernet_Controller_Create(
  ib_Ethernet_Controller** outController,
  ib_Participant* participant, const char* name, const char* network);

typedef ib_ReturnCode(*ib_Ethernet_Controller_Create_t)(
  ib_Ethernet_Controller** outController,
  ib_Participant* participant, 
  const char* name,
  const char* network);

/*! \brief Activates the Ethernet controller
*
* Upon activation of the controller, the controller attempts to
* establish a link. Messages can only be sent once the link has
* been successfully established,
*
* NB: Only supported in VIBE simulation! In simple simulation,
* messages can be sent without need to call ib_Ethernet_Controller_Activate()
* 
* \param controller The ethernet controller to be activated.
* \result A return code identifying the success/failure of the call.
* 
* \see ib::sim::eth::IEthController::Activate
*/
IntegrationBusAPI ib_ReturnCode ib_Ethernet_Controller_Activate(ib_Ethernet_Controller* controller);

typedef ib_ReturnCode(*ib_Ethernet_Controller_Activate_t)(ib_Ethernet_Controller* controller);

/*! \brief Deactivate the Ethernet controller
*
* Deactivate the controller and shut down the link. The
* controller will no longer receive messages, and it cannot send
* messages anymore.
*
* NB: Only supported in VIBE simulation! In simple simulation,
* ib_Ethernet_Controller_Deactivate() has no effects and messages can still be sent.
* 
* \param controller The ethernet controller to be deactivated.
* \result A return code identifying the success/failure of the call.
* 
* \see ib::sim::eth::IEthController::Deactivate
*/
IntegrationBusAPI ib_ReturnCode ib_Ethernet_Controller_Deactivate(ib_Ethernet_Controller* controller);

typedef ib_ReturnCode(*ib_Ethernet_Controller_Deactivate_t)(ib_Ethernet_Controller* controller);
/*! \brief Register a callback for Ethernet message reception
*
* The handler is called when the controller receives a new
* Ethernet message.
* 
* \param controller The Ethernet controller for which the message callback should be registered.
* \param context The user provided context pointer, that is reobtained in the callback.
* \param handler The handler to be called on reception.
* \result A return code identifying the success/failure of the call.
* 
* \see ib::sim::eth::IEthController::RegisterReceiveMessageHandler
*/
IntegrationBusAPI ib_ReturnCode ib_Ethernet_Controller_RegisterReceiveMessageHandler(
  ib_Ethernet_Controller* controller, 
  void* context, 
  ib_Ethernet_ReceiveMessageHandler_t handler);

typedef ib_ReturnCode(*ib_Ethernet_Controller_RegisterReceiveMessageHandler_t)(
  ib_Ethernet_Controller* controller, 
  void* context,
  ib_Ethernet_ReceiveMessageHandler_t handler);

/*! \brief Register a callback for Ethernet transmit acknowledgments
*
* The handler is called when a previously sent message was
* successfully transmitted or when the transmission has
* failed. The original message is identified by the userContext.
*
* NB: Full support in VIBE Ethernet simulation. In simple
* simulation, all messages are immediately positively
* acknowledged by a receiving controller.
* 
* \param controller The Ethernet controller for which the message acknowledge callback should be registered.
* \param context The user provided context pointer, that is reobtained in the callback.
* \param handler The handler to be called on reception.
* \result A return code identifying the success/failure of the call.
* 
* \see ib::sim::eth::IEthController::RegisterMessageAckHandler
*/
IntegrationBusAPI ib_ReturnCode ib_Ethernet_Controller_RegisterFrameAckHandler(
  ib_Ethernet_Controller* controller,
  void* context,
  ib_Ethernet_FrameAckHandler_t handler);

typedef ib_ReturnCode(*ib_Ethernet_Controller_RegisterFrameAckHandler_t)(
  ib_Ethernet_Controller* controller,
  void* context,
  ib_Ethernet_FrameAckHandler_t handler);

/*! \brief Register a callback for changes of the controller state
*
* The handler is called when the state of the controller
* changes. E.g., a call to ib_Ethernet_Controller_Activate() causes the controller to
* change from state ib_Ethernet_State_Inactive to ib_Ethernet_State_LinkDown. Later, when the link
* has been established, the state changes again from ib_Ethernet_State_LinkDown to
* ib_Ethernet_State_LinkUp. Similarly, the status changes back to ib_Ethernet_State_Inactive upon a
* call to ib_Ethernet_Controller_Deactivate().
*
* NB: Only supported in VIBE Ethernet simulation.
*
* \param controller The Ethernet controller for which the message acknowledge callback should be registered.
* \param context The user provided context pointer, that is reobtained in the callback.
* \param handler The handler to be called on reception.
* \result A return code identifying the success/failure of the call.
* 
* \see ib::sim::eth::IEthController::RegisterStateChangedHandler
*/
IntegrationBusAPI ib_ReturnCode ib_Ethernet_Controller_RegisterStateChangedHandler(
  ib_Ethernet_Controller* controller, 
  void* context,
  ib_Ethernet_StateChangedHandler_t handler);

typedef ib_ReturnCode(*ib_Ethernet_Controller_RegisterStateChangedHandler_t)(
  ib_Ethernet_Controller* controller,
  void* context,
  ib_Ethernet_StateChangedHandler_t handler);

/*! \brief Register a callback for changes of the link bit rate
*
* The handler is called when the bit rate of the connected link
* changes. This is typically the case when a link was
* successfully established, or the controller was deactivated.
*
* NB: Only supported in VIBE Ethernet simulation.
* 
* \param controller The Ethernet controller for which the bitrate change callback should be registered.
* \param context The user provided context pointer, that is reobtained in the callback.
* \param handler The handler to be called on change.
* \result A return code identifying the success/failure of the call.
* 
* \see ib::sim::eth::IEthController::RegisterBitRateChangedHandler
*/
IntegrationBusAPI ib_ReturnCode ib_Ethernet_Controller_RegisterBitRateChangedHandler(
  ib_Ethernet_Controller* controller, 
  void* context,
  ib_Ethernet_BitRateChangedHandler_t handler);

typedef ib_ReturnCode(*ib_Ethernet_Controller_RegisterBitRateChangedHandler_t)(
  ib_Ethernet_Controller* controller, 
  void* context,
  ib_Ethernet_BitRateChangedHandler_t handler);

/*! \brief Send an Ethernet frame
*
* NB: In VIBE simulation, requires previous activation of the
* controller and a successfully established link. Also, the
* entire EthFrame must be valid, e.g., destination and source MAC
* addresses must be valid, ether type and vlan tags must be
* correct, payload size must be valid.
*
* These requirements for VIBE simulation are not enforced in
* simple simulation. In this case, the message is simply passed
* on to all connected controllers without performing any check.
* Nonetheless, the minimum frame size of 60 bytes must be provided, or 
* ib_ReturnCode_BAD_PARAMETER will be returned.
*
* \param controller The Ethernet controller that should send the frame.
* \param frame The Ethernet frame to be sent.
* \param userContext The user provided context pointer, that is reobtained in the frame ack handler
* \result A return code identifying the success/failure of the call.
* 
* \see ib::sim::eth::IEthController::SendFrame
*/
IntegrationBusAPI ib_ReturnCode ib_Ethernet_Controller_SendFrame(
  ib_Ethernet_Controller* controller,
  ib_Ethernet_Frame* frame, 
  void* userContext);

typedef ib_ReturnCode(*ib_Ethernet_Controller_SendFrame_t)(
  ib_Ethernet_Controller* controller,
  ib_Ethernet_Frame* frame,
  void* userContext);

IB_END_DECLS

#pragma pack(pop)
