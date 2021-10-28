#pragma once
#include <stdint.h>
#include "ib/capi/Utils.h"
#include "ib/capi/InterfaceIdentifiers.h"

#pragma pack(push)
#pragma pack(8)

__IB_BEGIN_DECLS


typedef int32_t ib_EthernetTransmitStatus;

/*! The message was successfully transmitted on the CAN bus. */
#define ib_EthernetTransmitStatus_Transmitted ((int32_t) 0)

/*! The transmit request was rejected, because the Ethernet controller is not active. */
#define ib_EthernetTransmitStatus_ControllerInactive ((int32_t) 1)

/*! The transmit request was rejected, because the Ethernet link is down. */
#define ib_EthernetTransmitStatus_LinkDown ((int32_t) 2)

/*! The transmit request was dropped, because the transmit queue is full. */
#define ib_EthernetTransmitStatus_Dropped ((int32_t) 3)

/*! The given raw Ethernet frame is ill formated (e.g. frame length is too small or too large, etc.). */
#define ib_EthernetTransmitStatus_InvalidFrameFormat ((int32_t) 4)


typedef int32_t ib_EthernetState;

//!< The Ethernet controller is switched off (default after reset).
#define ib_EthernetState_Inactive ((int32_t) 0)

//!< The Ethernet controller is active, but a link to another Ethernet controller in not yet established.
#define ib_EthernetState_LinkDown ((int32_t) 1)

//!< The Ethernet controller is active and the link to another Ethernet controller is established.
#define ib_EthernetState_LinkUp ((int32_t) 2)

#pragma pack(push)
#pragma pack(1)
struct ib_EthernetFrame_Header 
{
    uint8_t destinationMac[6];
    uint8_t sourceMac[6];
    uint16_t etherType;
};
struct ib_EthernetFrame_HeaderVlanTagged 
{
    uint8_t destinationMac[6];
    uint8_t sourceMac[6];
    uint8_t vlanTag[4];
    uint16_t etherType;
};
#pragma pack(pop)

typedef struct ib_EthernetFrame_Header ib_EthernetFrame_Header;
typedef struct ib_EthernetFrame_HeaderVlanTagged ib_EthernetFrame_HeaderVlanTagged;

struct ib_EthernetFrame
{
    union {
        uint8_t* frameData; //!< Ethernet raw frame
        ib_EthernetFrame_Header* frameHeader; //!< Ethernet frame without VlanTag as in IEEE 802.3
        ib_EthernetFrame_HeaderVlanTagged* frameHeaderVlanTagged; //!< Ethernet frame with VlanTag as in IEEE 802.1Q
    };
    size_t frameSize; //!< The current frame size
};

typedef struct ib_EthernetFrame ib_EthernetFrame;
struct ib_EthernetMessage
{
    ib_InterfaceIdentifier interfaceId; //!< The interface id that specifies which version of this struct was obtained
    ib_NanosecondsTime timestamp;
    ib_EthernetFrame* ethernetFrame;
};

typedef struct ib_EthernetMessage ib_EthernetMessage;

struct ib_EthernetTransmitAcknowledge
{
    ib_InterfaceIdentifier interfaceId; //!< The interface id that specifies which version of this struct was obtained
    void* userContext; //!< Value that was provided by user in corresponding parameter on send of Ethernet frame
    ib_NanosecondsTime timestamp; //!< Reception time
    ib_EthernetTransmitStatus status; //!< Status of the EthernetTransmitRequest
};

typedef struct ib_EthernetTransmitAcknowledge ib_EthernetTransmitAcknowledge;

typedef void ib_EthernetController;

/*! Callback type to indicate that a EthernetMessage has been received.
* \param context The context provided by the user upon registration.
* \param controller The Ethernet controller that received the message.
* \param metaData The struct containing meta data and referencing the Ethernet frame itself.
*/
typedef void ib_EthernetReceiveMessageHandler_t(void* context, ib_EthernetController* controller, 
    ib_EthernetMessage* message);
    
/*! Callback type to indicate that a EthernetFrame has been sent.
* \param context The by the user provided context on registration.
* \param controller The Ethernet controller that received the acknowledge.
* \param acknowledge The acknowledge and its data.
*/
typedef void ib_EthernetFrameAckHandler_t(void* context, ib_EthernetController* controller, 
    ib_EthernetTransmitAcknowledge* acknowledge);
    
/*! Callback type to indicate that the Ethernet controller state has changed.
* \param context The by the user provided context on registration.
* \param controller The Ethernet controller whose state did change.
* \param state The new state.
*/
typedef void ib_EthernetStateChangedHandler_t(void* context, ib_EthernetController* controller, 
    ib_EthernetState state);

/*! Callback type to indicate that the link bit rate has changed.
* \param context The by the user provided context on registration.
* \param controller The Ethernet controller that is affected.
* \param bitrate The new bitrate in kbits per second of the Ethernet link.
*/
typedef void ib_EthernetBitRateChangedHandler_t(void* context, ib_EthernetController* controller, uint32_t bitrate);

typedef ib_ReturnCode(*ib_EthernetController_create_t)(ib_EthernetController** outController, 
    ib_SimulationParticipant* participant, const char* name);
/*! \brief Create an Ethernet controller at this IB simulation participant.
* 
* \param outController A pointer to a pointer in which the ethernet controller will be stored (out parameter).
* \param participant The simulation participant for whicht the ethernet controller should be created.
* \param name The utf8 encoded name of the new Ethernet controller.
* \result A return code identifying the success/failure of the call.
* 
* \see ib::mw::IComAdapter::CreateEthController
*/
CIntegrationBusAPI ib_ReturnCode ib_EthernetController_create(ib_EthernetController** outController, 
    ib_SimulationParticipant* participant, const char* name);

typedef ib_ReturnCode(*ib_EthernetController_Activate_t)(ib_EthernetController* self);
/*! \brief Activates the Ethernet controller
*
* Upon activation of the controller, the controller attempts to
* establish a link. Messages can only be sent once the link has
* been successfully established,
*
* NB: Only supported in VIBE simulation! In simple simulation,
* messages can be sent without need to call ib_EthernetController_Activate()
* 
* \param self The ethernet controller to be activated.
* \result A return code identifying the success/failure of the call.
* 
* \see ib::sim::eth::IEthController::Activate
*/
CIntegrationBusAPI ib_ReturnCode ib_EthernetController_Activate(ib_EthernetController* self);

typedef ib_ReturnCode(*ib_EthernetController_Deactivate_t)(ib_EthernetController* self);
/*! \brief Deactivate the Ethernet controller
*
* Deactivate the controller and shut down the link. The
* controller will no longer receive messages, and it cannot send
* messages anymore.
*
* NB: Only supported in VIBE simulation! In simple simulation,
* ib_EthernetController_Deactivate() has no effects and messages can still be sent.
* 
* \param self The ethernet controller to be deactivated.
* \result A return code identifying the success/failure of the call.
* 
* \see ib::sim::eth::IEthController::Deactivate
*/
CIntegrationBusAPI ib_ReturnCode ib_EthernetController_Deactivate(ib_EthernetController* self);

typedef ib_ReturnCode(*ib_EthernetController_RegisterReceiveMessageHandler_t)(ib_EthernetController* self, 
    void* context, ib_EthernetReceiveMessageHandler_t* handler);
/*! \brief Register a callback for Ethernet message reception
*
* The handler is called when the controller receives a new
* Ethernet message.
* 
* \param self The Ethernet controller for which the message callback should be registered.
* \param context The user provided context pointer, that is reobtained in the callback.
* \param handler The handler to be called on reception.
* \result A return code identifying the success/failure of the call.
* 
* \see ib::sim::eth::IEthController::RegisterReceiveMessageHandler
*/
CIntegrationBusAPI ib_ReturnCode ib_EthernetController_RegisterReceiveMessageHandler(ib_EthernetController* self, 
    void* context, ib_EthernetReceiveMessageHandler_t* handler);

typedef ib_ReturnCode(*ib_EthernetController_RegisterFrameAckHandler_t)(ib_EthernetController* self, void* context,
    ib_EthernetFrameAckHandler_t* handler);
/*! \brief Register a callback for Ethernet transmit acknowledgements
*
* The handler is called when a previously sent message was
* successfully transmitted or when the transmission has
* failed. The original message is identified by the userContext.
*
* NB: Full support in VIBE Ethernet simulation. In simple
* simulation, all messages are immediately positively
* acknowledged by a receiving controller.
* 
* \param self The Ethernet controller for which the message acknowledge callback should be registered.
* \param context The user provided context pointer, that is reobtained in the callback.
* \param handler The handler to be called on reception.
* \result A return code identifying the success/failure of the call.
* 
* \see ib::sim::eth::IEthController::RegisterMessageAckHandler
*/
CIntegrationBusAPI ib_ReturnCode ib_EthernetController_RegisterFrameAckHandler(ib_EthernetController* self,
    void* context, ib_EthernetFrameAckHandler_t* handler);


typedef ib_ReturnCode(*ib_EthernetController_RegisterStateChangedHandler_t)(ib_EthernetController* self,
    void* context, ib_EthernetStateChangedHandler_t* handler);
/*! \brief Register a callback for changes of the controller state
*
* The handler is called when the state of the controller
* changes. E.g., a call to ib_EthernetController_Activate() causes the controller to
* change from state ib_EthernetState_Inactive to ib_EthernetState_LinkDown. Later, when the link
* has been established, the state changes again from ib_EthernetState_LinkDown to
* ib_EthernetState_LinkUp. Similarly, the status changes back to ib_EthernetState_Inactive upon a
* call to ib_EthernetController_Deactivate().
*
* NB: Only supported in VIBE Ethernet simulation.
*
* \param self The Ethernet controller for which the message acknowledge callback should be registered.
* \param context The user provided context pointer, that is reobtained in the callback.
* \param handler The handler to be called on reception.
* \result A return code identifying the success/failure of the call.
* 
* \see ib::sim::eth::IEthController::RegisterStateChangedHandler
*/
CIntegrationBusAPI ib_ReturnCode ib_EthernetController_RegisterStateChangedHandler(ib_EthernetController* self, 
    void* context, ib_EthernetStateChangedHandler_t* handler);


typedef ib_ReturnCode(*ib_EthernetController_RegisterBitRateChangedHandler_t)(ib_EthernetController* self, 
    void* context, ib_EthernetBitRateChangedHandler_t* handler);
/*! \brief Register a callback for changes of the link bit rate
*
* The handler is called when the bit rate of the connected link
* changes. This is typically the case when a link was
* successfully established, or the controller was deactivated.
*
* NB: Only supported in VIBE Ethernet simulation.
* 
* \param self The Ethernet controller for which the bitrate change callback should be registered.
* \param context The user provided context pointer, that is reobtained in the callback.
* \param handler The handler to be called on change.
* \result A return code identifying the success/failure of the call.
* 
* \see ib::sim::eth::IEthController::RegisterBitRateChangedHandler
*/
CIntegrationBusAPI ib_ReturnCode ib_EthernetController_RegisterBitRateChangedHandler(ib_EthernetController* self, 
    void* context, ib_EthernetBitRateChangedHandler_t* handler);

typedef ib_ReturnCode(*ib_EthernetController_SendFrame_t)(ib_EthernetController* self, ib_EthernetFrame* frame, 
    void* userContext);
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
*
* \param self The Ethernet controller that should send the frame.
* \param frame The Ethernet frame to be sent.
* \param userContext The user provided context pointer, that is reobtained in the callback of RegisterFrameAckHandler()
* \result A return code identifying the success/failure of the call.
* 
* \see ib::sim::eth::IEthController::SendFrame
*/
CIntegrationBusAPI ib_ReturnCode ib_EthernetController_SendFrame(ib_EthernetController* self, ib_EthernetFrame* frame, 
    void* userContext);

__IB_END_DECLS

#pragma pack(pop)