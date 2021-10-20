/* Copyright (c) Vector Informatik GmbH. All rights reserved. */

#pragma once
#include <stdint.h>
#include "ib/capi/Utils.h"
#include "ib/capi/InterfaceIdentifiers.h"

#ifdef __cplusplus
extern "C"
{
#endif
    typedef void ib_SimulationParticipant;

    /*! \brief A CAN frame
     */
    struct ib_CanFrame
    {
        uint32_t id; //!< CAN Identifier
        uint32_t flags; //!< CAN Arbitration and Control Field Flags; see ib_CanMessageFlag
        uint8_t dlc; //!< Data Length Code - determined by the Network Simulator

        uint32_t dataLength; //!< Payload data byte count
        uint8_t data[64]; //!< Payload data field

    };

    typedef struct ib_CanFrame ib_CanFrame;

    struct ib_CanFrame_Meta
    {
        ib_InterfaceIdentifier interfaceId; //!< The interface id that specifies which version of this struct was obtained
        ib_NanosecondsTime timestamp; //!< Reception time
        ib_CanFrame* canFrame; //!< The Can Frame that corresponds to the meta data
    };

    typedef struct ib_CanFrame_Meta ib_CanFrame_Meta;

    /*! The available flags within the flags member of a Can frame.
    */
    typedef uint32_t ib_CanFrameFlag;
    #define ib_CanFrameFlag_ide (((uint32_t) 1) << 9) //!< Identifier Extension
    #define ib_CanFrameFlag_rtr (((uint32_t) 1) << 4) //!< Remote Transmission Request
    #define ib_CanFrameFlag_fdf (((uint32_t) 1) << 12) //!< FD Format Indicator
    #define ib_CanFrameFlag_brs (((uint32_t) 1) << 13) //!< Bit Rate Switch  (for FD Format only)
    #define ib_CanFrameFlag_esi (((uint32_t) 1) << 14) //!< Error State indicator (for FD Format only)


    typedef int32_t ib_CanTransmitStatus;
    /*! The message was successfully transmitted on the CAN bus.
    */
    #define ib_CanTransmitStatus_Transmitted ((int32_t) 0)
    /*! (currently not in use)
    *
    * The transmit queue was reset.
    */
    #define ib_CanTransmitStatus_Canceled ((int32_t) 1)
    /*! The transmit request was rejected, because the transmit queue is full.
    */
    #define ib_CanTransmitStatus_TransmitQueueFull ((int32_t) 2)
    /*! (currently not in use)
    *
    * The transmit request was rejected, because there is already another request with the same transmitId.
    */
    #define ib_CanTransmitStatus_DuplicatedTransmitId ((int32_t) 3)

    /*! \brief The acknowledgment of a CAN message, sent to the controller
    */
    struct ib_CanTransmitAcknowledge
    {
        void* userContext; //!< Value that was provided by user in corresponding parameter on send of Can frame
        ib_NanosecondsTime timestamp; //!< Reception time
        ib_CanTransmitStatus status; //!< Status of the CanTransmitRequest
    };

    typedef struct ib_CanTransmitAcknowledge ib_CanTransmitAcknowledge;

    /*! \brief CAN Controller state according to AUTOSAR specification AUTOSAR_SWS_CANDriver 4.3.1
    */
    typedef int32_t ib_CanControllerState;
    /*! CAN controller is not initialized (initial state after reset).
    */
    #define ib_CanControllerState_Uninit  ((int32_t) 0)
    /*! CAN controller is initialized but does not participate on the CAN bus.
    */
    #define ib_CanControllerState_Stopped ((int32_t) 1)
    /*! CAN controller is in normal operation mode.
    */
    #define ib_CanControllerState_Started ((int32_t) 2)
    /*! CAN controller is in sleep mode which is similar to the Stopped state.
    */
    #define ib_CanControllerState_Sleep   ((int32_t) 3)

    /*! \brief Error state of a CAN node according to CAN specification.
    */
    typedef int ib_CanErrorState;
    /*! Error State is Not Available, because CAN controller is in state Uninit.
    *
    * *AUTOSAR Doc:* Successful transmission.
    */
    #define ib_CanErrorState_NotAvailable ((int32_t) 0)
    /*! Error Active Mode, the CAN controller is allowed to send messages and active error flags.
    */
    #define ib_CanErrorState_ErrorActive  ((int32_t) 1)
    /*! Error Passive Mode, the CAN controller is still allowed to send messages, but must not send active error flags.
    */
    #define ib_CanErrorState_ErrorPassive ((int32_t) 2)
    /*! (currently not in use)
    *
    * *AUTOSAR Doc:* Bus Off Mode, the CAN controller does not take part in communication.
    */
    #define ib_CanErrorState_BusOff       ((int32_t) 3)

    typedef void ib_CanController;

    /*! Callback type to indicate that a CanTransmitAcknowledge has been received.
    * \param context The by the user provided context on registration.
    * \param controller The Can controller that received the acknowledge.
    * \param acknowledge The acknowledge and its data.
    */
    typedef void ib_CanTransmitStatusHandler_t(void* context, ib_CanController* controller, ib_CanTransmitAcknowledge* acknowledge);

    /*! Callback type to indicate that a CanMessage has been received.
    * \param context The by the user provided context on registration.
    * \param controller The Can controller that received the message.
    * \param metaData The struct containing meta data and referencing the can frame itself.
    */
    typedef void ib_CanReceiveMessageHandler_t(void* context, ib_CanController* controller, ib_CanFrame_Meta* metaData);

    /*! Callback type to indicate that the State of the Can Controller has changed.
    * \param context The by the user provided context on registration.
    * \param controller The Can controller that changed its state.
    * \param state The new state of the Can controller.
    */
    typedef void ib_CanStateChangedHandler_t(void* context, ib_CanController* controller, ib_CanControllerState state);

    /*! Callback type to indicate that the controller Can error state has changed.
    * \param context The by the user provided context on registration.
    * \param controller The Can controller that received the message.
    * \param errorState The new can error state.
    */
    typedef void ib_CanErrorStateChangedHandler_t(void* context, ib_CanController* controller, ib_CanErrorState errorState);

    typedef ib_ReturnCode(*ib_CanController_create_t)(ib_CanController** outCanController, ib_SimulationParticipant* participant, const char* name);
    /*! \brief Create a CAN controller at this IB simulation participant.
    * \param outCanController Pointer into which the resulting Can controller will be written (out parameter).
    * \param participant The simulation participant at which the Can controller should be created.
    * \param name The name of the new Can controller.
    *
    * The lifetime of the resulting Can controller is directly bound to the lifetime of the simulation participant.
    * There is no futhert cleanup necessary except for destroying the simulation participant at the end of the simulation
    */
    CIntegrationBusAPI ib_ReturnCode ib_CanController_create(ib_CanController** outCanController, ib_SimulationParticipant* participant, const char* name);


    typedef ib_ReturnCode (*ib_CanController_Start_t)(ib_CanController* self);
    /*! \brief Start the CAN controller
    *
    * NB: Only supported in VIBE simulation, the command is ignored
    * in simple simulation.
    *
    * \ref ib_CanController_Reset(), \ref ib_CanController_Stop(), \ref ib_CanController_Sleep()
    */
    CIntegrationBusAPI ib_ReturnCode ib_CanController_Start(ib_CanController* self);

    typedef ib_ReturnCode (*ib_CanController_Stop_t)(ib_CanController* self);
    /*! \brief Stop the CAN controller
    *
    * NB: Only supported in VIBE simulation, the command is ignored
    * in simple simulation.
    *
    * \ref ib_CanController_Reset(), \ref ib_CanController_Start(), \ref ib_CanController_Sleep()
    */
    CIntegrationBusAPI ib_ReturnCode ib_CanController_Stop(ib_CanController* self);


    typedef ib_ReturnCode (*ib_CanController_Reset_t)(ib_CanController* self);
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
    * \ref ib_CanController_Start(), \ref ib_CanController_Stop(), \ref ib_CanController_Sleep()
    */
    CIntegrationBusAPI ib_ReturnCode ib_CanController_Reset(ib_CanController* self);

    typedef ib_ReturnCode(*ib_CanController_Sleep_t)(ib_CanController* self);
    /*! \brief Put the CAN controller in sleep mode
    *
    * NB: Only supported in VIBE simulation, the command is ignored
    * in simple simulation.
    *
    * \ref ib_CanController_Reset(), ib_CanController_Start(), \ref ib_CanController_Stop()
    */
    CIntegrationBusAPI ib_ReturnCode ib_CanController_Sleep(ib_CanController* self);


    typedef ib_ReturnCode (*ib_CanController_SendFrame_t)(ib_CanController* self, ib_CanFrame* frame, void* userContext);
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
    * \param self The Can controller that should send the Can frame.
    * \param frame The Can frame to transmit.
    * \param userContext A user provided context pointer, that is
    * reobtained in the ib_CanController_RegisterTransmitStatusHandler
    * handler.
    */
    CIntegrationBusAPI ib_ReturnCode ib_CanController_SendFrame(ib_CanController* self, ib_CanFrame* frame, void* userContext);
    

    typedef ib_ReturnCode(*ib_CanController_SetBaudRate_t)(ib_CanController* self, uint32_t rate, uint32_t fdRate);
    /*! \brief Configure the baudrate of the controller
    *
    * \param self The Can controller for which the baudrate should be changed.
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
    CIntegrationBusAPI ib_ReturnCode ib_CanController_SetBaudRate(ib_CanController* self, uint32_t rate, uint32_t fdRate);

    typedef ib_ReturnCode (*ib_CanController_RegisterTransmitStatusHandler_t)(ib_CanController* self, void* context, ib_CanTransmitStatusHandler_t* handler);
    /*! \brief Register a callback for the TX status of sent CAN messages
    *
    * The registered handler is called when a CAN message was
    * successfully transmitted on the bus or when an error occurred.
    *
    * NB: Full support in VIBE simulation. In simple simulation, all
    * messages are automatically positively acknowledged.
    *
    * \param self The Can controller for which the callback should be registered.
    * \param context The user provided context pointer, that is reobtained in the callback.
    * \param handler The handler to be called on transmit acknowledge.
    */
    CIntegrationBusAPI ib_ReturnCode ib_CanController_RegisterTransmitStatusHandler(ib_CanController* self, void* context, ib_CanTransmitStatusHandler_t* handler);

    typedef ib_ReturnCode (*ib_CanController_RegisterReceiveMessageHandler_t)(ib_CanController* self, void* context, ib_CanReceiveMessageHandler_t* handler);
    /*! \brief Register a callback for CAN message reception
    *
    * The registered handler is called when the controller receives a
    * new Can frame.
    *
    * \param self The Can controller for which the message callback should be registered.
    * \param context The user provided context pointer, that is reobtained in the callback.
    * \param handler The handler to be called on reception.
    */
    CIntegrationBusAPI ib_ReturnCode ib_CanController_RegisterReceiveMessageHandler(ib_CanController* self, void* context, ib_CanReceiveMessageHandler_t* handler);
    
    typedef ib_ReturnCode (*ib_CanController_RegisterStateChangedHandler_t)(ib_CanController* self, void* context, ib_CanStateChangedHandler_t* handler);
    /*! \brief Register a callback for controller state changes
    *
    * The registered handler is called when the CanControllerState of
    * the controller changes. E.g., after starting the controller, the
    * state changes from ib_CanControllerState_Uninit to
    * ib_CanControllerState_Started.
    *
    * NB: Only supported in VIBE simulation. In simple simulation,
    * the handler is never called.
    *
    * \param self The Can controller for which the state change callback should be registered.
    * \param context The user provided context pointer, that is reobtained in the callback.
    * \param handler The handler to be called on state change.
    */
    CIntegrationBusAPI ib_ReturnCode ib_CanController_RegisterStateChangedHandler(ib_CanController* self, void* context, ib_CanStateChangedHandler_t* handler);
    
    typedef ib_ReturnCode (*ib_CanController_RegisterErrorStateChangedHandler_t)(ib_CanController* self, void* context, ib_CanErrorStateChangedHandler_t* handler);
    /*! \brief Register a callback for changes of the controller's error state
    *
    * The registered handler is called when the CanErrorState of the
    * controller changes. During normal operation, the controller
    * should be in state ib_CanErrorState_ErrorActive. The states correspond
    * to the error state handling protocol of the CAN specification.
    *
    * NB: Only supported in VIBE simulation. In simple simulation,
    * the handler is never called.
    *
    * \param self The Can controller for which the error state callback should be registered.
    * \param context The user provided context pointer, that is reobtained in the callback.
    * \param handler The handler to be called on error state change.
    */
    CIntegrationBusAPI ib_ReturnCode ib_CanController_RegisterErrorStateChangedHandler(ib_CanController* self, void* context, ib_CanErrorStateChangedHandler_t* handler);



#ifdef __cplusplus
}
#endif
