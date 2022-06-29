// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "CanDatatypesUtils.hpp"

namespace ib {
namespace sim {
namespace can {

bool operator==(const CanFrame::CanFrameFlags& lhs, const CanFrame::CanFrameFlags& rhs)
{
    return lhs.ide == rhs.ide
        && lhs.rtr == rhs.rtr
        && lhs.fdf == rhs.fdf
        && lhs.brs == rhs.brs
        && lhs.esi == rhs.esi;
}

bool operator==(const CanFrame& lhs, const CanFrame& rhs)
{
    return lhs.canId == rhs.canId 
        && lhs.flags == rhs.flags 
        && lhs.dlc == rhs.dlc 
        && lhs.dataField == rhs.dataField;
}

bool operator==(const CanFrameEvent& lhs, const CanFrameEvent& rhs)
{
    return lhs.transmitId == rhs.transmitId 
        && lhs.timestamp == rhs.timestamp 
        && lhs.frame == rhs.frame
        && lhs.userContext == rhs.userContext;
    ;
}

bool operator==(const CanFrameTransmitEvent& lhs, const CanFrameTransmitEvent& rhs)
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

bool operator==(const CanStateChangeEvent& lhs, const CanStateChangeEvent& rhs)
{
    return lhs.timestamp == rhs.timestamp 
        && lhs.state == rhs.state;
}

bool operator==(const CanErrorStateChangeEvent& lhs, const CanErrorStateChangeEvent& rhs)
{
    return lhs.timestamp == rhs.timestamp
        && lhs.errorState == rhs.errorState;
}


}
}
}
