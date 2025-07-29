// SPDX-FileCopyrightText: 2022 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include "CanDatatypesUtils.hpp"

namespace SilKit {
namespace Services {
namespace Can {

bool operator==(const CanFrame& lhs, const CanFrame& rhs)
{
    return lhs.canId == rhs.canId && lhs.flags == rhs.flags && lhs.dlc == rhs.dlc && lhs.sdt == rhs.sdt
           && lhs.vcid == rhs.vcid && lhs.af == rhs.af && Util::ItemsAreEqual(lhs.dataField, rhs.dataField);
}

bool operator==(const CanFrameEvent& lhs, const CanFrameEvent& rhs)
{
    return lhs.timestamp == rhs.timestamp && lhs.frame == rhs.frame && lhs.userContext == rhs.userContext;
    ;
}

bool operator==(const CanFrameTransmitEvent& lhs, const CanFrameTransmitEvent& rhs)
{
    return lhs.status == rhs.status && lhs.userContext == rhs.userContext;
}

bool operator==(const CanSetControllerMode& lhs, const CanSetControllerMode& rhs)
{
    return lhs.mode == rhs.mode && lhs.flags.cancelTransmitRequests == rhs.flags.cancelTransmitRequests
           && lhs.flags.resetErrorHandling == rhs.flags.resetErrorHandling;
}

bool operator==(const CanConfigureBaudrate& lhs, const CanConfigureBaudrate& rhs)
{
    return lhs.baudRate == rhs.baudRate && lhs.fdBaudRate == rhs.fdBaudRate;
}

bool operator==(const CanStateChangeEvent& lhs, const CanStateChangeEvent& rhs)
{
    return lhs.timestamp == rhs.timestamp && lhs.state == rhs.state;
}

bool operator==(const CanErrorStateChangeEvent& lhs, const CanErrorStateChangeEvent& rhs)
{
    return lhs.timestamp == rhs.timestamp && lhs.errorState == rhs.errorState;
}

bool operator==(const WireCanFrameEvent& lhs, const WireCanFrameEvent& rhs)
{
    return ToCanFrameEvent(lhs) == ToCanFrameEvent(rhs);
}

} // namespace Can
} // namespace Services
} // namespace SilKit
