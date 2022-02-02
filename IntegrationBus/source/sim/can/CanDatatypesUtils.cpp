// Copyright (c) Vector Informatik GmbH. All rights reserved.

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
        && lhs.dataField == rhs.dataField
        && lhs.userContext == rhs.userContext;
}

bool operator==(const CanTransmitAcknowledge& lhs, const CanTransmitAcknowledge& rhs)
{
    return lhs.transmitId == rhs.transmitId
        && lhs.status == rhs.status && lhs.userContext == rhs.userContext;
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


}
}
}
