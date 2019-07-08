// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <array>
#include <chrono>
#include <vector>

#include "ib/sim/datatypes.hpp"

namespace ib {
namespace sim {
//! The LIN namespace
namespace lin {

//! (range 0-61)
using LinId = uint8_t;

enum class ControllerMode : uint8_t
{
    Inactive, //!< LIN controller is deactivated.
    Master,   //!< LIN controller is in LIN master mode and operational.
    Slave,    //!< LIN controller is in LIN slave mode and operational.
    Sleep     //!< LIN controller is in sleep mode.
};

//! \brief This type is used to specify the Checksum model to be used for the LIN Frame.
enum class ChecksumModel : uint8_t
{
    Undefined = 0,
    Enhanced, //!< Enhanced checksum model.
    Classic //!< Classic checksum model.
};

enum class ResponseMode : uint8_t
{
    Unused, //!< The LIN frame is neither received nor transmitted by the LIN slave.
    Rx,     //!< The response of the LIN frame is received by the LIN slave.
    TxUnconditional, //!< The LIN frame is an 'unconditional frame' and the response is transmitted by the slave.
};

enum class MessageStatus : uint8_t
{
    TxSuccess, //!< The LIN header and the response was sent successfully on the bus.
    RxSuccess, //!< The LIN header was sent successfully on the bus and a LIN response was successfully received.
    TxResponseError, //!< Erroneous response transmission. Only used with Network Simulator.
    RxResponseError, //!< Erroneous response reception. Only used with Network Simulator.
    RxNoResponse, //!< No response byte has been received. Only used with Network Simulator.
    HeaderError,  //!< Erroneous header transmission. Only used with Network Simulator.
    Canceled, //!< The Transmit operation was aborted. E.g., the controller mode was set to inactive before the transmission was completed. Only used with Network Simulator.
    Busy //!< The transmit request was rejected, because another transmission is already running. Only used with Network Simulator.
};

struct Payload
{
    constexpr Payload(uint8_t size, std::array<uint8_t, 8> data) : size{size}, data{data} {}
    Payload() = default;

    uint8_t size{0}; //!< \brief The payload size.
    std::array<uint8_t, 8> data{}; //!< \brief The data of the specified payload.
};

/* With Network Simulator:
*  - Used by Master Proxies for TX request.
*  - Used by Network Simulator for TX to slaves, RX replies to master and slaves.
* Without Network Simulator:
*  - Used by Master Controllers for TX to slaves.
*  - Used by Slave Controllers for RX replies to master and slaves.
*/
struct LinMessage
{
    MessageStatus status; //!< Status code of the transmission; set by LinControllers or Network Simulator.
    std::chrono::nanoseconds timestamp; //!< End of frame time stamp.

    LinId linId; //!< The LIN ID for the LIN message.
    Payload payload; //!< The payload of the LIN message.
    ChecksumModel checksumModel{ChecksumModel::Undefined}; //!< The checksum model to be used.
};

//! \brief RxRequest issued by a Master
struct RxRequest
{
    LinId linId; //!< The requested LIN ID.
    uint8_t payloadLength; //!< The expected length of the reply's payload.
    ChecksumModel checksumModel{ChecksumModel::Undefined}; //!< The checksum model to be used.
};

//! \brief TxAcknowledge for a LIN Master
struct TxAcknowledge
{
    std::chrono::nanoseconds timestamp; //!< End of frame time stamp.

    LinId linId; //!< The LIN ID for the ack.
    MessageStatus status; //!< The acknowledge status.
};

struct WakeupRequest
{
    std::chrono::nanoseconds timestamp; //!< End of frame time stamp for the wakeUp.
};

struct ControllerConfig
{
    ControllerMode controllerMode{ControllerMode::Inactive}; //!< The controller mode to be used.
    uint32_t baudrate{0}; //!< The baudrate to be used.
};

struct SlaveResponseConfig
{
    LinId linId; //!< The requested LIN ID.
    ResponseMode responseMode{ResponseMode::Unused}; //!< The response mode to be used.
    ChecksumModel checksumModel{ChecksumModel::Enhanced}; //!< The checksum model to be used.
    uint8_t payloadLength{0}; //!< The expected length of the reply's payload.
};

struct SlaveConfiguration
{
    std::vector<SlaveResponseConfig> responseConfigs; //!< All slave response configs.
};

struct SlaveResponse
{
    LinId linId; //!< The response LIN ID.
    Payload payload; //!< The payload for the slave response.
    ChecksumModel checksumModel{ChecksumModel::Undefined}; //!< The checksum model used.
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
