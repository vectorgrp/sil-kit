// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "LinSerdes.hpp"


namespace SilKit {
namespace Services {
namespace Lin {

SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const LinFrame& frame)
{
    buffer << frame.id << frame.checksumModel << frame.dataLength << frame.data;
    return buffer;
}
SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, LinFrame& frame)
{
    buffer >> frame.id >> frame.checksumModel >> frame.dataLength >> frame.data;
    return buffer;
}

SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const LinSendFrameRequest& frame)
{
    buffer << frame.frame << frame.responseType;
    return buffer;
}
SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, LinSendFrameRequest& frame)
{
    buffer >> frame.frame >> frame.responseType;
    return buffer;
}

SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const LinSendFrameHeaderRequest& header)
{
    buffer << header.timestamp << header.id;
    return buffer;
}
SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, LinSendFrameHeaderRequest& header)
{
    buffer >> header.timestamp >> header.id;
    return buffer;
}

SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const LinTransmission& transmission)
{
    buffer << transmission.timestamp << transmission.frame << transmission.status;
    return buffer;
}
SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, LinTransmission& transmission)
{
    buffer >> transmission.timestamp >> transmission.frame >> transmission.status;
    return buffer;
}

SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const LinWakeupPulse& pulse)
{
    buffer << pulse.timestamp << pulse.direction;
    return buffer;
}
SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, LinWakeupPulse& pulse)
{
    buffer >> pulse.timestamp >> pulse.direction;
    return buffer;
}

SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const LinFrameResponse& response)
{
    buffer << response.frame << response.responseMode;
    return buffer;
}
SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, LinFrameResponse& response)
{
    buffer >> response.frame >> response.responseMode;
    return buffer;
}

SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const WireLinControllerConfig& config)
{
    buffer << config.controllerMode << config.baudRate << config.frameResponses << config.simulationMode;
    return buffer;
}
SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, WireLinControllerConfig& config)
{
    buffer >> config.controllerMode >> config.baudRate >> config.frameResponses;
    if (buffer.RemainingBytesLeft() >= sizeof(WireLinControllerConfig::SimulationMode))
    {
        buffer >> config.simulationMode;
    }
    return buffer;
}

SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const LinControllerStatusUpdate& msg)
{
    buffer << msg.timestamp << msg.status;
    return buffer;
}
SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, LinControllerStatusUpdate& msg)
{
    buffer >> msg.timestamp >> msg.status;
    return buffer;
}

SilKit::Core::MessageBuffer& operator<<(SilKit::Core::MessageBuffer& buffer, const LinFrameResponseUpdate& update)
{
    buffer << update.frameResponses;
    return buffer;
}
SilKit::Core::MessageBuffer& operator>>(SilKit::Core::MessageBuffer& buffer, LinFrameResponseUpdate& update)
{
    buffer >> update.frameResponses;
    return buffer;
}

using SilKit::Core::MessageBuffer;
void Serialize(MessageBuffer& buffer, const LinFrame& msg)
{
    buffer << msg;
    return;
}
void Serialize(MessageBuffer& buffer, const LinSendFrameRequest& msg)
{
    buffer << msg;
    return;
}
void Serialize(MessageBuffer& buffer, const LinSendFrameHeaderRequest& msg)
{
    buffer << msg;
    return;
}
void Serialize(MessageBuffer& buffer, const LinTransmission& msg)
{
    buffer << msg;
    return;
}
void Serialize(MessageBuffer& buffer, const LinWakeupPulse& msg)
{
    buffer << msg;
    return;
}
void Serialize(MessageBuffer& buffer, const WireLinControllerConfig& msg)
{
    buffer << msg;
    return;
}
void Serialize(MessageBuffer& buffer, const LinControllerStatusUpdate& msg)
{
    buffer << msg;
    return;
}
void Serialize(MessageBuffer& buffer, const LinFrameResponseUpdate& msg)
{
    buffer << msg;
    return;
}
void Serialize(MessageBuffer& buffer, const LinFrameResponse& msg)
{
    buffer << msg;
}

void Deserialize(MessageBuffer& buffer, LinFrame& out)
{
    buffer >> out;
}
void Deserialize(MessageBuffer& buffer, LinSendFrameRequest& out)
{
    buffer >> out;
}
void Deserialize(MessageBuffer& buffer, LinSendFrameHeaderRequest& out)
{
    buffer >> out;
}
void Deserialize(MessageBuffer& buffer, LinTransmission& out)
{
    buffer >> out;
}
void Deserialize(MessageBuffer& buffer, LinWakeupPulse& out)
{
    buffer >> out;
}
void Deserialize(MessageBuffer& buffer, WireLinControllerConfig& out)
{
    buffer >> out;
}
void Deserialize(MessageBuffer& buffer, LinControllerStatusUpdate& out)
{
    buffer >> out;
}
void Deserialize(MessageBuffer& buffer, LinFrameResponse& out)
{
    buffer >> out;
}
void Deserialize(MessageBuffer& buffer, LinFrameResponseUpdate& out)
{
    buffer >> out;
}

} // namespace Lin
} // namespace Services
} // namespace SilKit
