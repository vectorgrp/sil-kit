// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once
#include <stdint.h>
#include "silkit/capi/SilKitMacros.h"
#include "silkit/capi/Types.h"
#include "silkit/capi/InterfaceIdentifiers.h"

#pragma pack(push)
#pragma pack(8)

SILKIT_BEGIN_DECLS

/*! \brief Device address on the I2C bus. 7-bit addresses use the range 0x00–0x7F,
 *         10-bit addresses use the range 0x000–0x3FF.
 *         Address 0x00 is the General Call address (broadcast to all slaves).
 */
typedef uint16_t SilKit_I2cAddress;

/*! \brief Addressing mode used for an I2C transfer. */
typedef int32_t SilKit_I2cAddressMode;
#define SilKit_I2cAddressMode_7Bit  ((SilKit_I2cAddressMode)0) //!< Standard 7-bit addressing
#define SilKit_I2cAddressMode_10Bit ((SilKit_I2cAddressMode)1) //!< Extended 10-bit addressing

/*! \brief Direction of an I2C data transfer (from the master's perspective). */
typedef int32_t SilKit_I2cTransferDirection;
#define SilKit_I2cTransferDirection_Write ((SilKit_I2cTransferDirection)0) //!< Master writes data to slave
#define SilKit_I2cTransferDirection_Read  ((SilKit_I2cTransferDirection)1) //!< Master reads data from slave

/*! \brief Bus speed mode. Informational; ignored by trivial simulation, used by network simulators. */
typedef int32_t SilKit_I2cSpeedMode;
#define SilKit_I2cSpeedMode_Standard  ((SilKit_I2cSpeedMode)0) //!< Standard mode: up to 100 kbit/s
#define SilKit_I2cSpeedMode_Fast      ((SilKit_I2cSpeedMode)1) //!< Fast mode: up to 400 kbit/s
#define SilKit_I2cSpeedMode_FastPlus  ((SilKit_I2cSpeedMode)2) //!< Fast-mode plus: up to 1 Mbit/s
#define SilKit_I2cSpeedMode_HighSpeed ((SilKit_I2cSpeedMode)3) //!< High-speed mode: up to 3.4 Mbit/s

/*! \brief Result of an I2C transfer as seen by the master. */
typedef int32_t SilKit_I2cTransmitStatus;
#define SilKit_I2cTransmitStatus_Transmitted     ((SilKit_I2cTransmitStatus)0) //!< All bytes ACKed by slave
#define SilKit_I2cTransmitStatus_AddressNak      ((SilKit_I2cTransmitStatus)1) //!< No slave responded to address
#define SilKit_I2cTransmitStatus_DataNak         ((SilKit_I2cTransmitStatus)2) //!< Slave NACKed a data byte
#define SilKit_I2cTransmitStatus_ArbitrationLost ((SilKit_I2cTransmitStatus)3) //!< Master lost arbitration (multi-master; detailed sim only)
#define SilKit_I2cTransmitStatus_BusError        ((SilKit_I2cTransmitStatus)4) //!< Protocol violation or invalid request

/*! \brief Operating mode of an I2C controller. */
typedef int32_t SilKit_I2cControllerMode;
#define SilKit_I2cControllerMode_Inactive ((SilKit_I2cControllerMode)0) //!< Controller not initialized
#define SilKit_I2cControllerMode_Master   ((SilKit_I2cControllerMode)1) //!< Controller acts as bus master
#define SilKit_I2cControllerMode_Slave    ((SilKit_I2cControllerMode)2) //!< Controller acts as addressed slave

/*! \brief An I2C frame representing a single transfer (START…STOP).
 *
 *  For Write transfers, \c data contains the payload sent by the master.
 *  For Read transfers, \c data is empty in the request; the slave response is
 *  delivered as a separate \ref SilKit_I2cFrameEvent.
 */
struct SilKit_I2cFrame
{
    SilKit_StructHeader         structHeader; //!< The interface id specifying which version of this struct was obtained
    SilKit_I2cAddress           address;      //!< Target slave address
    SilKit_I2cAddressMode       addressMode;  //!< 7-bit or 10-bit addressing
    SilKit_I2cTransferDirection direction;    //!< Write or Read
    SilKit_ByteVector           data;         //!< Payload; empty for Read requests
};
typedef struct SilKit_I2cFrame SilKit_I2cFrame;

/*! \brief Event delivered to frame handlers when an I2C frame is observed on the bus. */
struct SilKit_I2cFrameEvent
{
    SilKit_StructHeader    structHeader; //!< The interface id specifying which version of this struct was obtained
    SilKit_NanosecondsTime timestamp;   //!< Simulation time of the transfer
    SilKit_I2cFrame*       frame;       //!< The I2C frame
    SilKit_Direction       direction;   //!< TX (sent by this controller) or RX (received)
    void*                  userContext; //!< Optional pointer provided by user when sending the frame
};
typedef struct SilKit_I2cFrameEvent SilKit_I2cFrameEvent;

/*! \brief Acknowledgment delivered to the master after a transfer completes (or fails). */
struct SilKit_I2cFrameTransmitEvent
{
    SilKit_StructHeader      structHeader; //!< The interface id specifying which version of this struct was obtained
    SilKit_NanosecondsTime   timestamp;   //!< Simulation time of the acknowledgment
    SilKit_I2cAddress        address;     //!< Slave address of the completed transfer
    SilKit_I2cTransmitStatus status;      //!< Outcome of the transfer
    void*                    userContext; //!< Optional pointer provided by user when sending the frame
};
typedef struct SilKit_I2cFrameTransmitEvent SilKit_I2cFrameTransmitEvent;

/*! \brief Configuration passed to \ref SilKit_I2cController_Init. */
struct SilKit_I2cControllerConfig
{
    SilKit_StructHeader      structHeader;    //!< The interface id specifying which version of this struct was obtained
    SilKit_I2cControllerMode mode;            //!< Master or Slave
    SilKit_I2cAddress        slaveAddress;    //!< Own address when in Slave mode; ignored for Master
    SilKit_I2cAddressMode    slaveAddressMode;//!< Address width when in Slave mode; ignored for Master
    SilKit_I2cSpeedMode      speedMode;       //!< Bus speed (informational; trivial sim ignores)
};
typedef struct SilKit_I2cControllerConfig SilKit_I2cControllerConfig;

typedef struct SilKit_I2cController SilKit_I2cController;

/*! Callback type invoked when an I2C frame is observed on the bus.
 * \param context   User-provided context pointer registered with the handler.
 * \param controller The controller that observed the frame.
 * \param frameEvent The frame event including timestamp, frame, and direction.
 */
typedef void(SilKitFPTR* SilKit_I2cFrameHandler_t)(void* context, SilKit_I2cController* controller,
                                                    SilKit_I2cFrameEvent* frameEvent);

/*! Callback type invoked when a frame transmission completes or fails.
 * \param context           User-provided context pointer registered with the handler.
 * \param controller        The controller that sent the frame.
 * \param frameTransmitEvent The transmit event including timestamp, address, and status.
 */
typedef void(SilKitFPTR* SilKit_I2cFrameTransmitHandler_t)(void* context, SilKit_I2cController* controller,
                                                            SilKit_I2cFrameTransmitEvent* frameTransmitEvent);

/*! \brief Create an I2C controller at this SIL Kit simulation participant.
 *
 * \param outI2cController Pointer that refers to the resulting I2C controller (out parameter).
 * \param participant      The simulation participant at which the controller should be created.
 * \param name             The name of the new controller (UTF-8).
 * \param network          The I2C network to operate in (UTF-8).
 *
 * The lifetime of the resulting controller is bound to the lifetime of the participant.
 * The object returned must not be deallocated using free().
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_I2cController_Create(SilKit_I2cController** outI2cController,
                                                                    SilKit_Participant* participant, const char* name,
                                                                    const char* network);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_I2cController_Create_t)(SilKit_I2cController** outI2cController,
                                                                      SilKit_Participant* participant,
                                                                      const char* name, const char* network);

/*! \brief Initialize the I2C controller with the given configuration.
 *
 * Must be called before SendFrame or SetReadResponse. Broadcasts the controller's
 * configuration to other participants on the same network so that slave address
 * routing can be established.
 *
 * \param controller The I2C controller to configure.
 * \param config     Controller configuration (mode, slave address, speed mode).
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_I2cController_Init(SilKit_I2cController* controller,
                                                                  SilKit_I2cControllerConfig* config);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_I2cController_Init_t)(SilKit_I2cController* controller,
                                                                    SilKit_I2cControllerConfig* config);

/*! \brief Initiate an I2C transfer (master only).
 *
 * For Write transfers, \c frame->data contains the payload.
 * For Read transfers, \c frame->data should be empty; the response data is delivered
 * via the frame handler.
 *
 * \param controller  The I2C controller sending the frame.
 * \param frame       The I2C frame to transmit.
 * \param userContext Optional user pointer reobtained in the frame transmit handler.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_I2cController_SendFrame(SilKit_I2cController* controller,
                                                                       SilKit_I2cFrame* frame, void* userContext);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_I2cController_SendFrame_t)(SilKit_I2cController* controller,
                                                                         SilKit_I2cFrame* frame, void* userContext);

/*! \brief Pre-load the read-response buffer for a slave controller.
 *
 * When a master issues a Read to this slave's address, the provided data
 * is returned as the slave's response. Must be called before the master
 * issues the Read.
 *
 * \param controller The slave I2C controller.
 * \param data       Buffer containing the data to return on the next Read request.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_I2cController_SetReadResponse(SilKit_I2cController* controller,
                                                                             SilKit_ByteVector* data);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_I2cController_SetReadResponse_t)(SilKit_I2cController* controller,
                                                                               SilKit_ByteVector* data);

/*! \brief Register a callback for I2C frame reception.
 *
 * \param controller     The I2C controller for which to register the callback.
 * \param context        User-provided context pointer reobtained in the callback.
 * \param handler        The handler called on each observed frame.
 * \param directionMask  Bitmask of \ref SilKit_Direction values to filter TX, RX, or both.
 * \param outHandlerId   Identifier that can be used to remove the callback.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_I2cController_AddFrameHandler(SilKit_I2cController* controller,
                                                                             void* context,
                                                                             SilKit_I2cFrameHandler_t handler,
                                                                             SilKit_Direction directionMask,
                                                                             SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_I2cController_AddFrameHandler_t)(SilKit_I2cController* controller,
                                                                               void* context,
                                                                               SilKit_I2cFrameHandler_t handler,
                                                                               SilKit_Direction directionMask,
                                                                               SilKit_HandlerId* outHandlerId);

/*! \brief Remove a frame handler by its handler id. */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_I2cController_RemoveFrameHandler(SilKit_I2cController* controller,
                                                                                SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_I2cController_RemoveFrameHandler_t)(SilKit_I2cController* controller,
                                                                                  SilKit_HandlerId handlerId);

/*! \brief Register a callback for I2C frame transmit events (master only).
 *
 * \param controller   The I2C controller for which to register the callback.
 * \param context      User-provided context pointer reobtained in the callback.
 * \param handler      The handler called when a transfer completes or fails.
 * \param outHandlerId Identifier that can be used to remove the callback.
 */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_I2cController_AddFrameTransmitHandler(
    SilKit_I2cController* controller, void* context, SilKit_I2cFrameTransmitHandler_t handler,
    SilKit_HandlerId* outHandlerId);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_I2cController_AddFrameTransmitHandler_t)(
    SilKit_I2cController* controller, void* context, SilKit_I2cFrameTransmitHandler_t handler,
    SilKit_HandlerId* outHandlerId);

/*! \brief Remove a frame transmit handler by its handler id. */
SilKitAPI SilKit_ReturnCode SilKitCALL SilKit_I2cController_RemoveFrameTransmitHandler(
    SilKit_I2cController* controller, SilKit_HandlerId handlerId);

typedef SilKit_ReturnCode(SilKitFPTR* SilKit_I2cController_RemoveFrameTransmitHandler_t)(
    SilKit_I2cController* controller, SilKit_HandlerId handlerId);

SILKIT_END_DECLS

#pragma pack(pop)
