// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>

#include "silkit/services/datatypes.hpp"
#include "silkit/util/Span.hpp"

#include "silkit/capi/I2c.h"

// ================================================================================
//  I2C specific data types
// ================================================================================

namespace SilKit {
namespace Services {
namespace I2c {

using I2cAddress = SilKit_I2cAddress;

using I2cAddressModeMask = SilKit_I2cAddressMode;

//! \brief Addressing mode used for an I2C transfer.
enum class I2cAddressMode : I2cAddressModeMask
{
    AddressMode7Bit  = SilKit_I2cAddressMode_7Bit,  //!< Standard 7-bit addressing
    AddressMode10Bit = SilKit_I2cAddressMode_10Bit, //!< Extended 10-bit addressing
};

using I2cTransferDirectionMask = SilKit_I2cTransferDirection;

//! \brief Direction of an I2C data transfer from the master's perspective.
enum class I2cTransferDirection : I2cTransferDirectionMask
{
    Write = SilKit_I2cTransferDirection_Write, //!< Master writes data to slave
    Read  = SilKit_I2cTransferDirection_Read,  //!< Master reads data from slave
};

using I2cSpeedModeMask = SilKit_I2cSpeedMode;

//! \brief Bus speed mode. Informational; ignored by trivial simulation, used by network simulators.
enum class I2cSpeedMode : I2cSpeedModeMask
{
    Standard  = SilKit_I2cSpeedMode_Standard,  //!< Standard mode: up to 100 kbit/s
    Fast      = SilKit_I2cSpeedMode_Fast,      //!< Fast mode: up to 400 kbit/s
    FastPlus  = SilKit_I2cSpeedMode_FastPlus,  //!< Fast-mode plus: up to 1 Mbit/s
    HighSpeed = SilKit_I2cSpeedMode_HighSpeed, //!< High-speed mode: up to 3.4 Mbit/s
};

using I2cTransmitStatusMask = SilKit_I2cTransmitStatus;

//! \brief Result of an I2C transfer as seen by the master.
enum class I2cTransmitStatus : I2cTransmitStatusMask
{
    Transmitted     = SilKit_I2cTransmitStatus_Transmitted,     //!< All bytes ACKed by slave
    AddressNak      = SilKit_I2cTransmitStatus_AddressNak,      //!< No slave responded to address
    DataNak         = SilKit_I2cTransmitStatus_DataNak,         //!< Slave NACKed a data byte
    ArbitrationLost = SilKit_I2cTransmitStatus_ArbitrationLost, //!< Master lost arbitration (detailed sim only)
    BusError        = SilKit_I2cTransmitStatus_BusError,        //!< Protocol violation or invalid request
};

using I2cControllerModeMask = SilKit_I2cControllerMode;

//! \brief Operating mode of an I2C controller.
enum class I2cControllerMode : I2cControllerModeMask
{
    Inactive = SilKit_I2cControllerMode_Inactive, //!< Controller not initialized
    Master   = SilKit_I2cControllerMode_Master,   //!< Controller acts as bus master
    Slave    = SilKit_I2cControllerMode_Slave,    //!< Controller acts as addressed slave
};

/*! \brief An I2C frame representing a single transfer (START…STOP).
 *
 *  For Write transfers, \c data contains the payload sent by the master.
 *  For Read transfers, \c data is empty in the request; the slave response is
 *  delivered as a separate I2cFrameEvent.
 */
struct I2cFrame
{
    I2cAddress                address;     //!< Target slave address
    I2cAddressMode            addressMode; //!< 7-bit or 10-bit addressing
    I2cTransferDirection      direction;   //!< Write or Read
    Util::Span<const uint8_t> data;        //!< Payload; empty for Read requests
};

/*! \brief Event delivered to frame handlers when an I2C frame is observed on the bus. */
struct I2cFrameEvent
{
    std::chrono::nanoseconds timestamp;   //!< Simulation time of the transfer
    I2cFrame                 frame;       //!< The I2C frame
    TransmitDirection        direction;   //!< TX (sent by this controller) or RX (received)
    void*                    userContext; //!< Optional pointer provided by user when sending the frame
};

/*! \brief Acknowledgment delivered to the master after a transfer completes (or fails). */
struct I2cFrameTransmitEvent
{
    std::chrono::nanoseconds timestamp;   //!< Simulation time of the acknowledgment
    I2cAddress               address;     //!< Slave address of the completed transfer
    I2cTransmitStatus        status;      //!< Outcome of the transfer
    void*                    userContext; //!< Optional pointer provided by user when sending the frame
};

/*! \brief Configuration passed to II2cController::Init. */
struct I2cControllerConfig
{
    I2cControllerMode mode;             //!< Master or Slave
    I2cAddress        slaveAddress;     //!< Own address when in Slave mode; ignored for Master
    I2cAddressMode    slaveAddressMode; //!< Address width when in Slave mode; ignored for Master
    I2cSpeedMode      speedMode;        //!< Bus speed (informational; trivial sim ignores)
};

} // namespace I2c
} // namespace Services
} // namespace SilKit
