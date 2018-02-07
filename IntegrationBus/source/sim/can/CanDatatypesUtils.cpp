// Copyright (c)  Vector Informatik GmbH. All rights reserved.

#include "CanDatatypesUtils.hpp"

#include <cassert>

namespace ib {
namespace sim {
namespace can {

bool operator==(const CanMessage::CanReceiveFlags& lhs, const CanMessage::CanReceiveFlags& rhs)
{
    return lhs.ide == rhs.ide
        && lhs.rtr == rhs.rtr
        && lhs.fdf == rhs.fdf
        && lhs.brs == rhs.brs
        && lhs.esi == rhs.esi;
}

bool operator==(const CanMessage& lhs, const CanMessage& rhs)
{
    return lhs.transmitId == rhs.transmitId
        && lhs.timestamp == rhs.timestamp

        && lhs.canId == rhs.canId
        && lhs.flags == rhs.flags
        && lhs.dlc == rhs.dlc
        && lhs.dataField == rhs.dataField;
}

bool operator==(const CanTransmitAcknowledge& lhs, const CanTransmitAcknowledge& rhs)
{
    return lhs.transmitId == rhs.transmitId
        && lhs.status == rhs.status;
}

bool operator==(const CanSetControllerMode& lhs, const CanSetControllerMode& rhs)
{
    return lhs.mode == rhs.mode
        && lhs.flags.cancelTransmitRequests == rhs.flags.cancelTransmitRequests
        && lhs.flags.resetErrorHandling == rhs.flags.resetErrorHandling;
}

bool operator==(const CanConfigureBaudrate& lhs, const CanConfigureBaudrate& rhs)
{
    return lhs.baudRate == rhs.baudRate
        && lhs.fdBaudRate == rhs.fdBaudRate;
}

std::ostream& operator<<(std::ostream& out, CanMessage::CanReceiveFlags flags)
{
    out << "["
        << (flags.ide ? "ide," : "")
        << (flags.rtr ? "rtr," : "")
        << (flags.fdf ? "fdf," : "")
        << (flags.brs ? "brs," : "")
        << (flags.esi ? "esi" : "")
        << "]";

    return out;
}

std::ostream& operator<<(std::ostream& out, const CanMessage& msg)
{
    out << "CanMsg{txId=" << msg.transmitId
        << ", time=" << msg.timestamp.count() << "ns"
        << ", canId=" << msg.canId
        << ", flags=" << msg.flags
        << ", dlc=" << static_cast<uint32_t>(msg.dlc)
        << ", data=[0x" << static_cast<const void*>(msg.dataField.data()) << ", s=" << msg.dataField.size() << "]"
        << "}";
    return out;
}

std::ostream& operator<<(std::ostream& out, const CanSetControllerMode& mode)
{
    out << "{";
    switch (mode.mode)
    {
    case CanControllerState::Uninit:
        out << "uninit";
        break;
    case CanControllerState::Stopped:
        out << "stopped";
        break;
    case CanControllerState::Started:
        out << "started";
        break;
    case CanControllerState::Sleep:
        out << "sleep";
        break;
    default:
        assert(false);
    }

    if (mode.flags.cancelTransmitRequests)
        out << ", cancelTX";
    if (mode.flags.resetErrorHandling)
        out << ", resetErrorHandling";
    out << "}";
    return out;
}

std::ostream& operator<<(std::ostream& out, const CanConfigureBaudrate& rate)
{
    out << "{" << rate.baudRate
        << ", " << rate.fdBaudRate
        << "}";
    return out;
}



}
}
}
