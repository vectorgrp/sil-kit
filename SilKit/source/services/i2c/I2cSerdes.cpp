// SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "silkit/services/i2c/I2cDatatypes.hpp"

#include "MessageBuffer.hpp"
#include "I2cSerdes.hpp"

namespace SilKit {
namespace Services {
namespace I2c {

SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const WireI2cFrameEvent& msg)
{
    buffer << msg.timestamp << msg.frame.address << msg.frame.addressMode << msg.frame.direction << msg.frame.data
           << msg.direction << msg.userContext;
    return buffer;
}

SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, WireI2cFrameEvent& msg)
{
    buffer >> msg.timestamp >> msg.frame.address >> msg.frame.addressMode >> msg.frame.direction >> msg.frame.data
        >> msg.direction >> msg.userContext;
    return buffer;
}

SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const I2cAcknowledge& msg)
{
    buffer << msg.timestamp << msg.address << msg.status << msg.userContext;
    return buffer;
}

SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, I2cAcknowledge& msg)
{
    buffer >> msg.timestamp >> msg.address >> msg.status >> msg.userContext;
    return buffer;
}

SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const WireI2cControllerConfig& msg)
{
    buffer << msg.mode << msg.slaveAddress << msg.slaveAddressMode << msg.speedMode;
    return buffer;
}

SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, WireI2cControllerConfig& msg)
{
    buffer >> msg.mode >> msg.slaveAddress >> msg.slaveAddressMode >> msg.speedMode;
    return buffer;
}

using namespace SilKit::Core;
// when changing any of the datatypes, add transparent compatibility code here,
// based on the buffer.GetProtocolVersion()

//////////////////////////////////////////////////////////////////////
// Serialize
//////////////////////////////////////////////////////////////////////

void Serialize(MessageBuffer& buffer, const WireI2cFrameEvent& msg)
{
    buffer << msg;
}

void Serialize(MessageBuffer& buffer, const I2cAcknowledge& msg)
{
    buffer << msg;
}

void Serialize(MessageBuffer& buffer, const WireI2cControllerConfig& msg)
{
    buffer << msg;
}

//////////////////////////////////////////////////////////////////////
// Deserialize
//////////////////////////////////////////////////////////////////////

void Deserialize(MessageBuffer& buffer, WireI2cFrameEvent& out)
{
    buffer >> out;
}

void Deserialize(MessageBuffer& buffer, I2cAcknowledge& out)
{
    buffer >> out;
}

void Deserialize(MessageBuffer& buffer, WireI2cControllerConfig& out)
{
    buffer >> out;
}

} // namespace I2c
} // namespace Services
} // namespace SilKit
