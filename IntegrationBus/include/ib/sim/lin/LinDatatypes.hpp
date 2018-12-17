// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#pragma once

#include <array>
#include <chrono>
#include <vector>

#include "ib/sim/datatypes.hpp"

namespace ib {
namespace sim {
namespace lin { 

using LinId = uint8_t;  // (range 0-61)

enum class ControllerMode : uint8_t
{
    Inactive, // LIN controller is deactivated
    Master,   // LIN controller is in LIN master mode and operational
    Slave,    // LIN controller is in LIN slave mode and operational
    Sleep     // LIN controller is in sleep mode.
};

// This type is used to specify the Checksum model to be used for the LIN Frame
enum class ChecksumModel : uint8_t
{
    Undefined = 0,
    Enhanced, // Enhanced checksum model
    Classic // Classic checksum model
};

enum class ResponseMode : uint8_t
{
    Unused, // The LIN frame is neither received not transmitted by the LIN slave
    Rx,     // The response of the LIN frame is received by the LIN slave
    TxUnconditional, // The LIN frame is an 'unconditional frame' and the
                     // response is transmitted by the slave
};

enum class MessageStatus : uint8_t
{
    TxSuccess, // The LIN header and the response was sent successfully on the bus
    RxSuccess, // The LIN header was sent successfully on the bus and a LIN response was successfully received
    TxResponseError, // Erroneous response transmission. Only used with Network Simulator
    RxResponseError, // Erroneous response reception. Only used with Network Simulator
    RxNoResponse, // No response byte has been received. Only used with Network Simulator
    HeaderError,  // Erroneous header transmission. Only used with Network Simulator
    Canceled, // The Transmit operation was aborted. E.g., the controller mode was set to inactive before the transmission was completed. Only used with Network Simulator
    Busy // The transmit request was rejected, because another transmission is already running. Only used with Network Simulator
};

struct Payload
{
    uint8_t size = 0;
    std::array<uint8_t, 8> data;
};

// With Network Simulator:
//  - Used by Master Proxies for TX request
//  - Used by Network Simulator for TX to slaves, RX replies to master and slaves
// Without Network Simulator
//  - Used by Master Controllers for TX to slaves
//  - Used by Slave Controllers for RX replies to master and slaves
struct LinMessage
{
    MessageStatus status;               // Status code of the transmission; set by LinControllers or Network Simulator
    std::chrono::nanoseconds timestamp; // end of frame time stamp

    LinId linId;
    Payload payload;
    ChecksumModel checksumModel{ChecksumModel::Undefined}; // must be set when using a Network Simulator
};

//! \brief RxRequest issued by a Master
struct RxRequest
{
    LinId linId; // The Requested LIN ID
    uint8_t payloadLength; // The expected length of the reply's payload
    ChecksumModel checksumModel{ChecksumModel::Undefined}; // The checksum model to be used
};

//! \brief TxAcknowledge for a LIN Master
struct TxAcknowledge
{
    std::chrono::nanoseconds timestamp; // end of frame time stamp

    LinId linId;
    MessageStatus status;
};

struct WakeupRequest
{
    std::chrono::nanoseconds timestamp; // end of frame time stamp
};

// LIN ControllerProxy to LIN Network Simulator
struct ControllerConfig
{
    ControllerMode controllerMode;
    uint32_t baudrate;
};

// LIN ControllerProxy to LIN Network Simulator
struct SlaveResponseConfig
{
    ResponseMode responseMode{ResponseMode::Unused};
    ChecksumModel checksumModel{ChecksumModel::Enhanced};
    uint8_t payloadLength{0};
};

struct SlaveConfiguration
{
    std::vector<SlaveResponseConfig> responseConfigs;
};

// LIN ControllerProxy to LIN Network Simulator
struct SlaveResponse
{
    LinId linId;
    Payload payload;
    ChecksumModel checksumModel{ChecksumModel::Undefined};
};

// ================================================================================
//  Inline Implementations
// ================================================================================
inline bool operator==(const Payload& lhs, const Payload& rhs)
{
    return lhs.size == rhs.size
        && lhs.data == rhs.data;
}

} // namespace lin
} // namespace sim
} // namespace ib
