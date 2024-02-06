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

/*! The available flags within the flags member of a CAN frame.
*/
typedef uint32_t SilKit_CanFrameFlag;
#define SilKit_CanFrameFlag_ide ((SilKit_CanFrameFlag)BIT(9)) //!< Identifier Extension
#define SilKit_CanFrameFlag_rtr ((SilKit_CanFrameFlag)BIT(4)) //!< Remote Transmission Request
#define SilKit_CanFrameFlag_fdf ((SilKit_CanFrameFlag)BIT(12)) //!< FD Format Indicator
#define SilKit_CanFrameFlag_brs ((SilKit_CanFrameFlag)BIT(13)) //!< Bit Rate Switch  (for FD Format only)
#define SilKit_CanFrameFlag_esi ((SilKit_CanFrameFlag)BIT(14)) //!< Error State indicator (for FD Format only)
#define SilKit_CanFrameFlag_xlf ((SilKit_CanFrameFlag)BIT(15)) //!< XL Format Indicator
#define SilKit_CanFrameFlag_sec ((SilKit_CanFrameFlag)BIT(16)) //!< Simple Extended Content (for XL Format only)

/*! \brief A CAN frame
    */
struct SilKit_CanFrame
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    uint32_t id; //!< CAN Identifier
    SilKit_CanFrameFlag flags; //!< CAN Arbitration and Control Field Flags; see SilKit_CanFrameFlag
    uint16_t dlc; //!< Data Length Code - determined by a network simulator if available
    uint8_t sdt; //!< SDU type - describes the structure of the frames Data Field content (for XL Format only)
    uint8_t vcid; //!< Virtual CAN network ID (for XL Format only)
    uint32_t af; //!< Acceptance field (for XL Format only)

    SilKit_ByteVector data; //!< Data field containing the payload
};
typedef struct SilKit_CanFrame SilKit_CanFrame;

struct SilKit_CanFrameEvent
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    SilKit_NanosecondsTime timestamp; //!< Reception time
    SilKit_CanFrame* frame; //!< The CAN Frame that corresponds to the meta data
    SilKit_Direction direction; //!< The transmit direction of the CAN frame (TX/RX)
    void* userContext; //!< Optional pointer provided by user when sending the frame
};
typedef struct SilKit_CanFrameEvent SilKit_CanFrameEvent;


typedef int32_t SilKit_CanTransmitStatus;

/*! The message was successfully transmitted on the CAN bus.
*/
#define SilKit_CanTransmitStatus_Transmitted          ((SilKit_CanTransmitStatus) BIT(0))

/*! (currently not in use)
*
* The transmit queue was reset.
*/
#define SilKit_CanTransmitStatus_Canceled             ((SilKit_CanTransmitStatus) BIT(1))

/*! The transmit request was rejected, because the transmit queue is full.
*/
#define SilKit_CanTransmitStatus_TransmitQueueFull    ((SilKit_CanTransmitStatus) BIT(2))

/* (SilKit_CanTransmitStatus) BIT(3) is RESERVED (used to be SilKit_CanTransmitStatus_DuplicatedTransmitId) */

/*! Combines all available transmit statuses.
 */
#define SilKit_CanTransmitStatus_DefaultMask \
    (SilKit_CanTransmitStatus_Transmitted | SilKit_CanTransmitStatus_Canceled \
     | SilKit_CanTransmitStatus_TransmitQueueFull)

/*! \brief The acknowledgment of a CAN message, sent to the controller
*/
struct SilKit_CanFrameTransmitEvent
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    void* userContext; //!< Value that was provided by user in corresponding parameter on send of CAN frame
    SilKit_NanosecondsTime timestamp; //!< Reception time
    SilKit_CanTransmitStatus status; //!< Status of the CanTransmitRequest
    /*! Identifies the CAN id to which this CanFrameTransmitEvent refers to.
     *
     * \version Check: SK_ID_GET_VERSION(SilKit_Struct_GetId(event)) >= 2
     *
     * You must check that the structure version is sufficient before accessing this field.
     * Added in SIL Kit version 4.0.11.
     */
    uint32_t canId;
};
typedef struct SilKit_CanFrameTransmitEvent SilKit_CanFrameTransmitEvent;

/*! \brief CAN Controller state according to AUTOSAR specification AUTOSAR_SWS_CANDriver 4.3.1
*/
typedef int32_t SilKit_CanControllerState;
/*! CAN controller is not initialized (initial state after reset).
*/
#define SilKit_CanControllerState_Uninit  ((SilKit_CanControllerState) 0)
/*! CAN controller is initialized but does not participate on the CAN bus.
*/
#define SilKit_CanControllerState_Stopped ((SilKit_CanControllerState) 1)
/*! CAN controller is in normal operation mode.
*/
#define SilKit_CanControllerState_Started ((SilKit_CanControllerState) 2)
/*! CAN controller is in sleep mode which is similar to the Stopped state.
*/
#define SilKit_CanControllerState_Sleep   ((SilKit_CanControllerState) 3)

/*! \brief An incoming state change of a CAN controller
*/
struct SilKit_CanStateChangeEvent
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    SilKit_NanosecondsTime timestamp; //!< Reception time
    SilKit_CanControllerState state; //!< CAN controller state
};
typedef struct SilKit_CanStateChangeEvent SilKit_CanStateChangeEvent;

/*! \brief Error state of a CAN node according to CAN specification.
*/
typedef int32_t SilKit_CanErrorState;
/*! Error State is Not Available, because CAN controller is in state Uninit.
*
* *AUTOSAR Doc:* Successful transmission.
*/
#define SilKit_CanErrorState_NotAvailable ((SilKit_CanControllerState) 0)
/*! Error Active Mode, the CAN controller is allowed to send messages and active error flags.
*/
#define SilKit_CanErrorState_ErrorActive  ((SilKit_CanControllerState) 1)
/*! Error Passive Mode, the CAN controller is still allowed to send messages, but must not send active error flags.
*/
#define SilKit_CanErrorState_ErrorPassive ((SilKit_CanControllerState) 2)
/*! (currently not in use)
*
* *AUTOSAR Doc:* Bus Off Mode, the CAN controller does not take part in communication.
*/
#define SilKit_CanErrorState_BusOff       ((SilKit_CanControllerState) 3)

/*! \brief An incoming state change of a CAN controller
*/
struct SilKit_CanErrorStateChangeEvent
{
    SilKit_StructHeader structHeader; //!< The interface id specifying which version of this struct was obtained
    SilKit_NanosecondsTime timestamp; //!< Reception time
    SilKit_CanErrorState errorState; //!< CAN controller error state
};
typedef struct SilKit_CanErrorStateChangeEvent SilKit_CanErrorStateChangeEvent;

typedef struct SilKit_CanController SilKit_CanController;

/*! Callback type to indicate that a CanTransmitAcknowledge has been received.
* \param context The by the user provided context on registration.
* \param controller The CAN controller that received the acknowledge.
* \param frameTransmitEvent The incoming CAN frame transmit event.
*/
typedef void (SilKitFPTR *SilKit_CanFrameTransmitHandler_t)(void* context, SilKit_CanController* controller,
                                              SilKit_CanFrameTransmitEvent* frameTransmitEvent);

/*! Callback type to indicate that a CanMessage has been received.
* \param context The by the user provided context on registration.
* \param controller The CAN controller that received the message.
* \param frameEvent The incoming CAN frame event containing timestamp, transmit ID and referencing the CAN frame itself.
*/
typedef void (SilKitFPTR *SilKit_CanFrameHandler_t)(void* context, SilKit_CanController* controller,
                                               SilKit_CanFrameEvent* frameEvent);

/*! Callback type to indicate that the State of the CAN Controller has changed.
* \param context The by the user provided context on registration.
* \param controller The CAN controller that changed its state.
* \param stateChangeEvent The state change event containing timestamp and new state.
*/
typedef void (SilKitFPTR *SilKit_CanStateChangeHandler_t)(void* context, SilKit_CanController* controller,
                                             SilKit_CanStateChangeEvent* stateChangeEvent);

/*! Callback type to indicate that the controller CAN error state has changed.
* \param context The by the user provided context on registration.
* \param controller The CAN controller that received the message.
* \param errorStateChangeEvent The error state change event containing timestamp and new error state.
*/
typedef void (SilKitFPTR *SilKit_CanErrorStateChangeHandler_t)(void* context, SilKit_CanController* controller,
                                                  SilKit_CanErrorStateChangeEvent* errorStateChangeEvent);

/*! \brief Create a CAN controller at this SIL Kit simulation participant.
 * \param outCanController Pointer that refers to the resulting CAN controller (out parameter).
 * \param participant The simulation participant at which the CAN controller should be created.
 * \param name The name of the new CAN controller (UTF-8).
 * \param network The network of the CAN controller to operate in (UTF-8).
 *
 * The lifetime of the resulting CAN controller is directly bound to the lifetime of the simulation participant.
 * There is no further cleanup necessary except for destroying the simulation participant at the end of the 
 * simulation.
 * The object returned must not be deallocated using free()!
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_CanController_Create(SilKit_CanController** outCanController,
                                                         SilKit_Participant* participant, const char* name,
                                                         const char* network);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_CanController_Create_t)(SilKit_CanController** outCanController,
                                                    SilKit_Participant* participant, const char* name,
                                                    const char* network);

/*! \brief Start the CAN controller
*
* \ref SilKit_CanController_Reset(), \ref SilKit_CanController_Stop(), \ref SilKit_CanController_Sleep()
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_CanController_Start(SilKit_CanController* controller);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_CanController_Start_t)(SilKit_CanController* controller);

/*! \brief Stop the CAN controller
*
* \ref SilKit_CanController_Reset(), \ref SilKit_CanController_Start(), \ref SilKit_CanController_Sleep()
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_CanController_Stop(SilKit_CanController* controller);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_CanController_Stop_t)(SilKit_CanController* controller);

/*! \brief Reset the CAN controller
*
* Resets the controller's Transmit Error Count (TEC) and the
* Receive Error Count (REC). Furthermore, sets the
* CAN controller state to CanControllerState::Uninit and the
* controller's error state to CanErrorState::NotAvailable.
*
* \ref SilKit_CanController_Start(), \ref SilKit_CanController_Stop(), \ref SilKit_CanController_Sleep()
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_CanController_Reset(SilKit_CanController* controller);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_CanController_Reset_t)(SilKit_CanController* controller);

/*! \brief Put the CAN controller in sleep mode
*
* \ref SilKit_CanController_Reset(), SilKit_CanController_Start(), \ref SilKit_CanController_Stop()
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_CanController_Sleep(SilKit_CanController* controller);

typedef SilKit_ReturnCode(SilKitFPTR *SilKit_CanController_Sleep_t)(SilKit_CanController* controller);

/*! \brief Request the transmission of a CanFrame
*
* The CanFrame must provide a valid CAN
* ID and valid flags. The controller
* must be in the Started state to transmit and receive messages.
*
* \param controller The CAN controller that should send the CAN frame.
* \param frame The CAN frame to transmit.
* \param userContext A user provided context pointer, that is
* reobtained in the SilKit_CanController_AddFrameTransmitHandler
* handler.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_CanController_SendFrame(SilKit_CanController* controller, SilKit_CanFrame* frame,
    void* userContext);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_CanController_SendFrame_t)(SilKit_CanController* controller, SilKit_CanFrame* frame,
    void* userContext);

/*! \brief Configure the baud rate of the controller
 *
 * \param controller The CAN controller for which the baud rate should be changed.
 *
 * \param rate Baud rate for regular (non FD) CAN messages given
 * in bps; valid range: 0 to 2'000'000
 *
 * \param fdRate Baud rate for CAN FD messages given in bps; valid
 * range: 0 to 16'000'000
 *
 * \param xlRate Baud rate for CAN XL messages given in bps; valid
 * range: 0 to 16'000'000
 *
 * In a detailed simulation, the baud rate is used to calculate
 * transmission delays of CAN messages and to determine proper
 * configuration and interoperation of the connected controllers.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_CanController_SetBaudRate(SilKit_CanController* controller, uint32_t rate,
    uint32_t fdRate, uint32_t xlRate);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_CanController_SetBaudRate_t)(SilKit_CanController* controller, uint32_t rate,
    uint32_t fdRate, uint32_t xlRate);

/*! \brief Register a callback for the TX status of sent CAN messages
*
* The registered handler is called when a CAN message was
* successfully transmitted on the bus or when an error occurred.
*
* NB: Full support in a detailed simulation. In simple simulation, all
* messages are automatically positively acknowledged.
*
* \param controller The CAN controller for which the callback should be registered.
* \param context The user provided context pointer, that is reobtained in the callback.
* \param handler The handler to be called on transmit acknowledge.
* \param statusMask A mask to select for which transmit statuses the handler should be called.
* \param outHandlerId The handler identifier that can be used to remove the callback.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_CanController_AddFrameTransmitHandler(SilKit_CanController* controller, void* context,
                                                                          SilKit_CanFrameTransmitHandler_t handler,
                                                                          SilKit_CanTransmitStatus statusMask,
                                                                          SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_CanController_AddFrameTransmitHandler_t)(SilKit_CanController* controller, void* context,
                                                                     SilKit_CanFrameTransmitHandler_t handler,
                                                                     SilKit_CanTransmitStatus statusMask,
                                                                     SilKit_HandlerId* outHandlerId);

/*! \brief  Remove a \ref SilKit_CanFrameTransmitHandler_t by SilKit_HandlerId on this controller 
*
* \param controller The CAN controller for which the callback should be removed.
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_CanController_RemoveFrameTransmitHandler(SilKit_CanController* controller,
                                                                             SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_CanController_RemoveFrameTransmitHandler_t)(SilKit_CanController* controller,
                                                                        SilKit_HandlerId handlerId);

/*! \brief Register a callback for CAN message reception
*
* The registered handler is called when the controller receives a
* new CAN frame.
*
* \param controller The CAN controller for which the message callback should be registered.
* \param context The user provided context pointer, that is reobtained in the callback.
* \param handler The handler to be called on reception.
* \param directionMask A bit mask defining the transmit direction of the messages (rx/tx)
* \param outHandlerId The handler identifier that can be used to remove the callback.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_CanController_AddFrameHandler(SilKit_CanController* controller, void* context,
                                                                  SilKit_CanFrameHandler_t handler,
                                                                  SilKit_Direction directionMask,
                                                                  SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_CanController_AddFrameHandler_t)(SilKit_CanController* controller, void* context,
                                                             SilKit_CanFrameHandler_t handler, SilKit_HandlerId* outHandlerId);

/*! \brief  Remove a \ref SilKit_CanFrameHandler_t by SilKit_HandlerId on this controller 
*
* \param controller The CAN controller for which the callback should be removed.
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_CanController_RemoveFrameHandler(SilKit_CanController* controller,
                                                                     SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_CanController_RemoveFrameHandler_t)(SilKit_CanController* controller, SilKit_HandlerId handlerId);

/*! \brief Register a callback for controller state changes
*
* The registered handler is called when the CanControllerState of
* the controller changes. E.g., after starting the controller, the
* state changes from SilKit_CanControllerState_Uninit to
* SilKit_CanControllerState_Started.
*
* \param controller The CAN controller for which the state change callback should be registered.
* \param context The user provided context pointer, that is reobtained in the callback.
* \param handler The handler to be called on state change.
* \param outHandlerId The handler identifier that can be used to remove the callback.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_CanController_AddStateChangeHandler(SilKit_CanController* controller, void* context,
                                                                        SilKit_CanStateChangeHandler_t handler,
                                                                        SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_CanController_AddStateChangeHandler_t)(SilKit_CanController* controller, void* context,
                                                                   SilKit_CanStateChangeHandler_t handler,
                                                                   SilKit_HandlerId* outHandlerId);

/*! \brief  Remove a \ref SilKit_CanStateChangeHandler_t by SilKit_HandlerId on this controller 
*
* \param controller The CAN controller for which the callback should be removed.
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_CanController_RemoveStateChangeHandler(SilKit_CanController* controller,
                                                                           SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_CanController_RemoveStateChangeHandler_t)(SilKit_CanController* controller,
                                                                      SilKit_HandlerId handlerId);

/*! \brief Register a callback for changes of the controller's error state
*
* The registered handler is called when the CanErrorState of the
* controller changes. During normal operation, the controller
* should be in state SilKit_CanErrorState_ErrorActive. The states correspond
* to the error state handling protocol of the CAN specification.
*
* \param controller The CAN controller for which the error state callback should be registered.
* \param context The user provided context pointer, that is reobtained in the callback.
* \param handler The handler to be called on error state change.
* \param outHandlerId The handler identifier that can be used to remove the callback.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_CanController_AddErrorStateChangeHandler(SilKit_CanController* controller,
                                                                             void* context,
                                                                             SilKit_CanErrorStateChangeHandler_t handler,
                                                                             SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_CanController_AddErrorStateChangeHandler_t)(SilKit_CanController* controller, void* context,
                                                                        SilKit_CanErrorStateChangeHandler_t handler,
                                                                        SilKit_HandlerId* outHandlerId);

/*! \brief  Remove a \ref SilKit_CanErrorStateChangeHandler_t by SilKit_HandlerId on this controller 
*
* \param controller The CAN controller for which the callback should be removed.
* \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
*/
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_CanController_RemoveErrorStateChangeHandler(SilKit_CanController* controller,
                                                                                SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode (SilKitFPTR *SilKit_CanController_RemoveErrorStateChangeHandler_t)(SilKit_CanController* controller,
                                                                           SilKit_HandlerId handlerId);


SILKIT_END_DECLS

#pragma pack(pop)
