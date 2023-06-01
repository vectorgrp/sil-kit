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

typedef uint32_t SilKit_EthernetTransmitStatus;

//! The message was successfully transmitted on the CAN bus.
#define SilKit_EthernetTransmitStatus_Transmitted ((SilKit_EthernetTransmitStatus) BIT(0))

//! The transmit request was rejected, because the Ethernet controller is not active.
#define SilKit_EthernetTransmitStatus_ControllerInactive ((SilKit_EthernetTransmitStatus) BIT(1))

//! The transmit request was rejected, because the Ethernet link is down.
#define SilKit_EthernetTransmitStatus_LinkDown ((SilKit_EthernetTransmitStatus) BIT(2))

//! The transmit request was dropped, because the transmit queue is full.
#define SilKit_EthernetTransmitStatus_Dropped ((SilKit_EthernetTransmitStatus) BIT(3))

/* (SilKit_EthernetTransmitStatus) BIT(4) is RESERVED (used to be SilKit_EthernetTransmitStatus_DuplicatedTransmitId) */

//! The given raw Ethernet frame is ill formated (e.g. frame length is too small or too large, etc.).
#define SilKit_EthernetTransmitStatus_InvalidFrameFormat ((SilKit_EthernetTransmitStatus) BIT(5))

//! Combines all available transmit statuses.
#define SilKit_EthernetTransmitStatus_DefaultMask \
    (SilKit_EthernetTransmitStatus_Transmitted | SilKit_EthernetTransmitStatus_ControllerInactive \
     | SilKit_EthernetTransmitStatus_LinkDown | SilKit_EthernetTransmitStatus_Dropped \
     | SilKit_EthernetTransmitStatus_InvalidFrameFormat)


typedef uint32_t SilKit_EthernetState;

//! The Ethernet controller is switched off (default after reset).
#define SilKit_EthernetState_Inactive ((SilKit_EthernetState) 0)

//! The Ethernet controller is active, but a link to another Ethernet controller in not yet established.
#define SilKit_EthernetState_LinkDown ((SilKit_EthernetState) 1)

//! The Ethernet controller is active and the link to another Ethernet controller is established.
#define SilKit_EthernetState_LinkUp ((SilKit_EthernetState) 2)


typedef struct
{
    SilKit_StructHeader structHeader; //!< The interface id that specifies which version of this struct was obtained
    SilKit_NanosecondsTime timestamp; //!< Timestamp of the state change event
    SilKit_EthernetState state; //!< New state of the Ethernet controller
} SilKit_EthernetStateChangeEvent;

typedef uint32_t SilKit_EthernetBitrate; //!< Bitrate in kBit/sec

typedef struct
{
    SilKit_StructHeader structHeader; //!< The interface id that specifies which version of this struct was obtained
    SilKit_NanosecondsTime timestamp; //!< Timestamp of the bitrate change event
    SilKit_EthernetBitrate bitrate; //!< New bitrate in kBit/sec
} SilKit_EthernetBitrateChangeEvent;

//! A raw Ethernet frame
typedef struct
{
    SilKit_StructHeader structHeader; //!< The interface id that specifies which version of this struct was obtained
    SilKit_ByteVector raw;
} SilKit_EthernetFrame;


typedef struct 
{
    SilKit_StructHeader structHeader; //!< The interface id that specifies which version of this struct was obtained
    SilKit_NanosecondsTime timestamp; //!< Send time
    SilKit_EthernetFrame* ethernetFrame; //!< The raw Ethernet frame
    SilKit_Direction direction; //!< Receive/Transmit direction
    void* userContext; //!< Optional pointer provided by user when sending the frame
} SilKit_EthernetFrameEvent;

struct SilKit_EthernetFrameTransmitEvent
{
    SilKit_StructHeader structHeader; //!< The interface id that specifies which version of this struct was obtained
    void* userContext; //!< Value that was provided by user in corresponding parameter on send of Ethernet frame
    SilKit_NanosecondsTime timestamp; //!< Reception time
    SilKit_EthernetTransmitStatus status; //!< Status of the EthernetTransmitRequest
};
typedef struct SilKit_EthernetFrameTransmitEvent SilKit_EthernetFrameTransmitEvent;

typedef struct SilKit_EthernetController SilKit_EthernetController;

/*! Callback type to indicate that a EthernetMessage has been received.
* \param context The context provided by the user upon registration.
* \param controller The Ethernet controller that received the message.
* \param frameEvent Contains the raw frame and the timestamp of the event.
*/
typedef void (SilKitFPTR *SilKit_EthernetFrameHandler_t)(void* context, SilKit_EthernetController* controller,
  SilKit_EthernetFrameEvent* frameEvent);
    
/*! Callback type to indicate that a EthernetFrame has been sent.
* \param context The by the user provided context on registration.
* \param controller The Ethernet controller that received the acknowledge.
* \param frameTransmitEvent Contains the transmit status and the timestamp of the event.
*/
typedef void (SilKitFPTR *SilKit_EthernetFrameTransmitHandler_t)(void* context, SilKit_EthernetController* controller,
  SilKit_EthernetFrameTransmitEvent* frameTransmitEvent);
    
/*! Callback type to indicate that the Ethernet controller state has changed.
* \param context The by the user provided context on registration.
* \param controller The Ethernet controller whose state did change.
* \param stateChangeEvent Contains the new state and the timestamp of the event.
*/
typedef void (SilKitFPTR *SilKit_EthernetStateChangeHandler_t)(void* context, SilKit_EthernetController* controller,
  SilKit_EthernetStateChangeEvent* stateChangeEvent);

/*! Callback type to indicate that the link bit rate has changed.
* \param context Context pointer provided by the user on registration.
* \param controller The Ethernet controller that is affected.
* \param bitrateChangeEvent Contains the new bitrate and the timestamp of the event.
*/
typedef void (SilKitFPTR *SilKit_EthernetBitrateChangeHandler_t)(void* context, SilKit_EthernetController* controller,
  SilKit_EthernetBitrateChangeEvent* bitrateChangeEvent);

/*! \brief Create an Ethernet controller at this SIL Kit simulation participant.
* 
* \param outController A pointer to a pointer in which the Ethernet controller will be stored (out parameter).
* \param participant The simulation participant for which the Ethernet controller should be created.
* \param name The utf8 encoded name of the new Ethernet controller.
* \param network The network of the Ethernet controller to operate in.
* \result A return code identifying the success/failure of the call.
* ! \note The object returned must not be deallocated using free()!
* 
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_EthernetController_Create(
  SilKit_EthernetController** outController,
  SilKit_Participant* participant, const char* name, const char* network);

typedef SilKit_ReturnCode(SilKitFPTR *SilKit_EthernetController_Create_t)(
  SilKit_EthernetController** outController,
  SilKit_Participant* participant, 
  const char* name,
  const char* network);

/*! \brief Activates the Ethernet controller
*
* Upon activation of the controller, the controller attempts to
* establish a link. Messages can only be sent once the link has
* been successfully established,
* 
* \param controller The Ethernet controller to be activated.
* \result A return code identifying the success/failure of the call.
* 
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_EthernetController_Activate(SilKit_EthernetController* controller);

typedef SilKit_ReturnCode(SilKitFPTR *SilKit_EthernetController_Activate_t)(SilKit_EthernetController* controller);

/*! \brief Deactivate the Ethernet controller
*
* Deactivate the controller and shut down the link. The
* controller will no longer receive messages, and it cannot send
* messages anymore.
* 
* \param controller The Ethernet controller to be deactivated.
* \result A return code identifying the success/failure of the call.
* 
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_EthernetController_Deactivate(SilKit_EthernetController* controller);

typedef SilKit_ReturnCode(SilKitFPTR *SilKit_EthernetController_Deactivate_t)(SilKit_EthernetController* controller);

/*! \brief Register a callback for Ethernet message reception
*
* The handler is called when the controller receives a new
* Ethernet message.
* 
* \param controller The Ethernet controller for which the message callback should be registered.
* \param context The user provided context pointer, that is reobtained in the callback.
* \param handler The handler to be called on reception.
* \param directionMask A bit mask defining the transmit direction of the messages (rx/tx)
* \param outHandlerId The handler identifier that can be used to remove the callback.
* \result A return code identifying the success/failure of the call.
* 
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_EthernetController_AddFrameHandler(SilKit_EthernetController* controller,
                                                                      void* context,
                                                                      SilKit_EthernetFrameHandler_t handler,
                                                                      SilKit_Direction directionMask,
                                                                      SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_EthernetController_AddFrameHandler_t)(SilKit_EthernetController* controller,
                                                                         void* context,
                                                                         SilKit_EthernetFrameHandler_t handler,
                                                                         SilKit_Direction directionMask,
                                                                         SilKit_HandlerId* outHandlerId);

/*! \brief  Remove a \ref SilKit_EthernetFrameHandler_t by SilKit_HandlerId on this controller 
*
* \param controller The Ethernet controller for which the callback should be removed.
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_EthernetController_RemoveFrameHandler(SilKit_EthernetController* controller,
                                                                          SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_EthernetController_RemoveFrameHandler_t)(SilKit_EthernetController* controller,
                                                                     SilKit_HandlerId handlerId);

/*! \brief Register a callback for Ethernet transmit acknowledgments
*
* The handler is called when a previously sent message was
* successfully transmitted or when the transmission has
* failed. The original message is identified by the userContext.
*
* NB: Full support in a detailed simulation. In a simple
* simulation, all messages are immediately positively
* acknowledged by a receiving controller.
* 
* \param controller The Ethernet controller for which the message acknowledge callback should be registered.
* \param context The user provided context pointer, that is reobtained in the callback.
* \param handler The handler to be called on reception.
* \param transmitStatusMask A bit mask defining the transmit status of the message
* \param outHandlerId The handler identifier that can be used to remove the callback.
* \result A return code identifying the success/failure of the call.
* 
*/
SilKitAPI SilKit_ReturnCode
    SilKitCALL SilKit_EthernetController_AddFrameTransmitHandler(SilKit_EthernetController* controller,
                                                  void* context,
                                                  SilKit_EthernetFrameTransmitHandler_t handler,
                                                  SilKit_EthernetTransmitStatus transmitStatusMask,
                                                  SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_EthernetController_AddFrameTransmitHandler_t)(
    SilKit_EthernetController* controller, void* context, SilKit_EthernetFrameTransmitHandler_t handler,
    SilKit_EthernetTransmitStatus transmitStatusMask, SilKit_HandlerId* outHandlerId);

/*! \brief  Remove a \ref SilKit_EthernetFrameTransmitHandler_t by SilKit_HandlerId on this controller
*
* \param controller The Ethernet controller for which the callback should be removed.
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_EthernetController_RemoveFrameTransmitHandler(SilKit_EthernetController* controller,
                                                                                  SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_EthernetController_RemoveFrameTransmitHandler_t)(SilKit_EthernetController* controller,
                                                                             SilKit_HandlerId handlerId);

/*! \brief Register a callback for changes of the controller state
*
* The handler is called when the state of the controller
* changes. E.g., a call to SilKit_EthernetController_Activate() causes the controller to
* change from state SilKit_EthernetState_Inactive to SilKit_EthernetState_LinkDown. Later, when the link
* has been established, the state changes again from SilKit_EthernetState_LinkDown to
* SilKit_EthernetState_LinkUp. Similarly, the status changes back to SilKit_EthernetState_Inactive upon a
* call to SilKit_EthernetController_Deactivate().
*
* \param controller The Ethernet controller for which the message acknowledge callback should be registered.
* \param context The user provided context pointer, that is reobtained in the callback.
* \param handler The handler to be called on reception.
* \param outHandlerId The handler identifier that can be used to remove the callback.
* \result A return code identifying the success/failure of the call.
* 
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_EthernetController_AddStateChangeHandler(SilKit_EthernetController* controller,
                                                                             void* context,
                                                                             SilKit_EthernetStateChangeHandler_t handler,
                                                                             SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_EthernetController_AddStateChangeHandler_t)(SilKit_EthernetController* controller,
                                                                        void* context,
                                                                        SilKit_EthernetStateChangeHandler_t handler,
                                                                        SilKit_HandlerId* outHandlerId);

/*! \brief  Remove a \ref SilKit_EthernetStateChangeHandler_t by SilKit_HandlerId on this controller 
*
* \param controller The Ethernet controller for which the callback should be removed.
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_EthernetController_RemoveStateChangeHandler(SilKit_EthernetController* controller,
                                                                                SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_EthernetController_RemoveStateChangeHandler_t)(SilKit_EthernetController* controller,
                                                                           SilKit_HandlerId handlerId);


/*! \brief Register a callback for changes of the link bit rate
*
* The handler is called when the bit rate of the connected link
* changes. This is typically the case when a link was
* successfully established, or the controller was deactivated.
*
* \param controller The Ethernet controller for which the bitrate change callback should be registered.
* \param context The user provided context pointer, that is reobtained in the callback.
* \param handler The handler to be called on change.
* \param outHandlerId The handler identifier that can be used to remove the callback.
* \result A return code identifying the success/failure of the call.
* 
*/
SilKitAPI SilKit_ReturnCode
SilKitCALL SilKit_EthernetController_AddBitrateChangeHandler(SilKit_EthernetController* controller, void* context,
                                               SilKit_EthernetBitrateChangeHandler_t handler, SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_EthernetController_AddBitrateChangeHandler_t)(SilKit_EthernetController* controller,
                                                                          void* context,
                                                                          SilKit_EthernetBitrateChangeHandler_t handler,
                                                                          SilKit_HandlerId* outHandlerId);
/*! \brief  Remove a \ref SilKit_EthernetBitrateChangeHandler_t by SilKit_HandlerId on this controller 
*
* \param controller The Ethernet controller for which the callback should be removed.
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_EthernetController_RemoveBitrateChangeHandler(SilKit_EthernetController* controller,
                                                                                  SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_EthernetController_RemoveBitrateChangeHandler_t)(SilKit_EthernetController* controller,
                                                                             SilKit_HandlerId handlerId);

/*! \brief Send an Ethernet frame
 *
 * Requires previous activation of the controller and a successfully
 * established link. Also, the entire EthernetFrame must be valid, e.g.,
 * destination and source MAC addresses must be valid, ether type and vlan tags
 * must be correct.
 *
 * These requirements are not enforced in simple simulation. In this case, the
 * message is simply passed on to all connected controllers without performing
 * any check. The user must ensure that a valid frame is provided.
 *
 * If the frame size is smaller than the minimum of 60 bytes, the frame will be
 * padded with zeros.
 *
 * \param controller The Ethernet controller that should send the frame.
 * \param frame The Ethernet frame to be sent.
 * \param userContext The user provided context pointer, that is reobtained in
 *                    the frame ack handler
 * \result A return code identifying the success/failure of the call.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_EthernetController_SendFrame(
  SilKit_EthernetController* controller,
  SilKit_EthernetFrame* frame,
  void* userContext);

typedef SilKit_ReturnCode(SilKitFPTR *SilKit_EthernetController_SendFrame_t)(
  SilKit_EthernetController* controller,
  SilKit_EthernetFrame* frame,
  void* userContext);

SILKIT_END_DECLS

#pragma pack(pop)
